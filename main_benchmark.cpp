#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>

#include "utils/structs.hpp"
#include "utils/interpreter.hpp"
#include "algorithms/grasp.cpp"
// #include "algorithms/ils.cpp"
// #include "algorithms/vns.cpp"
#include "algorithms/TS.cpp"
<<<<<<< HEAD
#include "algorithms/GA.cpp"
=======
// #include "algorithms/genetic.cpp"
>>>>>>> e4813fbc3dec9ee9edaff393931619526fa59b79
// #include "algorithms/simulated_annealing.cpp"

namespace fs = std::filesystem;

// Gera o caminho correspondente em "outputs/"
fs::path gerarCaminhoOutput(const fs::path& inputPath, const std::string& algoritmo) {
    fs::path relativo = fs::relative(inputPath, "instances");
    fs::path outputPath = fs::path("outputs") / relativo;
    outputPath.replace_filename(outputPath.stem().string() + "_" + algoritmo + ".txt");
    return outputPath;
}

// Salva o resultado no arquivo correspondente
void salvarResultado(const fs::path& caminho, const Resultado& res, const Instance& inst) {
    fs::create_directories(caminho.parent_path());
    std::ofstream out(caminho, std::ios_base::app);

    // Adiciona um separador se o arquivo já tiver conteúdo, para melhor legibilidade
    if (out.tellp() > 0) {
        out << "\n----------------------------------------\n\n";
    }
    out << "Lucro total: " << res.lucroTotal << "\n";
    out << "Penalidade total: " << res.penalidadeTotal << "\n";
    out << "Valor objetivo: " << res.valorObjetivo << "\n";
    out << "Peso na Mochila:" << res.pesoTotal << "/" << inst.capacity <<"\n";
    out << "Tempo (ms): " << res.tempoMs << "\n";
    out << "Itens selecionados (por índice):\n";
    for (size_t i = 0; i < res.itensSelecionados.size(); ++i) {
        if (res.itensSelecionados[i])
            out << i << " ";
    }
    out << "\n";
}

int main() {
    std::string algoritmo;
    std::cout << "Selecione o algoritmo:\n";
    std::cout << "1 - GRASP\n";
    // std::cout << "2 - ILS\n";
    // std::cout << "3 - VNS\n";
    std::cout << "4 - Tabu Search\n";
    std::cout << "5 - Genetic Algorithm\n";
    // std::cout << "6 - Simulated Annealing\n";
    std::cout << ">> ";
    int opcao;
    std::cin >> opcao;

    switch (opcao) {
        case 1: algoritmo = "grasp"; break;
        case 2: algoritmo = "ils"; break;
        case 3: algoritmo = "vns"; break;
        case 4: algoritmo = "tabu"; break;
        case 5: algoritmo = "genetic"; break;
        case 6: algoritmo = "sa"; break;
        default:
            std::cerr << "Opção inválida.\n";
            return 1;
    }

    std::cout << "\nExecutando benchmark com algoritmo: " << algoritmo << "\n";

    for (const auto& entry : fs::recursive_directory_iterator("instances")) {
        if (!entry.is_regular_file()) continue;

        fs::path caminhoInstancia = entry.path();
        std::string nome = caminhoInstancia.filename().string();
        if (nome == "readme.txt" || nome == ".DS_Store")
            continue;

        if (caminhoInstancia.extension() != ".txt")
            continue;
        std::cout << "Processando: " << caminhoInstancia << "\n";

        try {
            Instance inst = lerInstancia(caminhoInstancia.string());
            fs::path caminhoOutput = gerarCaminhoOutput(caminhoInstancia, algoritmo);
            fs::create_directories(caminhoOutput.parent_path());

            auto start = std::chrono::high_resolution_clock::now();

            Resultado res;
            if (algoritmo == "grasp") res = grasp(inst, caminhoOutput);
            // else if (algoritmo == "ils") res = ils(inst);
            // else if (algoritmo == "vns") res = vns(inst);
            else if (algoritmo == "tabu") res = tabu_search(inst, caminhoOutput);
            else if (algoritmo == "genetic") res = genetic_algorithm(inst,caminhoOutput);
            // else if (algoritmo == "sa") res = simulatedAnnealing(inst);

            auto end = std::chrono::high_resolution_clock::now();
            res.tempoMs = std::chrono::duration<double, std::milli>(end - start).count();

            
            salvarResultado(caminhoOutput, res, inst);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao processar " << caminhoInstancia << ": " << e.what() << "\n";
        }
    }

    std::cout << "Benchmark finalizado.\n";
    return 0;
}
