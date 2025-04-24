//boids_parallel_grid.cpp
#include "BoidsGrid.hpp"
#include "BoidsCommon.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <random>
#include <cstdlib>
#include <omp.h>

// Versione parallela di buildGrid su griglia piatta (ottimizzata)
void parallelBuildGrid(const std::vector<Boid>& oldState,
                        UniformGrid& grid,
                        const float cellSize)
{
    const int N = static_cast<int>(oldState.size());
    const int numCells = grid.cellCountX * grid.cellCountY;

    // Svuota solo le celle usate
    #pragma omp parallel for default(none) shared(grid) schedule(static)
    for (int i = 0; i < static_cast<int>(grid.usedCells.size()); ++i) {
        grid.cells[grid.usedCells[i]].clear();
    }
    grid.usedCells.clear();

    // Prepara storage per ogni thread
    const int maxThreads = omp_get_max_threads();
    std::vector threadLocalCells(maxThreads, std::vector<std::vector<int>>(numCells));

    // Inserisce boid nelle celle in parallelo (senza lock)
    #pragma omp parallel default(none) shared(oldState, grid, threadLocalCells, cellSize, numCells, N)
    {
        const int threadId = omp_get_thread_num();
        auto& localCells = threadLocalCells[threadId];

        #pragma omp for schedule(static)
        for (int i = 0; i < N; ++i) {
            auto [cellX, cellY] = getCellCoords(oldState[i].position,
                cellSize, grid.cellCountX, grid.cellCountY);
            const int cellIndex = grid.getCellIndex(cellX, cellY);
            localCells[cellIndex].push_back(i);
        }
    }

    // Merge delle celle in parallelo + tracciamento usedCells
    #pragma omp parallel for default(none) shared(grid, threadLocalCells, numCells) schedule(static)
    for (int i = 0; i < numCells; ++i) {
        for (const auto& local : threadLocalCells) {
            if (!local[i].empty()) {
                grid.cells[i].insert(grid.cells[i].end(), local[i].begin(), local[i].end());
            }
        }
        if (!grid.cells[i].empty()) {
            #pragma omp critical
            grid.usedCells.push_back(i);
        }
    }
}

int main(const int argc, char* argv[]) {
    int numBoids = 0;

    if (argc > 1) {
        char* end;
        if (const long val = std::strtol(argv[1], &end, 10); *end == '\0' && val > 0) {
            numBoids = static_cast<int>(val);
        } else {
            std::cerr << "Argomento non valido. Uscita.\n";
            return 1;
        }
    } else {
        std::cerr << "Numero di boid non specificato. Uscita.\n";
        return 1;
    }

    constexpr float cellSize = CELL_SIZE;

    std::vector<Boid> oldState(numBoids), newState(numBoids);

    const int gridSize = static_cast<int>(std::ceil(std::sqrt(numBoids)));
    const float spacingX = WIDTH / static_cast<float>(gridSize);
    const float spacingY = HEIGHT / static_cast<float>(gridSize);
    constexpr float centerX = WIDTH / 2.0f;
    constexpr float centerY = HEIGHT / 2.0f;

    #pragma omp parallel for default(none) shared(oldState, numBoids, gridSize, spacingX, spacingY) firstprivate(centerX, centerY) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        const int row = i / gridSize;
        const int col = i % gridSize;
        const float posX = static_cast<float>(col) * spacingX + spacingX / 2.0f;
        const float posY = static_cast<float>(row) * spacingY + spacingY / 2.0f;

        oldState[i].position = { posX, posY };

        const float angle = std::atan2(posY - centerY, posX - centerX);
        oldState[i].velocity = { std::cos(angle) * MAX_SPEED, std::sin(angle) * MAX_SPEED };
    }

    #pragma omp parallel for default(none) shared(newState, numBoids) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        newState[i] = Boid{};
    }

    UniformGrid grid(WIDTH, HEIGHT, cellSize);
    for (auto& cell : grid.cells) cell.reserve(64);  // Prealloca spazio medio

    const auto start = std::chrono::high_resolution_clock::now();

    #pragma omp parallel default(none) shared(oldState, newState, grid, cellSize, numBoids)
    {
        constexpr int STEPS = 600;
        for (int step = 0; step < STEPS; ++step) {
            #pragma omp single
            {
                parallelBuildGrid(oldState, grid, cellSize);
            }

            #pragma omp barrier

            #pragma omp for schedule(static)
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoidGrid(i, oldState, newState, grid, cellSize);
            }

            #pragma omp single
            {
                oldState.swap(newState);
            }
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "TIME=" << elapsed << " seconds" << std::endl;
    return 0;
}



