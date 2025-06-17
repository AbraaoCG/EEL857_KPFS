#include <vector>
#pragma once

struct ForfeitSet {
    int nA;                     // Número de itens "permitidos"
    int forfeitCost;           // Penalidade caso exceda nA
    std::vector<int> items;    // IDs dos itens nesse conjunto
    ForfeitSet(int a, int pc, std::vector<int>& i) : nA(a), forfeitCost(pc), items(i) {}
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
    int penalidadeTotal;
    int valorObjetivo;
    double tempoMs;
};

struct Item {
    int id;
    int peso;
    int profit;
    

    Item(int _id, int p, int l ): id(_id), peso(p), profit(l) {}
};