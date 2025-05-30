#pragma once

#include "BoidsCommon.hpp"
#include <cmath>
#include <functional>
#include <vector>

constexpr float CELL_SIZE = VIEW_RADIUS * 2;

struct UniformGrid {
    int cellCountX;
    int cellCountY;

    std::vector<std::vector<int>> cells;
    std::vector<int> usedCells; // <-- AGGIUNTO per clear selettivo

    UniformGrid(const float width, const float height, const float cellSize) {
        cellCountX = static_cast<int>(std::ceil(width / cellSize));
        cellCountY = static_cast<int>(std::ceil(height / cellSize));
        cells.resize(cellCountX * cellCountY);
    }

    void clear() {
        for (const int idx : usedCells) {
            cells[idx].clear();
        }
        usedCells.clear();
    }

    [[nodiscard]] int getCellIndex(const int cellX, const int cellY) const {
        return cellY * cellCountX + cellX; // numero di cella contandole dall'inizio alla fine
    }
};

inline std::pair<int, int> getCellCoords(const Vector2& pos, const float cellSize, const int maxX, const int maxY) {
    int cellX = static_cast<int>(std::floor(pos.x / cellSize));
    int cellY = static_cast<int>(std::floor(pos.y / cellSize));

    if (cellX < 0) cellX = 0;
    if (cellX >= maxX) cellX = maxX - 1;
    if (cellY < 0) cellY = 0;
    if (cellY >= maxY) cellY = maxY - 1;

    return {cellX, cellY};
}

inline void buildGrid(const std::vector<Boid>& oldState,
                      UniformGrid& grid,
                      const float cellSize)
{
    grid.clear();

    for (int i = 0; i < static_cast<int>(oldState.size()); ++i) {
        auto [cellX, cellY] = getCellCoords(oldState[i].position, cellSize,
            grid.cellCountX, grid.cellCountY);
        const int cellIndex = grid.getCellIndex(cellX, cellY);
        grid.cells[cellIndex].push_back(i);
        grid.usedCells.push_back(cellIndex);
    }
}

inline void forEachNeighborBoid(
    const int cellX, const int cellY,
    const UniformGrid& grid,
    const std::function<void(int otherIndex)>& callback)
{
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            const int nx = cellX + dx;
            const int ny = cellY + dy;

            if (nx < 0 || nx >= grid.cellCountX) continue;
            if (ny < 0 || ny >= grid.cellCountY) continue;

            for (const int neighborCellIndex = grid.getCellIndex(nx, ny);
                const int otherIdx : grid.cells[neighborCellIndex]) {
                callback(otherIdx);
            }
        }
    }
}

/**
 * Esempio di "separation" usando la griglia:
 * - sai in quale cella (cellX, cellY) si trova il boid 'b'
 * - scorri solo la cella e quelle adiacenti
 */

inline Vector2 separationGrid(const Boid& b,
                              const int boidIndex,
                              const std::vector<Boid>& current,
                              const UniformGrid& grid,
                              const int cellX, const int cellY)
{
    Vector2 steer{0, 0};
    int count = 0;

    forEachNeighborBoid(cellX, cellY, grid, [&](const int otherIdx){
        if (otherIdx == boidIndex) return;
        const auto&[position, velocity] = current[otherIdx];
        if (const float dist = (b.position - position).magnitude();
            dist < SEPARATION_RADIUS) {
            Vector2 diff = (b.position - position).normalized();
            diff = diff / (dist * dist);
            steer = steer + diff;
            count++;
        }
    });

    if (count > 0) {
        steer = steer / static_cast<float>(count);
        steer = steer.normalized() * MAX_SPEED;
        steer = steer - b.velocity;
        steer.limit(MAX_FORCE);
    }
    return steer;
}

/**
 * Alignment usando la griglia
 */
inline Vector2 alignmentGrid(const Boid& b,
                             const int boidIndex,
                             const std::vector<Boid>& current,
                             const UniformGrid& grid,
                             const int cellX, const int cellY)
{
    Vector2 sum{0, 0};
    int count = 0;

    forEachNeighborBoid(cellX, cellY, grid, [&](const int otherIdx){
        if (otherIdx == boidIndex) return;
        const auto&[position, velocity] = current[otherIdx];
        if (const float dist = (b.position - position).magnitude(); dist < VIEW_RADIUS) {
            const float weight = (VIEW_RADIUS - dist) / VIEW_RADIUS;
            sum = sum + velocity * weight;
            count++;
        }
    });

    if (count > 0) {
        sum = sum / static_cast<float>(count);
        sum = sum.normalized() * MAX_SPEED;
        Vector2 steer = sum - b.velocity;
        steer.limit(MAX_FORCE);
        return steer;
    }
    return Vector2{0, 0};
}

/**
 * Cohesion usando la griglia
 */
inline Vector2 cohesionGrid(const Boid& b,
                            const int boidIndex,
                            const std::vector<Boid>& current,
                            const UniformGrid& grid,
                            const int cellX, const int cellY)
{
    Vector2 center{0, 0};
    int count = 0;

    forEachNeighborBoid(cellX, cellY, grid, [&](const int otherIdx){
        if (otherIdx == boidIndex) return;
        const auto&[position, velocity] = current[otherIdx];
        if (const float dist = (b.position - position).magnitude(); dist < VIEW_RADIUS) {
            center = center + position;
            count++;
        }
    });

    if (count > 0) {
        center = center / static_cast<float>(count);
        const Vector2 desired = (center - b.position).normalized() * MAX_SPEED;
        Vector2 steer = desired - b.velocity;
        steer.limit(MAX_FORCE);
        return steer;
    }
    return Vector2{0, 0};
}

/**
 * computeNextBoidGrid:
 * Calcola la nuova posizione/velocità di boid "i" usando la GRIGLIA.
 */
inline void computeNextBoidGrid(const int i,
                                const std::vector<Boid>& oldState,
                                std::vector<Boid>& newState,
                                const UniformGrid& grid,
                                const float cellSize)
{
    const Boid& b = oldState[i];

    auto [cellX, cellY] = getCellCoords(b.position, cellSize, grid.cellCountX, grid.cellCountY);

    // Calcolo forze (usando le versioni "Grid"!)
    const Vector2 sep = separationGrid(b, i, oldState, grid, cellX, cellY) * SEPARATION_WEIGHT;
    const Vector2 ali = alignmentGrid(b, i, oldState, grid, cellX, cellY)   * ALIGNMENT_WEIGHT;
    const Vector2 coh = cohesionGrid(b, i, oldState, grid, cellX, cellY)    * COHESION_WEIGHT;

    // Somma delle forze = accelerazione
    Vector2 acceleration = sep + ali + coh;
    acceleration.limit(MAX_FORCE);

    // Nuova velocità
    Vector2 newVelocity = b.velocity + acceleration;
    newVelocity.limit(MAX_SPEED);

    // Nuova posizione
    Vector2 newPosition = b.position + newVelocity;

    // Wrap-around
    constexpr float margin = 5.0f;
    if (newPosition.x < -margin)       newPosition.x = WIDTH + margin;
    else if (newPosition.x > WIDTH + margin)   newPosition.x = -margin;
    if (newPosition.y < -margin)       newPosition.y = HEIGHT + margin;
    else if (newPosition.y > HEIGHT + margin)  newPosition.y = -margin;

    // Salvo nello stato "futuro"
    newState[i].position = newPosition;
    newState[i].velocity = newVelocity;
}





