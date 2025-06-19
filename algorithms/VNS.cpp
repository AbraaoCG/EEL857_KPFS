#include <vector>
#include <numeric>
#include <algorithm> // For std::max and std::min
#include <cfloat>    // For DBL_MIN
#include <iostream>
#include <fstream>
#include <filesystem>
#include <omp.h>     // For OpenMP
#include <random>    // For C++11 random number generation

#include "../utils/structs.hpp"
#include "../utils/functions.hpp" 

namespace fs = std::filesystem;
using namespace std;

// --- Thread-local Random Number Generator (for OpenMP) ---
// Each thread gets its own generator instance to ensure independent sequences.
thread_local std::mt19937_64 generator;

// Function to initialize RNG for each thread (call once per thread, e.g., at thread start)
void init_thread_rng() {
    // A simple way to get unique seeds per thread: based on current time + thread ID
    std::random_device rd;
    generator.seed(rd() + omp_get_thread_num());
}

// Function to get a random integer in a range (inclusive)
int get_random_int(int min_val, int max_val) {
    std::uniform_int_distribution<int> distribution(min_val, max_val);
    return distribution(generator);
}

// --- Helper Functions ---

// Shaking - Generates a new solution from the current one by applying 'k' random bit inversions
vector<bool> shake_solution(const vector<bool>& sol, int n_items, int k) {
    vector<bool> shaken_sol = sol;
    for (int i = 0; i < k; ++i) {
        int random_bit_index = get_random_int(0, n_items - 1); // Using thread-safe RNG
        shaken_sol[random_bit_index] = !shaken_sol[random_bit_index];
    }
    return shaken_sol;
}

// --- Local Search (Best Improvement 1-Opt Local Search) ---
// Evaluates all 1-Opt neighbors and returns the best one found.
vector<bool> local_search(const vector<bool>& current_sol, const Instance& inst) {
    vector<bool> best_local_sol = current_sol;
    double best_local_obj_value = get_objective_value(current_sol, inst);
    int n_items = current_sol.size();

    // Variables for each thread to find its local best
    double thread_best_obj_value;
    vector<bool> thread_best_sol;

    // Parallelize the loop that evaluates neighbors
    #pragma omp parallel private(thread_best_obj_value, thread_best_sol)
    {
        init_thread_rng(); // Initialize RNG for this thread within the parallel region

        // Initialize with a sufficiently small negative value for maximization
        thread_best_obj_value = -1e+9;
        thread_best_sol = current_sol; // Each thread starts with the current solution as its local best

        #pragma omp for nowait // Distribute loop iterations; threads don't wait at the end of the loop
        for (int i = 0; i < n_items; ++i) {
            vector<bool> neighbor_sol = current_sol; // Start with the current solution for each neighbor
            neighbor_sol[i] = !neighbor_sol[i];      // Invert one bit to get a neighbor

            double neighbor_obj_value = get_objective_value(neighbor_sol, inst);

            // Each thread finds the best neighbor within its assigned partition
            if (neighbor_obj_value > thread_best_obj_value) { // Assuming maximization
                thread_best_obj_value = neighbor_obj_value;
                thread_best_sol = neighbor_sol;
            }
        }

        // Combine the best results from all threads into the global best for this local search
        #pragma omp critical // Only one thread can execute this block at a time
        {
            if (thread_best_obj_value > best_local_obj_value) { // Assuming maximization
                best_local_obj_value = thread_best_obj_value;
                best_local_sol = thread_best_sol;
            }
        }
    }
    return best_local_sol;
}


