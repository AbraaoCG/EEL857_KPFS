#include <vector>
#include <numeric>
#include <cfloat> // Para DBL_MIN
#include "../utils/structs.hpp"
#include <filesystem>
#include "../utils/functions.hpp"
#include <omp.h>  // Para OpenMP

namespace fs = std::filesystem;
using namespace std;



// Função para seleção de parentes com método da roleta. (Indivíduos com maior fitness/Valor na mochila tem mais chance de serem escolhidos).
vector<bool> roulette_wheel_selection(const vector<vector<bool>> &population, const vector<double> &fitness) {
    double total_fitness = accumulate(fitness.begin(), fitness.end(), 0.0);
    double random_value = ((double)rand() / RAND_MAX) * total_fitness;
    double cumulative_sum = 0.0;

    for (size_t i = 0; i < population.size(); ++i) {
        cumulative_sum += fitness[i];
        if (cumulative_sum >= random_value) {
            return population[i];
        }
    }
    return population.back();
}

// Função auxiliar para cálculo do peso.
int calcularPeso(const Instance &inst, const vector<bool> &best_sol) {
    double final_weight = 0;
    int items_count = 0;

    for (size_t i = 0; i < best_sol.size(); ++i) {
        if (best_sol[i]) {
            final_weight += inst.weights[i];
            items_count++;
        }
    }
    return final_weight;
}

// Função para cálculo da função objetivo. Difere da função definida em 'utils.hpp',
// pois ao invés de retornar fitness o menor possível(-DBL_MAX) para um candidato que extrapola
// o peso da mochila, retorna fitness próximo de 0.(DBL_MIN).

// Essa decisão de implementação foi tomada de forma experimental, pois observa-se que penalizar 
// grandemente candidatos com alto valor na mochila, mas que extrapolavam o peso, tornava
// dificil para esses serem selecionados como pais. Como consequencia, alguns candidatos com potencial
// para gerarem filhos com soluções ótimas eram 'descartados'.
double calculate_objective(const vector<bool>& sol,const Instance &inst  ) {
    double total_profit = 0;
    double total_weight = 0;

    for (size_t j = 0; j < inst.numItems; ++j) {
        if (sol[j]) {
            total_profit += inst.profits[j];
            total_weight += inst.weights[j];
        }
    }

    // 1. Restrição de capacidade da mochila 
    if (total_weight > inst.capacity) {
        return -DBL_MAX; // Solução inviável
    }
    double total_penalty_cost = 0;
    int total_violations = 0;

    for (const auto& fs : inst.forfeitSets) {
        int items_in_set_count = 0;
        for (int item_id : fs.items) {
            if (sol[item_id]) {
                items_in_set_count++;
            }
        }
        // Calcula as violações para o conjunto C_i 
        int violations_v_i = max(0, items_in_set_count - fs.nA);
        total_violations += violations_v_i;
        total_penalty_cost += static_cast<double>(violations_v_i) * fs.forfeitCost;
    }
    
    // Função Objetivo: Lucro - Penalidades 
    return total_profit - total_penalty_cost;
}



