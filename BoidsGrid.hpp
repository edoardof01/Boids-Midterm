#pragma once

#include "BoidsCommon.hpp"
#include <cmath>
#include <functional>
#include <vector>

constexpr float CELL_SIZE = VIEW_RADIUS * 2;

struct UniformGrid {
    int cellCountX;
    int cellCountY;

    std::vector<std::vector<std::vector<int>>> cells;

    UniformGrid(const float width, const float height, const float cellSize) {
        cellCountX = static_cast<int>(std::ceil(width  / cellSize));
        cellCountY = static_cast<int>(std::ceil(height / cellSize));
        cells.resize(cellCountX, std::vector<std::vector<int>>(cellCountY));
    }

    // Svuota tutte le celle per ripopolare
    void clear() {
        for (int x = 0; x < cellCountX; x++) {
            for (int y = 0; y < cellCountY; y++) {
                cells[x][y].clear();
            }
        }
    }
};

/**
 * Inserisce i boid in "grid", in base alla loro posizione.
 * oldState[i] va nella cella (cellX, cellY).
 */
inline void buildGrid(const std::vector<Boid>& oldState,
                      UniformGrid& grid,
                      const float cellSize)
{
    grid.clear();

    for (int i = 0; i < static_cast<int>(oldState.size()); i++) {
        const float x = oldState[i].position.x;
        const float y = oldState[i].position.y;

        // Calcolo cella
        int cellX = static_cast<int>(std::floor(x / cellSize));
        int cellY = static_cast<int>(std::floor(y / cellSize));

        // Attenzione al wrap-around se i boid escono dai bordi
        // (facoltativo: se li rimetti in [0, WIDTH], [0, HEIGHT] altrove
        //  potrebbe non servire)
        if (cellX < 0)         cellX = 0;
        if (cellX >= grid.cellCountX) cellX = grid.cellCountX - 1;
        if (cellY < 0)         cellY = 0;
        if (cellY >= grid.cellCountY) cellY = grid.cellCountY - 1;

        // Aggiunge l'indice boid "i" in quella cella
        grid.cells[cellX][cellY].push_back(i);
    }
}

/**
 * Helpers per trovare i boid "vicini" in una UniformGrid.
 *
 * Data la cella (cellX, cellY), scorriamo se x e y intorno a +/-1
 * (finché restiamo entro i limiti).
 */
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

            // Per tutti i boid nella cella [nx][ny], chiamiamo callback(...)
            for (const int otherIdx : grid.cells[nx][ny]) {
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
        if (const float dist = (b.position - position).magnitude(); dist < SEPARATION_RADIUS) {
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
            sum = sum + (velocity * weight);
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

    // Calcolo la cella in cui si trova boid i
    int cellX = static_cast<int>(std::floor(b.position.x / cellSize));
    int cellY = static_cast<int>(std::floor(b.position.y / cellSize));

    // Controllo bounds
    if (cellX < 0) cellX = 0;
    if (cellX >= grid.cellCountX) cellX = grid.cellCountX - 1;
    if (cellY < 0) cellY = 0;
    if (cellY >= grid.cellCountY) cellY = grid.cellCountY - 1;

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

