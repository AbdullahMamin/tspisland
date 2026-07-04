// ga.h: single island genetic algorithm
#ifndef GA_H
#define GA_H
#include "tsp.h"

typedef struct {
    // optional metric paths (NULL if unneeded)
    FILE *fitness_summary_file;
    FILE *edge_profile_file;

    // parameters
    u32 population_size;
    f64 mutation_rate;
    f64 max_mutation_strength;
} GAParameters;

// Single island GA
typedef struct {
    const TSPInstance *problem;
    GAParameters parameters;
    Array shuffle_indices;
    Table crossover_tracker;
    TourArray population;
    f64 *population_fitness;
} GAIsland;

// Initialize single GA island
GAIsland GAIslandInit(const TSPInstance *problem, GAParameters parameters);

// Free GA island data
void GAIslandFree(GAIsland *island);

// Returns if GA initialization succeeded
bool GAIslandOkay(const GAIsland *island);

// Seed island population
void GAIslandSeed(GAIsland *island, const TourArray *seeds, size n_seeds);

// Evolve island for some generations
void GAIslandEvolve(GAIsland *island, u32 n_generations);

#endif // GA_H
