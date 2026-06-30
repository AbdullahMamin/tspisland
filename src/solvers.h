// Some TSP solvers for testing and evaluation purposes
#ifndef SOLVERS_H
#define SOLVERS_H
#include "tsp.h"
#include "util.h"

#define MAX_CITIES_FOR_SUMMARY (0x2 << 16)

// Just go from 1 -> 2 -> ... -> n
u32 *SolveBasic(const TSPInstance *tsp_instance);

// Greedy, nearest neighbour solver TODO: optimize
u32 *SolveGreedy(const TSPInstance *tsp_instance);

// Our homebrew GA method
typedef struct {
    // Optional metric logging (provide NULL if unneeded)
    const char *log_path;

    // TSP problem instance
    const TSPInstance *problem;

    // GA parameters
    u32 population_size;
    u32 max_generations;
    f64 mutation_rate;

    // Optional seeds
    const u32 *seed_tours;
    u32 n_seeds;
    f64 seed_percentage;

    // GA data
    TourArray population;
    f64 *population_fitness;
    u32 *r;
    Table child_city_table;
} GASolver;

// Initializes data of GA solver and returns if it succeeded or not
bool GASolverInit(GASolver *solver);

// Frees GA data
void GASolverFree(GASolver *solver);

// TODO: possibly keep track of best tours over generations?
// Runs the GA algorithm and returns best found tour
u32 *GASolverSolve(GASolver *solver);

// TODO: Island based method

#endif // SOLVERS_H
