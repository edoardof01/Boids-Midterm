#include "BoidsCommon.hpp"
#include "BoidsUpdate.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <omp.h>


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

    // First-touch: inizializza newState in parallelo per mappare bene la memoria
    #pragma omp parallel for default(none) shared(oldState, newState, numBoids) schedule(static)
    for (int i = 0; i < numBoids; ++i) {
        oldState[i] = Boid{};  // scrittura = first-touch
        newState[i] = Boid{};  // scrittura = first-touch
    }


    // --- Inizializzazione griglia regolare ---
    const int gridSize = static_cast<int>(std::ceil(std::sqrt(numBoids)));
    const float spacingX = WIDTH / static_cast<float>(gridSize);
    const float spacingY = HEIGHT / static_cast<float>(gridSize);
    constexpr float centerX = WIDTH / 2.0f;
    constexpr float centerY = HEIGHT / 2.0f;

    // Parallelizzata: ogni boid viene posizionato su una griglia e riceve velocità radiale
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

    // --- Simulazione ---
    const auto start = std::chrono::high_resolution_clock::now();

    // Regione parallela globale
    #pragma omp parallel default(none) shared(oldState, newState, numBoids) nowait
    {
        constexpr int STEPS = 600;
        for (int step = 0; step < STEPS; ++step) {
            // Calcolo parallelo del nuovo stato
            #pragma omp for schedule(static)
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoid(i, oldState, newState);
            }

            // Swap sequenziale tra i due buffer
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
