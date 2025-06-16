#include<iostream> 
#include<fstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cfloat> // Para DBL_MIN
#include "filepath.h"


using namespace std;

struct Item {
    int id;
    int peso;
    int profit;
    

    Item(int _id, int p, int l ): id(_id), peso(p), profit(l) {}
};

struct ConjuntoPenalidade{
    int id;
    int limiteItens; //Limite de itens que podem ser escolhidos deste conjunto sem penalidade.
    int penalidade; //Valor da penalidade associada a este conjunto
    int cardinalidade; //Cardinalidade do conjunto de penalidade
    vector<int> itens; //IDs dos itens que pertencem a este conjunto de penalidade

    ConjuntoPenalidade(int _id, int _limiteItens, int _penalidade, int _cardinalidade, vector<int> _itens): id(_id), limiteItens(_limiteItens), penalidade(_penalidade), cardinalidade(_cardinalidade), itens(_itens) {}
};

//TODO criar a estrutura dos conjuntos de penalidade (nP)
struct ForfeitSet {
    int allowance;    // nA_i do readme, h_i do PDF
    int penalty_cost; // fC_i do readme, d_i do PDF
    vector<int> items; // id_s_i do readme, C_i do PDF

    ForfeitSet(int a, int pc, vector<int>& i) : allowance(a), penalty_cost(pc), items(i) {}
};

double calculate_objective(const vector<bool>& sol, const vector<Item>& items, const vector<ForfeitSet>& forfeit_sets, int knapsack_capacity, int max_global_violations) {
    double total_profit = 0;
    double total_weight = 0;

    for (size_t j = 0; j < items.size(); ++j) {
        if (sol[j]) {
            total_profit += items[j].profit;
            total_weight += items[j].peso;
        }
    }

    // 1. Restrição de capacidade da mochila 
    if (total_weight > knapsack_capacity) {
        return DBL_MIN; // Solução inviável
    }
    double total_penalty_cost = 0;
    int total_violations = 0;

    for (const auto& fs : forfeit_sets) {
        int items_in_set_count = 0;
        for (int item_id : fs.items) {
            if (sol[item_id]) {
                items_in_set_count++;
            }
        }
        // Calcula as violações para o conjunto C_i 
        int violations_v_i = max(0, items_in_set_count - fs.allowance);
        total_violations += violations_v_i;
        total_penalty_cost += static_cast<double>(violations_v_i) * fs.penalty_cost;
    }

    // 2. Restrição de limite global de violações 
    if (total_violations > max_global_violations) {
        return DBL_MIN; // Solução inviável
    }
    
    // Função Objetivo: Lucro - Penalidades 
    return total_profit - total_penalty_cost;
}

