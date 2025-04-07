#include "BoidsGrid.hpp"
#include "BoidsCommon.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <random>
#include <cstdlib>
#include <omp.h>

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

    // --- Inizializzazione su griglia regolare con direzioni radiali ---
    const int gridSize = static_cast<int>(std::ceil(std::sqrt(numBoids)));
    const float spacingX = WIDTH / static_cast<float>(gridSize);
    const float spacingY = HEIGHT / static_cast<float>(gridSize);
    constexpr float centerX = WIDTH / 2.0f;
    constexpr float centerY = HEIGHT / 2.0f;

    // Parallelizza la posizione iniziale (e first-touch di oldState)
    #pragma omp parallel for default(none) shared(oldState, numBoids, gridSize, spacingX, spacingY, centerX, centerY) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        const int row = i / gridSize;
        const int col = i % gridSize;
        const float posX = static_cast<float>(col) * spacingX + spacingX / 2.0f;
        const float posY = static_cast<float>(row) * spacingY + spacingY / 2.0f;

        oldState[i].position = { posX, posY };

        const float angle = std::atan2(posY - centerY, posX - centerX);
        oldState[i].velocity = { std::cos(angle) * MAX_SPEED, std::sin(angle) * MAX_SPEED };
    }

    // --- First-touch per newState ---
    #pragma omp parallel for default(none) shared(newState, numBoids) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        newState[i] = Boid{};
    }

    // Griglia
    UniformGrid grid(WIDTH, HEIGHT, cellSize);

    const auto start = std::chrono::high_resolution_clock::now();

    // --- Simulazione ---
    #pragma omp parallel default(none) shared(oldState, newState, grid, cellSize, numBoids) nowait
    {
        constexpr int STEPS = 600;
        for (int step = 0; step < STEPS; ++step) {
            // Costruzione griglia
            #pragma omp single
            {
                buildGrid(oldState, grid, cellSize);
            }

            #pragma omp barrier

            // Calcolo del nuovo stato
            #pragma omp for schedule(static)
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoidGrid(i, oldState, newState, grid, cellSize);
            }

            // Swap tra old e new
            #pragma omp single
            {
                oldState.swap(newState);
            }

            // Implicit barrier qui
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "TIME=" << elapsed << " seconds" << std::endl;
    return 0;
}
