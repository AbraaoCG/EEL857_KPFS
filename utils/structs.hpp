#pragma once

struct ForfeitSet {
    int nA;                     // Número de itens "permitidos"
    int forfeitCost;           // Penalidade caso exceda nA
    std::vector<int> items;    // IDs dos itens nesse conjunto
};

struct Instance {
    int numItems;
    int numSets;
    int capacity;

    std::vector<int> profits;
    std::vector<int> weights;
    std::vector<ForfeitSet> forfeitSets;
};

struct Resultado {
    std::vector<bool> itensSelecionados;
    int lucroTotal;
    int pesoTotal;
    int penalidadeTotal;
    int valorObjetivo;
    double tempoMs;
};