void tabu_search(const vector<Item>& items,
                 const vector<ForfeitSet>& forfeit_sets,
                 int knapsack_capacity,
                 int max_global_violations,
                 int max_iterations,
                 int tabu_tenure) {

    int n_items = items.size();

    // 1. Solução Inicial (vazia, sempre viável)
    vector<bool> current_sol(n_items, false);
    vector<bool> best_sol = current_sol;

    double best_obj_value = calculate_objective(best_sol, items, forfeit_sets, knapsack_capacity, max_global_violations);
    
    // 2. Lista Tabu (armazena a iteração em que o item deixa de ser tabu)
    vector<int> tabu_list(n_items, 0);

    cout << "Iteração 0: Melhor Obj = " << best_obj_value << endl;

    // 3. Loop Principal
    for (int iter = 1; iter <= max_iterations; ++iter) {
        vector<bool> best_neighbor_sol;
        double best_neighbor_obj = DBL_MIN;
        int best_move_item = -1;

        // 4. Explorar a vizinhança (1-flip)
        for (int j = 0; j < n_items; ++j) {
            vector<bool> neighbor_sol = current_sol;
            neighbor_sol[j] = !neighbor_sol[j]; // Movimento de "flip"

            double neighbor_obj = calculate_objective(neighbor_sol, items, forfeit_sets, knapsack_capacity, max_global_violations);

            // Se o movimento não é tabu
            if (iter >= tabu_list[j]) {
                if (neighbor_obj > best_neighbor_obj) {
                    best_neighbor_obj = neighbor_obj;
                    best_neighbor_sol = neighbor_sol;
                    best_move_item = j;
                }
            } 
            // Critério de Aspiração
            else {
                if (neighbor_obj > best_obj_value) {
                    best_neighbor_obj = neighbor_obj;
                    best_neighbor_sol = neighbor_sol;
                    best_move_item = j;
                }
            }
        }

        if (best_move_item != -1) {
            // Atualiza a solução atual
            current_sol = best_neighbor_sol;
            
            // Atualiza a lista tabu
            tabu_list[best_move_item] = iter + tabu_tenure;

            // Atualiza a melhor solução encontrada
            if (best_neighbor_obj > best_obj_value) {
                best_sol = best_neighbor_sol;
                best_obj_value = best_neighbor_obj;
                cout << "Iteração " << iter << ": Novo Melhor Obj = " << best_obj_value << endl;
            }
        } else {
            cout << "Nenhum movimento válido encontrado na iteração " << iter << ". Parando." << endl;
            break;
        }
    }

    // Imprime os resultados finais
    cout << "\n--- Busca Tabu Finalizada ---" << endl;
    cout << "Melhor valor objetivo encontrado: " << best_obj_value << endl;
    cout << "Itens na solução final: ";
    double final_weight = 0;
    int items_count = 0;
    for(size_t i = 0; i < best_sol.size(); ++i) {
        if(best_sol[i]) {
            cout << items[i].id << " ";
            final_weight += items[i].peso;
            items_count++;
        }
    }
    cout << "\nNúmero de itens: " << items_count << endl;
    cout << "Peso total: " << final_weight << "/" << knapsack_capacity << endl;
}

// vector<bool> generate_random_solution(const vector<Item>& items, int knapsack_capacity) {
//     vector<bool> solution(items.size(), false);
//     vector<int> indices(items.size());

//     // Preencher índices dos itens
//     iota(indices.begin(), indices.end(), 0);

//     // Embaralhar os índices para adicionar itens aleatoriamente
//     random_shuffle(indices.begin(), indices.end());

//     int current_weight = 0;

//     for (int idx : indices) {
//         // Adicionar item somente se não exceder o limite
//         if (current_weight + items[idx].peso <= knapsack_capacity) {
//             solution[idx] = true;
//             current_weight += items[idx].peso;
//         }
//     }

//     return solution;
// }

// vector<vector<bool>> initialize_population_random(const vector<Item>& items, int knapsack_capacity, int population_size) {
//     vector<vector<bool>> population;
//     population.reserve(population_size);

//     for (int i = 0; i < population_size; ++i) {
//         // Gerar uma solução inicial aleatória válida
//         population.push_back(generate_random_solution(items, knapsack_capacity));
//     }

