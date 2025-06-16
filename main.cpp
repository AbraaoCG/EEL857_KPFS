#include "algorithms/grasp.cpp"
#include "algorithms/TS.cpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_instancia>\n";
        return 1;
    }

    std::string algoritmo = "tabu";
    std::string caminho = argv[1];
    try {
        Instance inst = lerInstancia(caminho);
        auto start = std::chrono::high_resolution_clock::now();

        Resultado res;
        if (algoritmo == "grasp") res = grasp(inst);
        // else if (algoritmo == "ils") res = ils(inst);
        // else if (algoritmo == "vns") res = vns(inst);
        else if (algoritmo == "tabu") res = tabu_search(inst);
        // else if (algoritmo == "genetic") res = genetic(inst);
        // else if (algoritmo == "sa") res = simulatedAnnealing(inst);
        else {
            std::cerr << "Algoritmo desconhecido.\n";
            return 1;
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.tempoMs = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "Lucro: " << res.lucroTotal
                << ", Penalidade: " << res.penalidadeTotal
                << ", Objetivo: " << res.valorObjetivo
                << ", Tempo: " << res.tempoMs << " ms\n";
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
