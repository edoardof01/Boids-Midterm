#include "BoidsCommon.hpp"
#include "BoidsUpdate.hpp"
#include <iostream>
#include <vector>
#include <random>
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

    // --- Creazione stato dei boids ---
    std::vector<Boid> oldState(numBoids);
    std::vector<Boid> newState(numBoids);





    // Usare OpenMP per parallelizzare l'inizializzazione
    #pragma omp parallel default(none) shared(oldState, numBoids)
    {
        std::random_device rd;
        // Ogni thread ha il proprio generatore casuale
        std::mt19937 gen(rd() ^ omp_get_thread_num());
        std::uniform_real_distribution distX(0.0f, WIDTH);
        std::uniform_real_distribution distY(0.0f, HEIGHT);
        std::uniform_real_distribution distVel(-1.0f, 1.0f);

        // Inizializzazione dei boids
        #pragma omp for schedule(static)
        for (int i = 0; i < numBoids; ++i) {
            oldState[i].position = { distX(gen), distY(gen) };
            Vector2 v{ distVel(gen), distVel(gen) };
            v = v.normalized() * MAX_SPEED;
            oldState[i].velocity = v;
        }
    }

    // Inizio conteggio del tempo
    const auto start = std::chrono::high_resolution_clock::now();

    // Numero di passi per la simulazione
    constexpr int STEPS = 600;

    // Simulazione della dinamica dei boids
    for (int step = 0; step < STEPS; ++step) {
    #pragma omp parallel default(none) shared(numBoids, oldState, newState) nowait
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numBoids; ++i) {
                computeNextBoid(i, oldState, newState);
            }
            #pragma omp single
            {
                oldState.swap(newState); // fa uno scambio degli array contenitori.
                                            //oldState punta al nuovo stato e newState diventa vuoto
            }
        }
    }



    // Fine conteggio del tempo
    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    // Output del tempo di esecuzione
    std::cout << "TIME=" << elapsed << " seconds" << std::endl;

    return 0;
}