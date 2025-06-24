#include "../utils/interpreter.hpp"
#include "../utils/functions.hpp"
#include <algorithm>
#include <filesystem>
#include <random>
#include <numeric>
#include <chrono>
#include <omp.h>
namespace fs = std::filesystem;

Resultado grasp(const Instance& inst, const fs::path& caminho, int maxIter = 1000, double alpha = 0.2) {
    Resultado melhorSol;
    melhorSol.valorObjetivo = -1e9;

    int limiteSemMelhora = std::max(1, maxIter / 10);
    int semMelhora = 0;

    std::ofstream log_file(caminho);
    if (!log_file.is_open()) {
        std::cerr << "Aviso: Nao foi possivel abrir o arquivo de log para escrita: " << caminho << std::endl;
    } else {
        log_file << "Iteracao;ValorObjetivo;Peso\n";
    }

    int iteracao = 0;
    const int blocoParalelo = 10;  // Número de iterações por bloco paralelo

    while (iteracao < maxIter && semMelhora < limiteSemMelhora) {
        int blocosExecutados = std::min(blocoParalelo, maxIter - iteracao);

        #pragma omp parallel
        {
            std::mt19937 rng(std::random_device{}() + omp_get_thread_num());
            Resultado melhorLocal;
            melhorLocal.valorObjetivo = -1e9;

            #pragma omp for schedule(dynamic)
            for (int i = 0; i < blocosExecutados; ++i) {
                std::vector<bool> selecionado(inst.numItems, false);
                int pesoAtual = 0, lucroAtual = 0;

                std::vector<int> candidatos(inst.numItems);
                std::iota(candidatos.begin(), candidatos.end(), 0);
                std::sort(candidatos.begin(), candidatos.end(), [&](int a, int b) {
                    return (double)inst.profits[a] / inst.weights[a] > (double)inst.profits[b] / inst.weights[b];
                });

                while (!candidatos.empty()) {
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

                bool melhorou = true;
                while (melhorou) {
                    melhorou = false;
                    int objetivoAtual = get_objective_value(selecionado, inst);

                    for (int i = 0; i < inst.numItems; ++i) {
                        if (!selecionado[i] && pesoAtual + inst.weights[i] <= inst.capacity) {
                            selecionado[i] = true;
                            int novoObjetivo = get_objective_value(selecionado, inst);
                            if (novoObjetivo > objetivoAtual) {
                                pesoAtual += inst.weights[i];
                                lucroAtual += inst.profits[i];
                                melhorou = true;
                                break;
                            } else {
                                selecionado[i] = false;
                            }
                        }
                    }

                    for (int i = 0; i < inst.numItems; ++i) {
                        if (selecionado[i]) {
                            selecionado[i] = false;
                            int novoObjetivo = get_objective_value(selecionado, inst);
                            if (novoObjetivo > objetivoAtual) {
                                pesoAtual -= inst.weights[i];
                                lucroAtual -= inst.profits[i];
                                melhorou = true;
                                break;
                            } else {
                                selecionado[i] = true;
                            }
                        }
                    }

                    for (int i = 0; i < inst.numItems; ++i) {
                        if (!selecionado[i]) {
                            for (int j = 0; j < inst.numItems; ++j) {
                                if (selecionado[j]) {
                                    int novoPeso = pesoAtual - inst.weights[j] + inst.weights[i];
                                    if (novoPeso > inst.capacity) continue;

                                    selecionado[i] = true;
                                    selecionado[j] = false;

                                    int novoObjetivo = get_objective_value(selecionado, inst);
                                    if (novoObjetivo > objetivoAtual) {
                                        pesoAtual = novoPeso;
                                        lucroAtual = lucroAtual - inst.profits[j] + inst.profits[i];
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

                int objetivo = get_objective_value(selecionado, inst);
                Resultado atual;
                atual.valorObjetivo = objetivo;
                atual.pesoTotal = pesoAtual;
                atual.lucroTotal = lucroAtual;
                atual.penalidadeTotal = objetivo - lucroAtual;
                atual.itensSelecionados = selecionado;

                #pragma omp critical
                {
                    if (log_file.is_open())
                        log_file << (iteracao + i + 1) << ";" << objetivo << ";" << pesoAtual << "\n";

                    if (atual.valorObjetivo > melhorSol.valorObjetivo) {
                        melhorSol = atual;
                        semMelhora = 0;
                        // std::cout << "Melhor FO ate agora (iteracao " << (iteracao + i + 1) << "): "
                                //   << melhorSol.valorObjetivo << " / Peso: "
                                //   << melhorSol.pesoTotal << "/" << inst.capacity << "\n";
                    } else {
                        semMelhora++;
                    }
                }
            }
        }

        iteracao += blocosExecutados;
    }

    return melhorSol;
}