Resultado genetic_algorithm_parallel(const Instance &inst, 
    const fs::path &caminho, 
    int population_size = 500, 
    int max_generations = 500, // Numero máximo de gerações. Também impacta o critério de parada por estagnação
    double crossover_rate = 0.9, // Taxa de Crossover entre pais. Caso não haja crossover, um ou dois pais vão para próxima geração.
    double mutation_rate = 0.15, // Taxa de mutação aleatória.
    double maxGenEstagnated = 0.2, // Valor % do máximo de gerações estagnadas para parar o algorítmo.
    double threshold = 0.001, // Valor de diferença % para se considerar estagnação
    bool verbose = 1) {

    int n_items = inst.numItems;
    vector<vector<bool>> population(population_size, vector<bool>(n_items));
    vector<double> fitness(population_size, -1e9);
    Resultado melhorSol;
    melhorSol.valorObjetivo = -1e9;
    
    if(inst.numItems == 0){
        cout << "Arquivo com problema! ";
        return melhorSol;
    }

    // Inicializar população com soluções zeradas ( Pode-se paralelizar ).
    #pragma omp parallel for
    for (int i = 0; i < population_size; ++i) {
        for (int j = 0; j < n_items; ++j) {
            population[i][j] = 0;
        }
        fitness[i] = calculate_objective(population[i], inst);
    }

    // Abrir arquivo para escrita de resultado.
    std::ofstream log_file(caminho);
    if (!log_file.is_open()) {
        std::cerr << "Aviso: Nao foi possivel abrir o arquivo de log para escrita: " << caminho << std::endl;
    } else {
        log_file << "Iteracao;ValorObjetivo;Peso\n";
    }

    // 2. Loop principal do algoritmo genético
    int ultimaGeracaoDeMelhora = -1;
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
            } else { // Caso não haja, os pais são vão para a nova população
                new_population.push_back(parent1);
                if (new_population.size() < population_size) {
                    new_population.push_back(parent2);
                }
            }
        }
        // 2.3 Mutação (Pode-se paralelizar)
        #pragma omp parallel for
        for (auto &individual : new_population) {
            if ((double)rand() / RAND_MAX < mutation_rate) {
                int mutation_point = rand() % n_items;
                individual[mutation_point] = !individual[mutation_point];
            }
        }
        // 2.41 Avaliação da nova população: Cálculo de fitness (paralelizado)
        #pragma omp parallel for
        for (int i = 0; i < population_size; ++i) {
            fitness[i] = calculate_objective(new_population[i], inst);
        }

        // 2.42 Depois do loop em paralelo, fazer a busca da melhor solução sequencialmente.
        double generation_best_fitness = -1e9;
        vector<bool> generation_best_sol;
        for (int i = 0; i < population_size; ++i) {
            if (fitness[i] > generation_best_fitness) {
                generation_best_fitness = fitness[i];
                generation_best_sol = new_population[i]; // ← CORREÇÃO DO SEU BUG!
            }
        }
        int pesoTotal = calcularPeso(inst, generation_best_sol);

        if (log_file.is_open()) {
            log_file << generation + 1 << ";" << generation_best_fitness << ";" << pesoTotal << "\n";
        }
        // Armazenar melhor de todas as gerações
        if (generation_best_fitness > melhorSol.valorObjetivo) {
            int ultimaSolOtima = melhorSol.valorObjetivo;
            melhorSol.valorObjetivo = generation_best_fitness;
            melhorSol.itensSelecionados = generation_best_sol;
            melhorSol.pesoTotal = pesoTotal;
            double melhoraNaFuncaoObjetivo;
            // Caso valor objetivo seja diferente de 0
            if (melhorSol.valorObjetivo != 0) {
                // calcula uma melhora percentual do valor objetivo
                melhoraNaFuncaoObjetivo = (melhorSol.valorObjetivo - ultimaSolOtima) / melhorSol.valorObjetivo;
            } else {
                // Caso seja 0, apenas considere que o algorítimo deve continuar 
                melhoraNaFuncaoObjetivo = threshold * 1.1;
            }
            // Caso a melhora seja maior que um limite, considera que houve melhora.
            if (melhoraNaFuncaoObjetivo > threshold) {
                ultimaGeracaoDeMelhora = generation;
            }
        }
        // Caso o algorítimo esteja estagnado por muito tempo, calcula pendências da melhor solução 
        // e a retorna.
        if (generation - ultimaGeracaoDeMelhora > maxGenEstagnated * max_generations) {
            melhorSol.penalidadeTotal = calcularPenalidade(inst, melhorSol.itensSelecionados);
            melhorSol.lucroTotal = melhorSol.valorObjetivo + melhorSol.penalidadeTotal;

            if (verbose) {
                cout << "GA interrompido em " << generation << " gerações.\n";
                cout << "\n--- Algoritmo Genético Finalizado ---" << endl;
                cout << "Melhor valor objetivo encontrado: " << melhorSol.valorObjetivo << endl;
                cout << "Itens na solução final: ";
                cout << "\nNúmero de itens: " << inst.numItems << endl;
                cout << "Peso total: " << melhorSol.pesoTotal << "/" << inst.capacity << endl;
            }
            
            return melhorSol;
        }

        population = new_population;
    }
    // Caso o algorítimo alcançe o número máximo de gerações, retorna a melhor solução.
    if (verbose) {
        cout << "\n--- Algoritmo Genético Finalizado ---" << endl;
        cout << "Melhor valor objetivo encontrado: " << melhorSol.valorObjetivo << endl;
        cout << "Itens na solução final: ";
        cout << "\nNúmero de itens: " << inst.numItems << endl;
        cout << "Peso total: " << melhorSol.pesoTotal << "/" << inst.capacity << endl;
    }

    melhorSol.penalidadeTotal = calcularPenalidade(inst, melhorSol.itensSelecionados);
    melhorSol.lucroTotal = melhorSol.valorObjetivo + melhorSol.penalidadeTotal;

    return melhorSol;
}



