#include "BoidsSOA.hpp"
#include "BoidsUpdateSOA.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
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

    BoidSoA oldState(numBoids), newState(numBoids);

    // Inizializzazione deterministica su griglia regolare
    const int gridSize = static_cast<int>(std::ceil(std::sqrt(numBoids)));
    const float spacingX = WIDTH / static_cast<float>(gridSize);
    const float spacingY = HEIGHT / static_cast<float>(gridSize);
    constexpr float centerX = WIDTH / 2.0f;
    constexpr float centerY = HEIGHT / 2.0f;

    #pragma omp parallel for default(none) shared(oldState, numBoids, gridSize, spacingX, spacingY, centerX, centerY) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        const int row = i / gridSize;
        const int col = i % gridSize;
        const float posX = static_cast<float>(col) * spacingX + spacingX / 2.0f;
        const float posY = static_cast<float>(row) * spacingY + spacingY / 2.0f;

        oldState.posX[i] = posX;
        oldState.posY[i] = posY;

        const float angle = std::atan2(posY - centerY, posX - centerX);
        oldState.velX[i] = std::cos(angle) * MAX_SPEED;
        oldState.velY[i] = std::sin(angle) * MAX_SPEED;
    }

    // First-touch: inizializza anche newState
    #pragma omp parallel for default(none) shared(newState, numBoids) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        newState.posX[i] = 0.0f;
        newState.posY[i] = 0.0f;
        newState.velX[i] = 0.0f;
        newState.velY[i] = 0.0f;
    }

    const auto start = std::chrono::high_resolution_clock::now();

    #pragma omp parallel default(none) shared(oldState, newState, numBoids) nowait
    {
        constexpr int STEPS = 600;
        for (int step = 0; step < STEPS; ++step) {
            #pragma omp for schedule(static)
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoidSoA(i, oldState, newState, numBoids);
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
