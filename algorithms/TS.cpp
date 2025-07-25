#include "../utils/interpreter.hpp"
#include "../utils/functions.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <chrono>
#include <cfloat>
#include <algorithm>
#include <omp.h>
namespace fs = std::filesystem;




/**
 * @brief Executa o algoritmo de Busca Tabu para o Problema da Mochila com Conjuntos de Penalidade.
 * @param inst A instância do problema contendo itens, conjuntos e capacidade.
 * @param max_iter O número máximo de iterações que o algoritmo executará.
 * @param tabu_tenure A duração (em iterações) que um movimento permanecerá na lista tabu.
 * @return Um struct Resultado contendo a melhor solução encontrada e suas métricas.
 */
Resultado tabu_search(const Instance& inst, const fs::path& caminho, int max_iter=1000, int tabu_tenure=7) {

    // 1. Inicialização
    std::vector<bool> current_sol(inst.numItems, false);
    std::vector<bool> best_sol = current_sol;
    std::vector<int> tabu_list(inst.numItems, 0);

    double best_obj_value = get_objective_value(best_sol, inst);
    double current_obj_value = best_obj_value;
    
    int iters_without_improvement = 0;
    const int stopping_threshold = static_cast<int>(max_iter * 0.2);

    // <<< ALTERAÇÃO 1: O arquivo de log é aberto UMA VEZ, ANTES do loop.
    std::ofstream log_file(caminho);
    if (!log_file.is_open()) {
        std::cerr << "Aviso: Nao foi possivel abrir o arquivo de log para escrita: " << caminho << std::endl;
    } else {
        log_file << "Iteracao;ValorObjetivo;Peso\n";
    }


    
    // 2. Loop Principal da Busca Tabu
    for (int iter = 0; iter < max_iter; ++iter) {
        // --- PARALELIZAÇÃO COM OPENMP ---
        int best_move = -1;
        double best_neighbor_obj = -DBL_MAX;

        #pragma omp parallel
        {
            int local_best_move = -1;
            double local_best_obj = -DBL_MAX;

            // O laço 'for' é dividido entre as threads
            #pragma omp for nowait
            for (int j = 0; j < inst.numItems; ++j) {
                std::vector<bool> neighbor_sol = current_sol;
                neighbor_sol[j] = !neighbor_sol[j];
                double neighbor_obj = get_objective_value(neighbor_sol, inst);
                if (neighbor_obj == -DBL_MAX) continue;

                if (iter >= tabu_list[j]) { // Não é tabu
                    if (neighbor_obj > local_best_obj) {
                        local_best_obj = neighbor_obj;
                        local_best_move = j;
                    }
                } else { // É tabu -> Critério de Aspiração
                    if (neighbor_obj > best_obj_value) {
                        // Se um movimento aspirado for encontrado, ele tem alta prioridade
                        #pragma omp critical
                        {
                            if (neighbor_obj > best_neighbor_obj) {
                                best_neighbor_obj = neighbor_obj;
                                best_move = j;
                            }
                        }
                    }
                }
            }

            // Cada thread compara seu melhor resultado com o melhor global (entre as threads)
            #pragma omp critical
            {
                if (local_best_obj > best_neighbor_obj) {
                    best_neighbor_obj = local_best_obj;
                    best_move = local_best_move;
                }
            }
        }

        // 4. Atualiza a solução
        if (best_move != -1) {
            current_obj_value = best_neighbor_obj;
            current_sol[best_move] = !current_sol[best_move];
            tabu_list[best_move] = iter + tabu_tenure;

            if (current_obj_value > best_obj_value) {
                best_sol = current_sol;
                best_obj_value = best_neighbor_obj;
                iters_without_improvement = 0;
            } else {
                iters_without_improvement++;
            }
        } else {
            iters_without_improvement++;
        }
        int peso = 0;
        for (int k = 0; k < inst.numItems; ++k) {
            if (best_sol[k]){
                peso += inst.weights[k];
            }
        }
        if (log_file.is_open()) {
            log_file << iter + 1 << ";"  << best_obj_value << ";" << peso <<"\n";
        }

        if (iters_without_improvement >= stopping_threshold) {
            break;
        }
    }

    // 5. Finaliza e Prepara o Resultado

    Resultado resultado;
    resultado.itensSelecionados = best_sol;


    // Calcula as métricas finais da melhor solução para preencher o struct Resultado
    int final_lucro = 0;
    int final_peso = 0;
    for (int j = 0; j < inst.numItems; ++j) {
        if (best_sol[j]) {
            final_lucro += inst.profits[j];
            final_peso += inst.weights[j];
        }
    }

    int final_penalidade = 0;
    for (const auto& fs : inst.forfeitSets) {
        int contagem_no_conjunto = 0;
        for (int item_id : fs.items) {
            if (best_sol[item_id]) {
                contagem_no_conjunto++;
            }
        }
        int violacoes = std::max(0, contagem_no_conjunto - fs.nA);
        final_penalidade += violacoes * fs.forfeitCost;
    }
    
    resultado.lucroTotal = final_lucro;
    resultado.pesoTotal = final_peso;
    resultado.penalidadeTotal = final_penalidade;
    resultado.valorObjetivo = final_lucro - final_penalidade;

    return resultado;
}