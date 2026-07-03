// Some TSP solvers for testing and evaluation purposes
#ifndef SOLVERS_H
#define SOLVERS_H
#include "tsp.h"
#include "util.h"
#include "worker.h"

#define MAX_CITIES_FOR_EDGE_STATISTICS (10000)

#define MAX_ISLANDS (100)

// Just go from 1 -> 2 -> ... -> n
u32 *SolveBasic(const TSPInstance *tsp_instance);

// Greedy, nearest neighbour solver TODO: optimize
u32 *SolveGreedy(const TSPInstance *tsp_instance, u32 starting_city);

// Our homebrew GA method
typedef struct {
    // Fitness summary file path (NULL if unneeded)
    const char *summary_out;

    // Edge entropy file path (NULL if unneeded)
    const char *edge_entropy_out;

    // Edge heatmap file path (NULL if unneeded)
    const char *edge_heat_out;

    // Seed tour file path (NULL if unneeded)
    const char *seed_in;

    // TSP problem instance to solve
    const TSPInstance *problem;

    // GA parameters
    u32 population_size;
    u32 max_generations;
    f64 mutation_rate;
    f64 max_mutation_strength;
    f64 seed_percentage;
} GAParameters;

// TODO: possibly keep track of best tours over generations?
u32 *SolveGA(GAParameters parameters);

// The island migration method
typedef struct {
    u32 epoch_length; // how many generations to run before migration
    f64 migration_rate; // what percent of the population to migrate

    // TODO: maybe more than just two islands?
    // MPI ranks of connected islands
    int dst_rank;
    int src_rank;
} IslandParameters;

u32 *SolveIsland(GAParameters ga_parameters, IslandParameters island_parameters);

#endif // SOLVERS_H
