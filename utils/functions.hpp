#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cfloat>
#include "structs.hpp"


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

double get_objective_value(const std::vector<bool>& sol, const Instance& inst) {
    long long total_weight = 0;
    int total_profit = 0;

    for (int j = 0; j < inst.numItems; ++j) {
        if (sol[j]) {
            total_profit += inst.profits[j];
            total_weight += inst.weights[j];
        }
    }

    // Se o peso excede a capacidade, a solução é inviável.
    if (total_weight > inst.capacity) {
        return -DBL_MAX;
    }

    int total_penalty = 0;
    for (const auto& fs : inst.forfeitSets) {
        int count_in_set = 0;
        for (int item_id : fs.items) {
            if (sol[item_id]) {
                count_in_set++;
            }
        }
        int violations = std::max(0, count_in_set - fs.nA);
        total_penalty += violations * fs.forfeitCost;
    }

    return static_cast<double>(total_profit - total_penalty);
}

// Função auxiliar para cálculo do peso.
int calcularPeso(const Instance &inst, const std::vector<bool> &best_sol) {
    int final_weight = 0;
    int items_count = 0;

    for (size_t i = 0; i < best_sol.size(); ++i) {
        if (best_sol[i]) {
            final_weight += inst.weights[i];
            items_count++;
        }
    }
    return final_weight;
}
