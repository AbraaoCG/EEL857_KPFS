#include<iostream> 
#include<fstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cfloat> // Para DBL_MIN
#include <cstdlib> // Para system()
#include "filepath.h"
#include "include/types.hpp"
#include "algorithms/GA.cpp"
using namespace std;

int main(){
    int nI = 0; //número de itens
    int nP = 0; //número de conjuntos de penalidade
    int kS = 0; //Capacidade de mochila
    int k_global = 1000;
    bool case_select_mode = 1;
    vector<Item> items;
    vector<ForfeitSet> forfeit_sets;
    string filepath, cenario, sc, tipo, tamanho, caso;
    pair<string, string> par_string;
    fstream newfile;

    // Modo de seleção manual
    if (case_select_mode == 0) { 
        //Escolha do cenário
        cout << "Escolha o cenário:" << endl;
        cout << "1: scenario1" << endl;
        cout << "2: scenario2" << endl;
        cout << "3: scenario3" << endl;
        cout << "4: scenario4" << endl;
        cin >> cenario;

        par_string = get_cenario(cenario, sc);
        cenario = par_string.first;
        
        if (cenario == "erro") {
            cerr << "Erro: Informe valores entre 1 e 4!";
            return 1;
        }
        sc = par_string.second;

        //Escolha do tipo de cenário
        cout << "Escolha o tipo do cenário:" << endl;
        cout << "1: correlated" << endl;
        cout << "2: fully correlated" << endl;
        cout << "3: not correlated" << endl;
        cin >> tipo;

        tipo = get_tipo(tipo);
        if (tipo == "erro") {
            cerr << "Erro: Informe valores entre 1 e 3!";
            return 1;
        }
        
        // TODO: Implementar a escolha do tamanho(300 até 1000) e do arquivo txt
        cout << "Escolha o tamanho do problema em número de itens:" << endl;
        cout << "1: 300" << endl;
        cout << "2: 500" << endl;
        cout << "3: 700" << endl;
        cout << "4: 800" << endl;
        cout << "5: 1000" << endl;
        cin >> tamanho;

        tamanho = get_tamanho(tamanho);
        if (tamanho == "erro") {
            cerr << "Erro: Informe valores entre 1 e 5!";
            return 1;
        }

        cout << "Escolha o caso desejado de 1 a 10:" << endl;
        cin >> caso;  
    // Modo de seleção fixa
    }
    else if (case_select_mode == 1) {
        cout << "Seleção Fixa de caso." << endl;
        cenario = "scenario1";
        sc = "sc1";
        tipo = "correlated";
        tamanho = "300";
        caso = "1";
    }
    // Modo de seleção - Rodar todos os casos
    else if(case_select_mode == 2) {
        return 1;
    }
    else {
        cerr << "Erro: Modo de seleção inválido!" << endl;
        return 1; // Retorna um código de erro indicando falha
    }
    
    #ifdef _WIN32
        filepath = "instances\\" + cenario + "\\" + tipo + "_" + sc + "\\"+ tamanho+"\\kpfs_"+caso+".txt";
    #else
        filepath = "instances/" + cenario + "/" + tipo + "_" + sc + "/"+tamanho+"/kpfs_"+caso+".txt";
    #endif
    
    cout << filepath << endl;

    newfile.open(filepath, ios_base::in);
    
    if(newfile.is_open()) {
        cout << "Arquivo aberto com sucesso." << endl;
        newfile >> nI >> nP >> kS;


        vector<int> profits_temp(nI);
        vector<int> pesos_temp(nI);
        items.reserve(nI);

        //Lendo os valores de cada item
        for (int i = 0; i < nI; i++) {
            newfile >> profits_temp[i];
        }
        //Lendo os pesos de cada item
        for (int i = 0; i < nI; i++) {
            newfile >> pesos_temp[i];
        }

        //Criando os objetos Item
        for (int i = 0; i < nI; ++i) {
            items.emplace_back(i, pesos_temp[i], profits_temp[i]);
        }

        forfeit_sets.reserve(nP);
        for (int i = 0; i < nP; ++i) {
            int allowance, penalty_cost, num_items_in_set;
            // Lê a linha de metadados do conjunto: nA_i, fC_i, nI_i
            newfile >> allowance >> penalty_cost >> num_items_in_set;

            vector<int> item_ids_in_set;
            item_ids_in_set.reserve(num_items_in_set);
            
            // Lê a linha com os IDs dos itens do conjunto
            for (int j = 0; j < num_items_in_set; ++j) {
                int item_id;
                newfile >> item_id;
                item_ids_in_set.push_back(item_id);
            }
            
            forfeit_sets.emplace_back(allowance, penalty_cost, item_ids_in_set);
        }

        newfile.close(); // Fecha o arquivo após terminar de usá-lo
        cout << "Arquivo fechado." << endl;
    } else {
        cerr << "Erro: O arquivo não pôde ser aberto!" << endl;
        return 1; // Retorna um código de erro indicando falha
    }

    cout << nI << endl;
    cout << nP << endl;
    cout << kS << endl;
   

    // int max_iterations = 1000;
    // int tabu_tenure = 7; 
    // // Execução
    // tabu_search(items, forfeit_sets, kS, k_global, max_iterations, tabu_tenure);


    int population_size = 20;
    int max_generations = 1000;
    double crossover_rate = 0.8;
    double mutation_rate = 0.25;
    bool track_evolution = 1;
    genetic_algorithm(items, forfeit_sets, kS, k_global, population_size, max_generations, crossover_rate, mutation_rate,track_evolution,cenario, tipo, tamanho, caso,sc);

    return 0;
}