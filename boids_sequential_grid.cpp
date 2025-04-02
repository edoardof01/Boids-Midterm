#include "BoidsGrid.hpp"
#include "BoidsCommon.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <cstdlib>

int main(const int argc, char* argv[]) {
    int numBoids = 0;

    // --- Parsing dell'argomento da riga di comando ---
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

    // Parametri:
    constexpr int STEPS = 500;
    constexpr float cellSize = CELL_SIZE;

    // Inizializzazione boid
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution distX(0.0f, WIDTH);
    std::uniform_real_distribution distY(0.0f, HEIGHT);
    std::uniform_real_distribution distVel(-1.0f, 1.0f);

    std::vector<Boid> oldState(numBoids), newState(numBoids);

    for (int i = 0; i < numBoids; ++i) {
        oldState[i].position = Vector2(distX(gen), distY(gen));
        Vector2 v(distVel(gen), distVel(gen));
        v = v.normalized() * MAX_SPEED;
        oldState[i].velocity = v;
    }

    // Griglia
    UniformGrid grid(WIDTH, HEIGHT, cellSize);

    const auto start = std::chrono::high_resolution_clock::now();

    for (int step = 0; step < STEPS; ++step) {
        buildGrid(oldState, grid, cellSize);

        for (int i = 0; i < numBoids; ++i) {
            computeNextBoidGrid(i, oldState, newState, grid, cellSize);
        }

        oldState.swap(newState);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "TIME=" << elapsed << std::endl;
    return 0;
}
