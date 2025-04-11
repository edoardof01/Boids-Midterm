#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <vector>


double measureExecutionTime(const std::string& command, const std::string& env = "") {
    using Clock = std::chrono::high_resolution_clock;
    const auto start = Clock::now();
    const int ret = std::system((env + command).c_str());
    const auto end = Clock::now();

    if (ret != 0) {
        std::cerr << "Warning: comando \"" << command
                  << "\" terminato con codice di ritorno " << ret << '\n';
    }
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    constexpr int numBoids = 1600;
    constexpr int TRIALS = 5;

    std::vector<int> threadCounts;
    std::cout << "Inserisci i valori di thread da testare, max 32 (termina con 0): ";
    while (true) {
        int val;
        std::cin >> val;
        if (!std::cin || val <= 0) break;
        threadCounts.push_back(val);
    }

    if (threadCounts.empty()) {
        std::cerr << "Nessun valore inserito. Uscita.\n";
        return 1;
    }

    std::ofstream outNoGrid("threads_speedup_nogrid.txt");
    std::ofstream outGrid("threads_speedup_grid.txt");
    std::ofstream outSoA("threads_speedup_soa.txt");
    if (!outNoGrid || !outGrid || !outSoA) {
        std::cerr << "Errore apertura file di output\n";
        return 1;
    }

    std::string seqCmd = "./SeqHeadless " + std::to_string(numBoids);

    std::cout << "\n=== Eseguo versione sequenziale per riferimento ===\n";
    double seqTotal = 0.0;
    for (int i = 0; i < TRIALS; ++i)
        seqTotal += measureExecutionTime(seqCmd);
    double seqAvg = seqTotal / TRIALS;
    std::cout << "Tempo medio sequenziale: " << seqAvg << " s\n\n";

    for (int threads : threadCounts) {
        std::cout << "=== Thread: " << threads << " ===\n";
        std::string env = "OMP_NUM_THREADS=" + std::to_string(threads) + " ";

        double parNoGrid = 0.0;
        for (int i = 0; i < TRIALS; ++i)
            parNoGrid += measureExecutionTime("./ParHeadless " + std::to_string(numBoids), env);
        parNoGrid /= TRIALS;

        double parGrid = 0.0;
        for (int i = 0; i < TRIALS; ++i)
            parGrid += measureExecutionTime("./ParallelGrid " + std::to_string(numBoids), env);
        parGrid /= TRIALS;

        double parSoA = 0.0;
        for (int i = 0; i < TRIALS; ++i)
            parSoA += measureExecutionTime("./ParSOAHeadless " + std::to_string(numBoids), env);
        parSoA /= TRIALS;

        double speedupNoGrid = seqAvg / parNoGrid;
        double speedupGrid = seqAvg / parGrid;
        double speedupSoA = seqAvg / parSoA;

        std::cout << "[NoGrid] Speedup: " << speedupNoGrid << "\n";
        std::cout << "[Grid]   Speedup: " << speedupGrid << "\n";
        std::cout << "[SoA]    Speedup: " << speedupSoA << "\n";

        outNoGrid << threads << " " << speedupNoGrid << "\n";
        outGrid   << threads << " " << speedupGrid   << "\n";
        outSoA    << threads << " " << speedupSoA    << "\n";
    }

    outNoGrid.close();
    outGrid.close();
    outSoA.close();

    // Generazione automatica del grafico con gnuplot
    if (threadCounts.size() >= 3) {
        // Assicurati che la cartella esista (in genere già esiste in cmake)
        std::system("mkdir -p cmake-build-debug");

        const std::string gnuplotCommand =
            "gnuplot plot_speedup_threads.gp";

        std::cout << "\nGenero il grafico con Gnuplot...\n";
        if (int ret = std::system(gnuplotCommand.c_str()); ret != 0) {
            std::cerr << " Errore nell'esecuzione di gnuplot (assicurati che sia installato e nella PATH)\n";
        } else {
            std::system("xdg-open cmake-build-debug/speedup_threads_plot.png > /dev/null 2>&1 &");
        }
    } else {
        std::cout << "\n Grafico non generato: servono almeno 3 valori di thread.\n";
    }


    return 0;
}
