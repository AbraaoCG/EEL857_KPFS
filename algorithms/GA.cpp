#include <vector>
#include <numeric>
#include <cfloat> // Para DBL_MIN
#include "../utils/visualizer.hpp"
#include "../include/types.hpp"
#include <filesystem>

using namespace std;

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


typedef pair<int,int> parValorPeso;

void genetic_algorithm(const vector<Item>& items,
                       const vector<ForfeitSet>& forfeit_sets,
                       int knapsack_capacity,
                       int max_global_violations,
                       int population_size,
                       int max_generations,
                       double crossover_rate,
                       double mutation_rate,
                       bool track_evolution,
                       std::string cenario,
                       std::string tipo,
                       std::string tamanho,
                       std::string caso,
                       std::string sc
                       ) {
    
    int n_items = items.size();
    vector<vector<bool>> population(population_size, vector<bool>(n_items));
    vector<double> fitness(population_size, DBL_MIN);
    vector<parValorPeso> generation_best_fitness_history(max_generations, make_pair(-1000, -1000));

    // Inicializar população com soluções zeradas
    for (int i = 0; i < population_size; ++i){
        for (int j = 0; j < n_items; ++j){
            population[i][j] = 0;
        }


        // fitness[i] = calculate_objective(population[i], items, forfeit_sets, knapsack_capacity, max_global_violations);
        fitness[i] = get_objective_value(const std::vector<bool>& sol, const Instance& inst)
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

        // Caso seja desejável armazenar a evolução da população
        if(track_evolution){
            double generation_best_fitness = DBL_MIN;
            vector<bool> generation_best_sol(n_items, false);
            for (int i = 0; i < population_size; ++i) {
                if (fitness[i] > generation_best_fitness) {
                    generation_best_fitness = fitness[i];
                    generation_best_sol = population[i];
                }
            }
            int final_weight = 0;
            for (int i = 0; i < generation_best_sol.size(); ++i) {
                if (generation_best_sol[i]) {
                    final_weight += items[i].peso;
                }
            }
            generation_best_fitness_history[generation].first =  generation_best_fitness;
            generation_best_fitness_history[generation].second =  final_weight;
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

    // Plotar o histórico de fitness e peso caso necessário.
    if(track_evolution ){
        string plot_title = "Population_" + std::to_string(population_size) + "_MaxGen_" + std::to_string(max_generations) + "_CrossOverRate_" + std::to_string(crossover_rate) + "_MutRate_" + std::to_string(mutation_rate);
        string filePath = "results/" + cenario + "/" + tipo + "_" + sc + "/"+tamanho+"/caso" +caso + "/plots/";
        
        if (!std::filesystem::exists(filePath)) {
            std::filesystem::create_directories(filePath);
        }

        writeAndPlotExecution(generation_best_fitness_history,filePath+plot_title);
    }

}