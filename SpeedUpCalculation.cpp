#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <vector>

// Misura il tempo di esecuzione di un comando esterno
double measureExecutionTime(const std::string &command) {
    using Clock = std::chrono::high_resolution_clock;
    const auto start = Clock::now();
    const int ret = std::system(command.c_str());
    const auto end = Clock::now();

    if (ret != 0) {
        std::cerr << "Warning: comando \"" << command
                  << "\" terminato con codice di ritorno " << ret << '\n';
    }
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    std::vector<int> boidCounts;
    std::cout << "Inserisci i valori di boid da testare (termina con 0): ";
    while (true) {
        int val;
        std::cin >> val;
        if (!std::cin || val <= 0) break;
        boidCounts.push_back(val);
    }

    if (boidCounts.empty()) {
        std::cerr << "Nessun valore inserito. Uscita.\n";
        return 1;
    }

    std::ofstream outNoGrid("speedup_data_nogrid.txt", std::ios::trunc);
    std::ofstream outGrid("speedup_data_grid.txt",  std::ios::trunc);
    std::ofstream outSoA("speedup_data_soa.txt", std::ios::trunc);
    if (!outNoGrid.is_open() || !outGrid.is_open() || !outSoA.is_open()) {
        std::cerr << "Errore: impossibile aprire i file di output.\n";
        return 1;
    }

    for (int numBoids : boidCounts) {
        constexpr int TRIALS = 2;
        std::cout << "\n=== Test con " << numBoids << " boid ===\n";

        std::string seqCmd     = "./SeqHeadless "     + std::to_string(numBoids);
        std::string parCmd     = "./ParHeadless "     + std::to_string(numBoids);
        std::string seqGridCmd = "./SequentialGrid "  + std::to_string(numBoids);
        std::string parGridCmd = "./ParallelGrid "    + std::to_string(numBoids);
        std::string parSoACmd  = "./ParSOAHeadless "  + std::to_string(numBoids);

        double seqTotalNoGrid = 0.0;
        for (int i = 0; i < TRIALS; ++i) seqTotalNoGrid += measureExecutionTime(seqCmd);
        double seqAvgNoGrid = seqTotalNoGrid / TRIALS;
        std::cout << "[NoGrid] Tempo medio seq: " << seqAvgNoGrid << " s\n";

        double parTotalNoGrid = 0.0;
        for (int i = 0; i < TRIALS; ++i) parTotalNoGrid += measureExecutionTime(parCmd);
        double parAvgNoGrid = parTotalNoGrid / TRIALS;
        std::cout << "[NoGrid] Tempo medio par: " << parAvgNoGrid << " s\n";

        if (double speedupNoGrid = (parAvgNoGrid > 0.0) ? (seqAvgNoGrid / parAvgNoGrid) : -1.0; speedupNoGrid > 0.0) {
            std::cout << " Speedup (NoGrid) = " << speedupNoGrid << "\n";
            outNoGrid << numBoids << " " << speedupNoGrid << "\n";
        } else {
            std::cerr << " Errore: parAvgNoGrid = 0.0?\n";
        }

        /*double seqTotalGrid = 0.0;
        for (int i = 0; i < TRIALS; ++i) seqTotalGrid += measureExecutionTime(seqGridCmd);
        double seqAvgGrid = seqTotalGrid / TRIALS;
        std::cout << "[Grid]   Tempo medio seq: " << seqAvgGrid << " s\n";*/


        double parTotalGrid = 0.0;
        for (int i = 0; i < TRIALS; ++i) parTotalGrid += measureExecutionTime(parGridCmd);
        double parAvgGrid = parTotalGrid / TRIALS;
        std::cout << "[Grid]   Tempo medio par: " << parAvgGrid << " s\n";

        if (double speedupGrid = (parAvgGrid > 0.0) ? (seqAvgNoGrid / parAvgGrid) : -1.0; speedupGrid > 0.0) { // Prima era seqAvgGrid
            std::cout << " Speedup (Grid) = " << speedupGrid << "\n";
            outGrid << numBoids << " " << speedupGrid << "\n";
        } else {
            std::cerr << " Errore: parAvgGrid = 0.0?\n";
        }

        double parTotalSoA = 0.0;
        for (int i = 0; i < TRIALS; ++i) parTotalSoA += measureExecutionTime(parSoACmd);
        double parAvgSoA = parTotalSoA / TRIALS;
        std::cout << "[SoA]    Tempo medio par: " << parAvgSoA << " s\n";

        if (double speedupSoA = (parAvgSoA > 0.0) ? (seqAvgNoGrid / parAvgSoA) : -1.0; speedupSoA > 0.0) {
            std::cout << " Speedup (SoA) = " << speedupSoA << "\n";
            outSoA << numBoids << " " << speedupSoA << "\n";
        } else {
            std::cerr << " Errore: parAvgSoA = 0.0?\n";
        }
    }

    outNoGrid.close();
    outGrid.close();
    outSoA.close();

    if (boidCounts.size() >= 3) {
        if (int ret = std::system("gnuplot plot_speedup_grid.gp"); ret != 0) {
            std::cerr << " Errore nell'esecuzione di gnuplot (assicurati che sia installato e nella PATH)\n";
        } else {
            std::cout << "\n Grafico generato: speedup_plot_grid.png\n";
        }
    } else {
        std::cout << "\n Grafico non generato: servono almeno 3 valori di boid.\n";
    }

    return 0;
}
