#include "BoidsGrid.hpp"
#include "BoidsCommon.hpp"
#include <iostream>
#include <chrono>
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

    // Inizializzazione parallela
    #pragma omp parallel default(none) shared(oldState, numBoids)
    {
        std::random_device rd;
        std::mt19937 gen(rd() ^ omp_get_thread_num());
        std::uniform_real_distribution<float> distX(0.0f, WIDTH);
        std::uniform_real_distribution<float> distY(0.0f, HEIGHT);
        std::uniform_real_distribution<float> distVel(-1.0f, 1.0f);

        #pragma omp for
        for (int i = 0; i < numBoids; ++i) {
            oldState[i].position = Vector2(distX(gen), distY(gen));
            Vector2 v(distVel(gen), distVel(gen));
            v = v.normalized() * MAX_SPEED;
            oldState[i].velocity = v;
        }
    }

    UniformGrid grid(WIDTH, HEIGHT, cellSize);
    const auto start = std::chrono::high_resolution_clock::now();

    // Tutta la simulazione dentro una singola parallel region
    #pragma omp parallel default(none) shared(oldState, newState, grid, cellSize, numBoids) nowait
    {
        constexpr int STEPS = 600;
        for (int step = 0; step < STEPS; ++step) {
            // Costruzione della griglia: eseguita da un solo thread
            #pragma omp single
            {
                buildGrid(oldState, grid, cellSize);
            }
            // Computazione parallela dei nuovi stati dei boid
            #pragma omp for schedule(static)
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoidGrid(i, oldState, newState, grid, cellSize);
            }

            // Swap tra buffer vecchio e nuovo: eseguito da un solo thread
            #pragma omp single
            {
                oldState.swap(newState);
            }

            // Barriera implicita dopo `single` → sincronizzazione per sicurezza
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "TIME=" << elapsed << " seconds" << std::endl;
    return 0;
}