Resultado genetic_algorithm_default(const Instance &inst, 
    const fs::path &caminho, 
    int population_size = 300, 
    int max_generations = 1000, 
    double crossover_rate = 0.9, 
    double mutation_rate = 0.35, 
    double maxGenEstagnated = 1, 
    double threshold = 0.01, 
    bool verbose = 1) {

    int n_items = inst.numItems;
    vector<vector<bool>> population(population_size, vector<bool>(n_items));
    vector<double> fitness(population_size, -1e9);
    Resultado melhorSol;
    melhorSol.valorObjetivo = -1e9;
    
    if(inst.numItems == 0){
        cout << "Arquivo com problema! ";
        return melhorSol;
    }

    // Inicializar população com soluções zeradas
    for (int i = 0; i < population_size; ++i) {
        for (int j = 0; j < n_items; ++j) {
            population[i][j] = 0;
        }
        fitness[i] = get_objective_value(population[i], inst);
    }
    // Abrir arquivo para escrita de resultado.
    std::ofstream log_file(caminho);
    if (!log_file.is_open()) {
        std::cerr << "Aviso: Nao foi possivel abrir o arquivo de log para escrita: " << caminho << std::endl;
    } else {
        log_file << "Iteracao;ValorObjetivo;Peso\n";
    }

    // 2. Loop principal do algoritmo genético
    int ultimaGeracaoDeMelhora = -1;
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
            } else { // Caso não haja, os pais são vão para a nova população
                new_population.push_back(parent1);
                if (new_population.size() < population_size) {
                    new_population.push_back(parent2);
                }
            }
        }
        // 2.3 Mutação
        for (auto &individual : new_population) {
            if ((double)rand() / RAND_MAX < mutation_rate) {
                int mutation_point = rand() % n_items;
                individual[mutation_point] = !individual[mutation_point];
            }
        }
        // 2.4 Avaliação da nova população
        double generation_best_fitness = -1e9;
        vector<bool> generation_best_sol;
        int pesoTotal;
        for (int i = 0; i < population_size; ++i) {
            fitness[i] = get_objective_value(new_population[i], inst);
            if (fitness[i] > generation_best_fitness) {
                generation_best_fitness = fitness[i];
                generation_best_sol = population[i];
            }
        }

        pesoTotal = calcularPeso(inst, generation_best_sol);

        if (log_file.is_open()) {
            log_file << generation + 1 << ";" << generation_best_fitness << ";" << pesoTotal << "\n";
        }
        // Armazenar melhor de todas as gerações
        if (generation_best_fitness > melhorSol.valorObjetivo) {
            int ultimaSolOtima = melhorSol.valorObjetivo;
            melhorSol.valorObjetivo = generation_best_fitness;
            melhorSol.itensSelecionados = generation_best_sol;
            melhorSol.pesoTotal = pesoTotal;
            double melhoraNaFuncaoObjetivo;

            if (melhorSol.valorObjetivo != 0) {
                melhoraNaFuncaoObjetivo = (melhorSol.valorObjetivo - ultimaSolOtima) / melhorSol.valorObjetivo;
            } else {
                // Caso a melhor solução
                melhoraNaFuncaoObjetivo = threshold * 1.1;
            }

            if (melhoraNaFuncaoObjetivo > threshold) {
                ultimaGeracaoDeMelhora = generation;
            }
        }

        if (generation - ultimaGeracaoDeMelhora > maxGenEstagnated * max_generations) {
            melhorSol.penalidadeTotal = calcularPenalidade(inst, melhorSol.itensSelecionados);
            melhorSol.lucroTotal = melhorSol.valorObjetivo + melhorSol.penalidadeTotal;

            if (verbose) {
                cout << "GA interrompido em " << generation << " gerações.\n";
            }
            return melhorSol;
        }

        population = new_population;
    }

    if (verbose) {
        cout << "\n--- Algoritmo Genético Finalizado ---" << endl;
        cout << "Melhor valor objetivo encontrado: " << melhorSol.valorObjetivo << endl;
        cout << "Itens na solução final: ";
        cout << "\nNúmero de itens: " << inst.numItems << endl;
        cout << "Peso total: " << melhorSol.pesoTotal << "/" << inst.capacity << endl;
    }

    melhorSol.penalidadeTotal = calcularPenalidade(inst, melhorSol.itensSelecionados);
    melhorSol.lucroTotal = melhorSol.valorObjetivo + melhorSol.penalidadeTotal;

    return melhorSol;
}



Resultado genetic_algorithm(const Instance &inst, 
    const fs::path &caminho, 
    int population_size = 500, 
    int max_generations = 5000, // Numero máximo de gerações. Também impacta o critério de parada por estagnação
    double crossover_rate = 0.9, // Taxa de Crossover entre pais. Caso não haja crossover, um ou dois pais vão para próxima geração.
    double mutation_rate = 0.15, // Taxa de mutação aleatória.
    double maxGenEstagnated = 0.2, // Valor % do máximo de gerações estagnadas para parar o algorítmo.
    double threshold = 0.001, // Valor de diferença % para se considerar estagnação
    bool verbose = 0) {
        
        int mode = 0;

        if(mode == 0){
            return genetic_algorithm_parallel(inst,caminho,population_size,max_generations,
                crossover_rate,mutation_rate,maxGenEstagnated,threshold,verbose);
        }else if(mode == 1){

            return genetic_algorithm_default(inst,caminho,population_size,max_generations,
                crossover_rate,mutation_rate,maxGenEstagnated,threshold,verbose);
        }
        cout << "Modo inválido" <<endl;
        Resultado res;
        return res;
    }