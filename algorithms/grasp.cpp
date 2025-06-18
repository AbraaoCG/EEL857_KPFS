#include "../utils/interpreter.hpp"
#include "../utils/functions.hpp"
#include <algorithm>
#include <filesystem>
#include <random>
#include <numeric>
#include <chrono>
namespace fs = std::filesystem;

Resultado grasp(const Instance& inst, const fs::path& caminho, int maxIter = 1000, double alpha = 0.2) {
    std::mt19937 rng(std::random_device{}());
    Resultado melhorSol;
    melhorSol.valorObjetivo = -1e9;

    int limiteSemMelhora = std::max(1, maxIter / 10);
    int semMelhora = 0;

    // <<< ALTERAÇÃO 1: O arquivo de log é aberto UMA VEZ, ANTES do loop.
    std::ofstream log_file(caminho);
    if (!log_file.is_open()) {
        std::cerr << "Aviso: Nao foi possivel abrir o arquivo de log para escrita: " << caminho << std::endl;
    } else {
        log_file << "Iteracao;ValorObjetivo\n";
    }

    for (int it = 0; it < maxIter; ++it) {
        if (semMelhora >= limiteSemMelhora) {
            std::cout << "Criterio de parada ativado por estagnacao (sem melhora em " << semMelhora << " iteracoes consecutivas).\n";
            break;
        }

        // --- Fase de construção aleatorizada ---
        std::vector<bool> selecionado(inst.numItems, false);
        int pesoAtual = 0, lucroAtual = 0;

        // Lista de candidatos: item_id
        std::vector<int> candidatos(inst.numItems);
        std::iota(candidatos.begin(), candidatos.end(), 0);

        // Ordena por razão lucro/peso (ou alguma heurística)
        std::sort(candidatos.begin(), candidatos.end(), [&](int a, int b) {
            double ra = (double)inst.profits[a] / inst.weights[a];
            double rb = (double)inst.profits[b] / inst.weights[b];
            return ra > rb;
        });

        while (!candidatos.empty()) {
            // Cria a lista RCL
            int limiteRCL = std::max(1, (int)(alpha * candidatos.size()));
            std::uniform_int_distribution<int> dist(0, limiteRCL - 1);
            int escolhido = candidatos[dist(rng)];

            int item = candidatos[escolhido];
            if (pesoAtual + inst.weights[item] <= inst.capacity) {
                selecionado[item] = true;
                pesoAtual += inst.weights[item];
                lucroAtual += inst.profits[item];
            }

            candidatos.erase(candidatos.begin() + escolhido);
        }

        // --- Fase de busca local (melhoria por troca simples) ---
        bool melhorou = true;
        while (melhorou) {
            melhorou = false;
            int objetivoAtual = get_objective_value(selecionado, inst);
            int penalidadeAtual = lucroAtual - objetivoAtual;

            // Tenta adicionar
            for (int i = 0; i < inst.numItems; ++i) {
                if (!selecionado[i] && pesoAtual + inst.weights[i] <= inst.capacity) {
                    selecionado[i] = true;
                    int novoObjetivo = get_objective_value(selecionado, inst);
                    int novoLucro = lucroAtual + inst.profits[i];
                    int novaPenalidade = novoLucro - novoObjetivo;

                    if (novoObjetivo > objetivoAtual) {
                        pesoAtual += inst.weights[i];
                        lucroAtual = novoLucro;
                        melhorou = true;
                        break;
                    } else {
                        selecionado[i] = false;
                    }
                }
            }

            // Tenta remover
            for (int i = 0; i < inst.numItems; ++i) {
                if (selecionado[i]) {
                    selecionado[i] = false;
                    int novoObjetivo = get_objective_value(selecionado, inst);
                    int novoLucro = lucroAtual - inst.profits[i];
                    int novaPenalidade = novoLucro - novoObjetivo;

                    if (novoObjetivo > objetivoAtual) {
                        pesoAtual -= inst.weights[i];
                        lucroAtual = novoLucro;
                        melhorou = true;
                        break;
                    } else {
                        selecionado[i] = true;
                    }
                }
            }

            // Tenta trocar
            for (int i = 0; i < inst.numItems; ++i) {
                if (!selecionado[i]) {
                    for (int j = 0; j < inst.numItems; ++j) {
                        if (selecionado[j]) {
                            int novoPeso = pesoAtual - inst.weights[j] + inst.weights[i];
                            if (novoPeso > inst.capacity) continue;

                            selecionado[i] = true;
                            selecionado[j] = false;

                            int novoLucro = lucroAtual - inst.profits[j] + inst.profits[i];
                            int novoObjetivo = get_objective_value(selecionado, inst);
                            int novaPenalidade = novoLucro - novoObjetivo;

                            if (novoObjetivo > objetivoAtual) {
                                pesoAtual = novoPeso;
                                lucroAtual = novoLucro;
                                melhorou = true;
                                break;
                            } else {
                                selecionado[i] = false;
                                selecionado[j] = true;
                            }
                        }
                    }
                    if (melhorou) break;
                }
            }
        }

        // Avalia a solução
        int objetivo = get_objective_value(selecionado, inst);
        int penalidade = lucroAtual - objetivo;

        if (log_file.is_open()) {
            log_file << it + 1 << ";"  << objetivo << ";" << pesoAtual << "\n";
        }  
        
        if (objetivo > melhorSol.valorObjetivo) {
            melhorSol.itensSelecionados = selecionado;
            melhorSol.lucroTotal = lucroAtual;
            melhorSol.penalidadeTotal = objetivo - lucroAtual;
            melhorSol.valorObjetivo = objetivo;
            melhorSol.pesoTotal = pesoAtual;
            semMelhora = 0;
            std::cout << "Funcao-Objetivo melhor encontrada na iteracao " << it+1 << ": " << objetivo << " / Peso da mochila: " << pesoAtual << "/" << inst.capacity << "\n";
        } else {
            semMelhora++;
        }
    }

    return melhorSol;
}