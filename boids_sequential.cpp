#include "BoidsCommon.hpp"
#include "BoidsUpdate.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
#include <cstdlib>

int main(const int argc, char* argv[]) {
    // --- Parsing argomento da linea di comando ---
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

    // --- Allocazione stati ---
    std::vector<Boid> oldState(numBoids);
    std::vector<Boid> newState(numBoids);

    // --- Inizializzazione su griglia con velocità radiale ---
    const int gridSize = static_cast<int>(std::ceil(std::sqrt(numBoids)));
    const float spacingX = WIDTH / static_cast<float>(gridSize);
    const float spacingY = HEIGHT / static_cast<float>(gridSize);
    constexpr float centerX = WIDTH / 2.0f;
    constexpr float centerY = HEIGHT / 2.0f;

    for (int i = 0; i < numBoids; ++i) {
        const int row = i / gridSize;
        const int col = i % gridSize;

        const float posX = static_cast<float>(col) * spacingX + spacingX / 2.0f;
        const float posY = static_cast<float>(row) * spacingY + spacingY / 2.0f;

        oldState[i].position = { posX, posY };

        const float angle = std::atan2(posY - centerY, posX - centerX);
        oldState[i].velocity = {
            std::cos(angle) * MAX_SPEED,
            std::sin(angle) * MAX_SPEED
        };
    }

    // --- Simulazione ---
    constexpr int STEPS = 600;
    const auto start = std::chrono::high_resolution_clock::now();

    for (int step = 0; step < STEPS; ++step) {
        for (int i = 0; i < numBoids; ++i) {
            computeNextBoid(i, oldState, newState);
        }
        oldState.swap(newState);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "TIME=" << elapsed << " seconds" << std::endl;
    return 0;
}
