#include "ga.h"

// Internal functions that don't need to be exposed
static void GAIslandUpdatePopulationFitness(GAIsland *island);
static void MutateTour(Tour *tour, f64 strength);
static void CrossoverTours(Table *crossover_tracker, Tour *child, Tour *parent1, Tour *parent2);

GAIsland GAIslandInit(const TSPInstance *problem, GAParameters parameters) {
    GAIsland island = (GAIsland) {
        .problem = problem,
        .parameters = parameters
    };

    // Check that parameters are okay
    if (
        parameters.population_size < 2 ||
        parameters.mutation_rate < 0.0 ||
        parameters.mutation_rate > 1.0 ||
        parameters.max_mutation_strength < 0.0 ||
        parameters.max_mutation_strength > 1.0
    ) {
        return island;
    }

    island.shuffle_indices = ArrayInit(parameters.population_size);
    if (!ArrayOkay(&island.shuffle_indices)) {
        return island;
    }
    for (u32 i = 0; i < parameters.population_size; i++) {
        *ArrayAt(&island.shuffle_indices, i) = i;
    }

    island.crossover_tracker = TableInit(problem->n_cities);
    if (!TableOkay(&island.crossover_tracker)) {
        ArrayFree(&island.shuffle_indices);
        return island;
    }

    // + 1 for the child during crossover
    size actual_population_size = parameters.population_size + 1;

    island.population = TourArrayInit(problem->n_cities, actual_population_size);
    if (!TourArrayOkay(&island.population)) {
        TableFree(&island.crossover_tracker);
        ArrayFree(&island.shuffle_indices);
        return island;
    }

    island.population_fitness = calloc(actual_population_size, sizeof(f64));
    if (!island.population_fitness) {
        TourArrayFree(&island.population);
        TableFree(&island.crossover_tracker);
        ArrayFree(&island.shuffle_indices);
        return island;
    }
    for (u32 i = 0; i < actual_population_size; i++) {
        island.population_fitness[i] = -1.0;
    }

    // TODO: file headers

    return island;
}

void GAIslandFree(GAIsland *island) {
    assert(GAIslandOkay(island));
    free(island->population_fitness);
    island->population_fitness = NULL;
    TourArrayFree(&island->population);
    TableFree(&island->crossover_tracker);
    ArrayFree(&island->shuffle_indices);
}

bool GAIslandOkay(const GAIsland *island) {
    return TourArrayOkay(&island->population) && island->population_fitness;
}

void GAIslandSeed(GAIsland *island, const TourArray *seeds, size n_seeds) {
    assert(GAIslandOkay(island) && TourArrayOkay(seeds));
    TourArrayCopy(&island->population, seeds, island->problem->n_cities, n_seeds);
}

void GAIslandEvolve(GAIsland *island, u32 n_generations) {
    assert(GAIslandOkay(island));
    for (u32 generation = 0; generation < n_generations; generation++) {
        GAIslandUpdatePopulationFitness(island);
        ArrayShuffle(
            &island->shuffle_indices,
            0,
            island->parameters.population_size - 1
        );

        // TODO: write stuff to files

        // Crossover
        // TODO: Better selection
        for (u32 i = 0; i < island->parameters.population_size; i++) {
            Tour child = TourArrayAt(
                &island->population,
                island->problem->n_cities,
                island->parameters.population_size
            );
            Tour parent1 = TourArrayAt(
                &island->population,
                island->problem->n_cities,
                *ArrayAt(&island->shuffle_indices, i)
            );
            Tour parent2 = TourArrayAt(
                &island->population,
                island->problem->n_cities,
                *ArrayAt(
                    &island->shuffle_indices,
                    (i + 1)%island->parameters.population_size
                )
            );
            CrossoverTours(
                &island->crossover_tracker,
                &child,
                &parent1,
                &parent2
            );
            f64 child_fitness = TourEvaluate(island->problem, &child);
            f64 parent1_fitness = island->population_fitness[*ArrayAt(&island->shuffle_indices, i)];
            if (child_fitness >= parent1_fitness) {
                TourCopy(&parent1, &child, island->problem->n_cities);
            }
        }

        // Mutation
        for (u32 i = 0; i < island->parameters.population_size; i++) {
            Tour individual = TourArrayAt(
                &island->population,
                island->problem->n_cities,
                i
            );
            if (CoinFlip(island->parameters.mutation_rate)) {
                MutateTour(&individual, 0.0);
                island->population_fitness[i] = -1.0;
            }
        }
    }
}

static void GAIslandUpdatePopulationFitness(GAIsland *island) {
    for (u32 i = 0; i < island->parameters.population_size; i++) {
        if (island->population_fitness[i] >= 0.0) {
            continue;
        }
        Tour tour = TourArrayAt(
            &island->population,
            island->problem->n_cities,
            i
        );
        island->population_fitness[i] = TourEvaluate(
            island->problem,
            &tour
        );
    }
}

// TODO: RSM/PSM mutation
static void MutateTour(Tour *tour, f64 strength) {
    (void)strength;
    assert(TourOkay(tour));
    u32 i = RandomInt(0, tour->capacity - 1);
    u32 j = RandomInt(0, tour->capacity - 1);
    u32 temp = *ArrayAt(tour, i);
    *ArrayAt(tour, i) = *ArrayAt(tour, j);
    *ArrayAt(tour, j) = temp;
}

static void CrossoverTours(Table *crossover_tracker, Tour *child, Tour *parent1, Tour *parent2) {
    assert(
        TableOkay(crossover_tracker) &&
        TourIsValid(child, NULL) &&
        TourIsValid(parent1, NULL) &&
        TourIsValid(parent2, NULL)
    );

    u32 n_cities = child->capacity;

    u32 crossover_length = RandomInt(0, n_cities);
    if (crossover_length == 0) {
        TourCopy(child, parent2, n_cities);
        return;
    } else if (crossover_length == n_cities) {
        TourCopy(child, parent1, n_cities);
        return;
    }

    TableClear(crossover_tracker);

    u32 crossover_point = RandomInt(0, n_cities - 1);
    for (u32 i = 0; i < crossover_length; i++) {
        u32 *c = ArrayAt(child, i);
        u32 *p1 = ArrayAt(parent1, (crossover_point + i)%n_cities);
        *c = *p1;
        TableInsert(crossover_tracker, *p1);
    }

    // A bit hacky, but it's okay :)
    u32 *p2 = ArrayAt(parent2, 0);
    u32 from_p2 = n_cities - crossover_length;
    for (u32 i = 0; i < from_p2; i++) {
        while (TableHas(crossover_tracker, *p2)) {
            p2++;
        }
        u32 *c = ArrayAt(child, i);
        *c = *p2;
    }
}
