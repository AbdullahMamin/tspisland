// Some TSP solvers for testing and evaluation purposes
#ifndef SOLVERS_H
#define SOLVERS_H
#include "tsp.h"
#include "util.h"

#define MAX_CITIES_FOR_ENTROPY (0x2 << 16)

// Just go from 1 -> 2 -> ... -> n
u32 *SolveBasic(TSPInstance tsp_instance);

// Greedy, nearest neighbour solver TODO: optimize
u32 *SolveGreedy(TSPInstance tsp_instance);

// Our homebrew GA method
typedef struct {
    // Optional metric logging (provide NULL if unneeded)
    const char *log_path;

    // TSP problem instance
    TSPInstance problem;

    // GA parameters
    u32 population_size;
    u32 max_generations;
    f64 mutation_rate;

    // Optional seeds
    u32 *seed_tours;
    u32 n_seeds;
    f64 seed_percentage;

    // GA data
    TourBuffer population;
    f64 *population_fitness;
    u32 *r;
    Table child_city_table;
    Counter edge_counter;
} GASolver;
// u32 *SolveGA(TSPInstance tsp_instance, GAParameters parameters, const char *metrics_filepath);

// Initializes data of GA solver
bool GASolverInit(GASolver *solver);

// Frees GA data
void GASolverFree(GASolver *solver);

// TODO: possibly keep track of best tours over generations?
// Runs the GA algorithm and returns best found tour
u32 *GASolverSolve(GASolver *solver);

// TODO: Island based method

#endif // SOLVERS_H
