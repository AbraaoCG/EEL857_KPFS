#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../include/types.hpp"

Instance lerInstancia(const std::string& caminho) {
    std::ifstream arq(caminho);
    if (!arq.is_open()) {
        throw std::runtime_error("Erro ao abrir o arquivo: " + caminho);
    }

    Instance inst;
    arq >> inst.numItems >> inst.numSets >> inst.capacity;

    inst.profits.resize(inst.numItems);
    for (int i = 0; i < inst.numItems; ++i) {
        arq >> inst.profits[i];
    }

    inst.weights.resize(inst.numItems);
    for (int i = 0; i < inst.numItems; ++i) {
        arq >> inst.weights[i];
    }

    inst.forfeitSets.resize(inst.numSets);
    for (int s = 0; s < inst.numSets; ++s) {
        int nA, fC, nI;
        arq >> nA >> fC >> nI;
        inst.forfeitSets[s].nA = nA;
        inst.forfeitSets[s].forfeitCost = fC;

        inst.forfeitSets[s].items.resize(nI);
        for (int j = 0; j < nI; ++j) {
            arq >> inst.forfeitSets[s].items[j];
        }
    }

    return inst;
}

void imprimirInstancia(const Instance& inst) {
    std::cout << "Itens: " << inst.numItems << ", Conjuntos: " << inst.numSets
              << ", Capacidade: " << inst.capacity << "\n";
    std::cout << "Lucros: ";
    for (int p : inst.profits) std::cout << p << " ";
    std::cout << "\nPesos: ";
    for (int w : inst.weights) std::cout << w << " ";
    std::cout << "\n\nForfeit Sets:\n";

    for (size_t i = 0; i < inst.forfeitSets.size(); ++i) {
        const auto& fs = inst.forfeitSets[i];
        std::cout << "Set " << i << ": nA=" << fs.nA
                  << ", Custo=" << fs.forfeitCost
                  << ", Itens=";
        for (int id : fs.items) std::cout << id << " ";
        std::cout << "\n";
    }
}