Resultado vns(const Instance &inst, const fs::path &caminho,
              int max_generations = 3000,
              double maxGenEstagnated_ratio = 0.3,
              double threshold = 0.01,
              int k_max = 10, bool verbose = 0) {
    int n_items = inst.numItems;
    Resultado melhorSol;
    // For maximization, initialize with a sufficiently small negative value.
    melhorSol.valorObjetivo = -1e+9;

    // Start with a solution where no items are selected.
    vector<bool> current_sol(n_items, false);

    double current_obj_value = get_objective_value(current_sol, inst);

    // Initialize the global best solution
    if (current_obj_value > melhorSol.valorObjetivo) {
        melhorSol.valorObjetivo = current_obj_value;
        melhorSol.itensSelecionados = current_sol;
        melhorSol.pesoTotal = calcularPeso(inst, current_sol);
    }

    std::ofstream log_file(caminho);
    if (!log_file.is_open()) {
        std::cerr << "Warning: Could not open log file for writing: " << caminho << std::endl;
    } else {
        log_file << "Iteration;ObjectiveValue;Weight\n";
    }

    if (verbose) {
        cout << "VNS Started. Initial Solution Objective: " << current_obj_value << endl;
    }

    int last_improvement_generation = -1; // Tracks the generation of the last significant improvement
    int max_stagnated_iterations = static_cast<int>(maxGenEstagnated_ratio * max_generations);

    // Seed the main thread's RNG (for shake_solution, which is called sequentially)
    std::random_device rd;
    generator.seed(rd());

    for (int generation = 0; generation < max_generations; ++generation) {
        int k = 1; // Start with the closest neighborhood
        while (k <= k_max) {
            // 1. Shaking: Generate a random neighbor in the k-th neighborhood
            vector<bool> shaken_sol = shake_solution(current_sol, n_items, k);

            // 2. Local Search: Apply local search on the shaken solution
            // This now uses the Best Improvement logic (no RCL)
            vector<bool> improved_sol = local_search(shaken_sol, inst);
            double improved_obj_value = get_objective_value(improved_sol, inst);

            // 3. Move or Not: Update the current and global best solution
            if (improved_obj_value > current_obj_value) {
                current_sol = improved_sol;
                current_obj_value = improved_obj_value;
                k = 1; // Reset to the closest neighborhood after an improvement

                // Update the global best solution if necessary
                if (current_obj_value > melhorSol.valorObjetivo) {
                    double old_best_objective = melhorSol.valorObjetivo;
                    melhorSol.valorObjetivo = current_obj_value;
                    melhorSol.itensSelecionados = current_sol;
                    melhorSol.pesoTotal = calcularPeso(inst, current_sol);

                    double improvement_ratio;
                    // Avoid division by zero or very small numbers when calculating relative improvement
                    if (abs(melhorSol.valorObjetivo) > 1e-9) {
                        improvement_ratio = (melhorSol.valorObjetivo - old_best_objective) / melhorSol.valorObjetivo;
                    } else {
                        // If objective is zero or very small, any positive increase can be significant
                        improvement_ratio = (melhorSol.valorObjetivo > old_best_objective) ? (threshold * 1.1) : 0.0;
                    }

                    if (improvement_ratio > threshold) {
                        last_improvement_generation = generation; // Reset stagnation counter
                    }
                }
            } else {
                k++; // Move to the next neighborhood if no improvement
            }
        }

        // Log the best solution of the iteration
        if (log_file.is_open()) {
            log_file << generation + 1 << ";" << melhorSol.valorObjetivo << ";" << melhorSol.pesoTotal << "\n";
        }

        if (verbose && (generation % 100 == 0 || generation == max_generations - 1)) {
            cout << "Iteration " << generation + 1 << ": Best Objective Value = " << melhorSol.valorObjetivo << endl;
        }

        // Stagnation stopping criterion
        if (generation - last_improvement_generation > max_stagnated_iterations) {
            if (verbose) {
                cout << "VNS terminated due to stagnation at " << generation << " generations.\n";
            }
            break; // Exit the main loop
        }
    }

    if (verbose) {
        cout << "\n--- VNS Algorithm Finished ---" << endl;
        cout << "Best objective value found: " << melhorSol.valorObjetivo << endl;
        cout << "Total weight: " << melhorSol.pesoTotal << "/" << inst.capacity << endl;
    }

    melhorSol.penalidadeTotal = calcularPenalidade(inst, melhorSol.itensSelecionados);
    melhorSol.lucroTotal = melhorSol.valorObjetivo + melhorSol.penalidadeTotal;

    log_file.close();
    return melhorSol;
}