//     return population;
// }
vector<bool> roulette_wheel_selection(const vector<vector<bool>>& population, 
                                      const vector<double>& fitness) {
    // 1. Calcular o somatório de fitness
    double total_fitness = accumulate(fitness.begin(), fitness.end(), 0.0);

    // 2. Gerar um valor aleatório no intervalo [0, total_fitness)
    double random_value = ((double)rand() / RAND_MAX) * total_fitness;

    // 3. Percorrer a população acumulando fitness
    double cumulative_sum = 0.0;
    for (size_t i = 0; i < population.size(); ++i) {
        cumulative_sum += fitness[i];
        if (cumulative_sum >= random_value) {
            return population[i];
        }
    }

    // 4. Retornar o último indivíduo por segurança (nunca deve chegar aqui)
    return population.back();
}
void genetic_algorithm(const vector<Item>& items,
                       const vector<ForfeitSet>& forfeit_sets,
                       int knapsack_capacity,
                       int max_global_violations,
                       int population_size,
                       int max_generations,
                       double crossover_rate,
                       double mutation_rate,
                       bool track_evolution) {
    int n_items = items.size();
    vector<vector<bool>> population(population_size, vector<bool>(n_items));
    vector<double> fitness(population_size, DBL_MIN);
    vector<int> generation_best_fitness_history(max_generations, DBL_MIN);

    // Inicializar população com soluções zeradas
    for (int i = 0; i < population_size; ++i){
        for (int j = 0; j < n_items; ++j){
            population[i][j] = 0;
        }
        fitness[i] = calculate_objective(population[i], items, forfeit_sets, knapsack_capacity, max_global_violations);
    }

    // 2. Loop principal do algoritmo genético
    for (int generation = 0; generation < max_generations; ++generation) {
        vector<vector<bool>> new_population;

        // 2.2 Reprodução
        while (new_population.size() < population_size) {
            vector<bool> parent1 = roulette_wheel_selection(population, fitness);
            vector<bool> parent2 = roulette_wheel_selection(population, fitness);

            // Crossover com probabilidade definida
            if ((double)rand() / RAND_MAX < crossover_rate) {
                int crossover_point = rand() % n_items;
                vector<bool> child1 = parent1, child2 = parent2;

                for (int i = crossover_point; i < n_items; ++i) {
                    swap(child1[i], child2[i]);
                }
                
                new_population.push_back(child1);
                if (new_population.size() < population_size) {
                    new_population.push_back(child2);
                }
            } else {
                new_population.push_back(parent1);
                if (new_population.size() < population_size) {
                    new_population.push_back(parent2);
                }
            }
        }

        // 2.3 Mutação
        for (auto& individual : new_population) {
            if ((double)rand() / RAND_MAX < mutation_rate) {
                int mutation_point = rand() % n_items;
                individual[mutation_point] = !individual[mutation_point]; // Flip do bit
            }
        }
        // 2.4 Avaliação da nova população
        for (int i = 0; i < population_size; ++i) {
            fitness[i] = calculate_objective(new_population[i], items, forfeit_sets, knapsack_capacity, max_global_violations);
        }

        // 2.5 Substituição da população antiga
        population = new_population;

        if(track_evolution){
            // 2.6 Encontrar melhor solução da geração atual
            double generation_best_fitness = DBL_MIN;
            vector<bool> generation_best_sol;
            for (int i = 0; i < population_size; ++i) {
                if (fitness[i] > generation_best_fitness) {
                    generation_best_fitness = fitness[i];
                    generation_best_sol = population[i];
                }
            }
            generation_best_fitness_history[generation] =  generation_best_fitness;
        }
    }

    // Encontrar e imprimir a melhor solução geral
    double best_fitness = DBL_MIN;
    vector<bool> best_sol;
    for (int i = 0; i < population_size; ++i) {
        if (fitness[i] > best_fitness) {
            best_fitness = fitness[i];
            best_sol = population[i];
        }
    }

    cout << "\n--- Algoritmo Genético Finalizado ---" << endl;
    cout << "Melhor valor objetivo encontrado: " << best_fitness << endl;
    cout << "Itens na solução final: ";
    double final_weight = 0;
    int items_count = 0;
    for (size_t i = 0; i < best_sol.size(); ++i) {
        if (best_sol[i]) {
            cout << items[i].id << " ";
            final_weight += items[i].peso;
            items_count++;
        }
    }
    cout << "\nNúmero de itens: " << items_count << endl;
    cout << "Peso total: " << final_weight << "/" << knapsack_capacity << endl;

    // // TODO: Gráfico de evolução nas gerações a ser construído
    // if(track_evolution){
        
    // }
}

    /*
    for(const auto& item : items) {
        std::cout << "ID: " << item.id << ", Peso: " << item.peso << ", Profit: " << item.profit << std::endl;
    }

   cout << "\n--- Conjuntos de Penalidade ---" << endl;
    for(size_t i = 0; i < forfeit_sets.size(); ++i) {
        cout << "Conjunto " << i << ": "
             << "Permitidos (h): " << forfeit_sets[i].allowance << ", "
             << "Custo (d): " << forfeit_sets[i].penalty_cost << ", "
             << "Itens: { ";
        for(size_t j = 0; j < forfeit_sets[i].items.size(); ++j) {
            cout << forfeit_sets[i].items[j] << (j == forfeit_sets[i].items.size() - 1 ? "" : ", ");
        }
        cout << " }" << endl;
    }*/


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
        cout << "Seleção automática de casos." << endl;
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


    int population_size = 1000;
    int max_generations = 1000;
    double crossover_rate = 0.8;
    double mutation_rate = 0.05;
    bool track_evolution = 0;
    genetic_algorithm(items, forfeit_sets, kS, k_global, population_size, max_generations, crossover_rate, mutation_rate,track_evolution);

    return 0;
}