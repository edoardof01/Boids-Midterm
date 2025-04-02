#include "BoidsSOA.hpp"
#include "BoidsUpdateSOA.hpp"
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

    BoidSoA oldState(numBoids), newState(numBoids);

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
            oldState.posX[i] = distX(gen);
            oldState.posY[i] = distY(gen);

            Vector2 v{ distVel(gen), distVel(gen) };
            v = v.normalized() * MAX_SPEED;

            oldState.velX[i] = v.x;
            oldState.velY[i] = v.y;
        }
    }

    const auto start = std::chrono::high_resolution_clock::now();

    // Tutta la simulazione dentro una singola parallel region
    #pragma omp parallel default(none) shared(oldState, newState, numBoids) nowait
    {
        constexpr int STEPS = 600;
        for (int step = 0; step < STEPS; ++step) {
            #pragma omp for
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoidSoA(i, oldState, newState, numBoids);
            }

            // Solo un thread fa lo swap
            #pragma omp single
            {
                oldState.swap(newState);
            }
            // Barriera implicita alla fine del ciclo `single` per sincronizzare tutti i thread
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "TIME=" << elapsed << " seconds" << std::endl;
    return 0;
}
