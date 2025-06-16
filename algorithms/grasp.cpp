#include "../utils/interpreter.hpp"
#include <algorithm>
#include <random>
#include <numeric>
#include <chrono>

int calcularPenalidade(const Instance& inst, const std::vector<bool>& selecionado) {
    int total = 0;

    for (const auto& fs : inst.forfeitSets) {
        int count = 0;
        for (int item : fs.items) {
            if (selecionado[item]) count++;
        }
        if (count > fs.nA) total += fs.forfeitCost;
    }

    return total;
}

Resultado grasp(const Instance& inst, int maxIter = 2000, double alpha = 0.2) {
    std::mt19937 rng(std::random_device{}());
    Resultado melhorSol;
    melhorSol.valorObjetivo = -1e9;

    for (int it = 0; it < maxIter; ++it) {
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
            for (int i = 0; i < inst.numItems; ++i) {
                if (!selecionado[i] && pesoAtual + inst.weights[i] <= inst.capacity) {
                    // Tenta adicionar o item i
                    selecionado[i] = true;
                    int novoLucro = lucroAtual + inst.profits[i];
                    int novaPenalidade = calcularPenalidade(inst, selecionado);
                    int novoObjetivo = novoLucro - novaPenalidade;

                    int atualObjetivo = lucroAtual - calcularPenalidade(inst, selecionado);

                    if (novoObjetivo > atualObjetivo) {
                        pesoAtual += inst.weights[i];
                        lucroAtual = novoLucro;
                        melhorou = true;
                    } else {
                        selecionado[i] = false; // Reverte
                    }
                }
            }
        }

        // Avalia a solução
        int penalidade = calcularPenalidade(inst, selecionado);
        int objetivo = lucroAtual - penalidade;

        if (objetivo > melhorSol.valorObjetivo) {
            melhorSol.itensSelecionados = selecionado;
            melhorSol.lucroTotal = lucroAtual;
            melhorSol.penalidadeTotal = penalidade;
            melhorSol.valorObjetivo = objetivo;
        }
    }

    return melhorSol;
}
