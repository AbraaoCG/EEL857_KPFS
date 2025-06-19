#include "algorithms/grasp.cpp"
#include "algorithms/TS.cpp"
#include "algorithms/GA.cpp"
#include <iostream>
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_instancia>\n";
        return 1;
    }

    std::string algoritmo = "genetic";
    std::string caminho = argv[1];
    try {
        Instance inst = lerInstancia(caminho);
        fs::path caminhoOutput = gerarCaminhoOutput(caminho, algoritmo);
        fs::create_directories(caminhoOutput.parent_path());

        auto start = std::chrono::high_resolution_clock::now();

        Resultado res;
        if (algoritmo == "grasp") res = grasp(inst, caminhoOutput);
        // else if (algoritmo == "ils") res = ils(inst);
        // else if (algoritmo == "vns") res = vns(inst);
        else if (algoritmo == "tabu") res = tabu_search(inst, caminhoOutput);
        else if(algoritmo == "genetic") res = genetic_algorithm(inst, caminhoOutput);
        // else if (algoritmo == "sa") res = simulatedAnnealing(inst);
        else {
            std::cerr << "Algoritmo desconhecido.\n";
            return 1;
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.tempoMs = std::chrono::duration<double, std::milli>(end - start).count();
        salvarResultado(caminhoOutput, res, inst);

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