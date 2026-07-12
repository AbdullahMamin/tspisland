#include "ga.h"

// TODO: No need for GAIslandUpdatePopulationFitness() if we fix things up

// Internal functions that don't need to be exposed
static void GAIslandDoLogHeaders(GAIsland *island);
static void GAIslandDoLogs(GAIsland *island);
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

    if ((parameters.edge_profile_file || parameters.edge_entropy_file) && problem->n_cities <= MAX_CITIES_FOR_EDGE_STATISTICS) {
        size n_edges = problem->n_cities*(problem->n_cities - 1)/2;
        island.edge_counter = ArrayInit(n_edges);
        if (!ArrayOkay(&island.edge_counter)) {
            island.parameters.edge_profile_file = NULL;
            island.parameters.edge_entropy_file = NULL;
        }
    } else {
        island.parameters.edge_profile_file = NULL;
        island.parameters.edge_entropy_file = NULL;
    }

    for (u32 i = 0; i < actual_population_size; i++) {
        Tour individual = TourArrayAt(
            &island.population,
            island.problem->n_cities,
            i
        );
        TourRandomize(&individual);
    }
    assert(GAIslandPopulationIsValid(&island));

    GAIslandDoLogHeaders(&island);
    GAIslandDoLogs(&island);

    return island;
}

void GAIslandFree(GAIsland *island) {
    assert(GAIslandOkay(island));
    if (island->parameters.edge_profile_file) {
        ArrayFree(&island->edge_counter);
    }
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
        assert(GAIslandPopulationIsValid(island));
        GAIslandUpdatePopulationFitness(island);
        ArrayShuffle(
            &island->shuffle_indices,
            0,
            island->parameters.population_size - 1
        );

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
                TourCopy(&parent1, &child);
                island->population_fitness[*ArrayAt(&island->shuffle_indices, i)] = child_fitness;
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
                MutateTour(&individual, RandomFloat(0.0, island->parameters.max_mutation_strength));
                island->population_fitness[i] = -1.0;
            }
        }

        GAIslandDoLogs(island);
    }
}

static void GAIslandDoLogHeaders(GAIsland *island) {
    if (island->parameters.fitness_summary_file) {
        fprintf(
            island->parameters.fitness_summary_file,
            "min,max,avg\n"
        );
    }

    if (island->parameters.edge_profile_file) {
        // Nothing to do here
    }

    if (island->parameters.edge_entropy_file) {
        fprintf(
            island->parameters.edge_entropy_file,
            "entropy\n"
        );
    }
}

static void GAIslandDoLogs(GAIsland *island) {
    GAIslandUpdatePopulationFitness(island);

    if (island->parameters.fitness_summary_file) {
        f64 avg = island->population_fitness[0];
        f64 min = avg;
        f64 max = avg;
        for (u32 i = 1; i < island->parameters.population_size; i++) {
            f64 fit = island->population_fitness[i];
            avg += fit;
            if (fit < min) {
                min = fit;
            }
            if (fit > max) {
                max = fit;
            }
        }
        avg /= island->parameters.population_size;
        fprintf(
            island->parameters.fitness_summary_file,
            "%f,%f,%f\n", min, max, avg
        );
    }

    // Weird index calculations come from triangular numbers so I don't have to create a 2D array
    if (island->parameters.edge_profile_file || island->parameters.edge_entropy_file) {
        // Clear edge counter
        u32 n_edges = island->problem->n_cities*(island->problem->n_cities - 1)/2;
        for (u32 i = 0; i < n_edges; i++) {
            *ArrayAt(&island->edge_counter, i) = 0;
        }

        // Count edges
        for (u32 i = 0; i < island->parameters.population_size; i++) {
            Tour tour = TourArrayAt(&island->population, island->problem->n_cities, i);
            for (u32 j = 0; j < island->problem->n_cities; j++) {
                u32 from = *ArrayAt(&tour, j);
                u32 to = *ArrayAt(&tour, (j + 1)%island->problem->n_cities);
                if (from > to) {
                    u32 temp = from;
                    from = to;
                    to = temp;
                }
                u32 row = island->problem->n_cities - from - 2;
                u32 col = to - from - 1;
                u32 idx = row*(row + 1)/2 + col;
                *ArrayAt(&island->edge_counter, idx) += 1;
            }
        }
    }

    if (island->parameters.edge_profile_file) {
        // Log edges to file
        for (u32 from = 0; from < island->problem->n_cities - 1; from++) {
            for (u32 to = from + 1; to < island->problem->n_cities; to++) {
                u32 row = island->problem->n_cities - from - 2;
                u32 col = to - from - 1;
                u32 idx = row*(row + 1)/2 + col;
                u32 count = *ArrayAt(&island->edge_counter, idx);
                if (count == 0) {
                    continue;
                }
                fprintf(island->parameters.edge_profile_file, "%u,%u,%f\n", from, to, (f64)count/(f64)island->parameters.population_size);
            }
        }
        fprintf(island->parameters.edge_profile_file, "---\n");
    }

    if (island->parameters.edge_entropy_file) {
        f64 entropy = 0.0;
        u32 n_edges = island->problem->n_cities*(island->problem->n_cities - 1)/2;
        for (u32 i = 0; i < n_edges; i++) {
            u32 count = *ArrayAt(&island->edge_counter, i);
            if (count == 0) {
                continue;
            }
            f64 p = (f64)count/(f64)island->parameters.population_size;
            entropy -= p*log(p);
        }
        fprintf(island->parameters.edge_entropy_file, "%f\n", entropy);
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

static void MutateTour(Tour *tour, f64 strength) {
    assert(TourOkay(tour));
    // TODO: magic number
    if (strength <= 1.0e-5) {
        // Swap mutation
        u32 i = RandomInt(0, tour->capacity - 1);
        u32 j = RandomInt(0, tour->capacity - 1);
        u32 temp = *ArrayAt(tour, i);
        *ArrayAt(tour, i) = *ArrayAt(tour, j);
        *ArrayAt(tour, j) = temp;
        assert(TourIsValid(tour, NULL));
        return;
    }

    u32 length = strength*tour->capacity;
    u32 i = RandomInt(0, tour->capacity - 1);
    u32 j = (i + length)%tour->capacity;
    // 50/50 PSM or RSM
    if (CoinFlip(0.5)) {
        ArrayShuffle(tour, i, j);
    } else {
        ArrayReverse(tour, i, j);
    }
    assert(TourIsValid(tour, NULL));
}

static void CrossoverTours(Table *crossover_tracker, Tour *child, Tour *parent1, Tour *parent2) {
    // assert(
    //     TableOkay(crossover_tracker) &&
    //     TourOkay(child) &&
    //     TourIsValid(parent1, NULL) &&
    //     TourIsValid(parent2, NULL) &&
    //     child->capacity == parent1->capacity &&
    //     child->capacity == parent2->capacity
    // );
    assert(TableOkay(crossover_tracker));
    assert(TourOkay(child));
    assert(TourIsValid(parent1, NULL));
    assert(TourIsValid(parent2, NULL));
    assert(child->capacity == parent1->capacity);
    assert(child->capacity == parent2->capacity);

    u32 n_cities = child->capacity;

    u32 crossover_length = RandomInt(0, n_cities);
    if (crossover_length == 0) {
        TourCopy(child, parent2);
        return;
    } else if (crossover_length == n_cities) {
        TourCopy(child, parent1);
        return;
    }

    TableClear(crossover_tracker);

    u32 crossover_point = RandomInt(0, n_cities - 1);
    for (u32 i = 0; i < crossover_length; i++) {
        u32 *p1 = ArrayAt(parent1, (crossover_point + i)%n_cities);
        u32 *c = ArrayAt(child, i);
        *c = *p1;
        TableInsert(crossover_tracker, *p1);
    }

    u32 p2_i = 0;
    while (*ArrayAt(parent2, p2_i) != *ArrayAt(child, crossover_length - 1)) {
        p2_i++;
    }
    u32 from_p2 = n_cities - crossover_length;
    for (u32 i = 0; i < from_p2; i++) {
        u32 *p2 = ArrayAt(parent2, p2_i);
        while (TableHas(crossover_tracker, *p2)) {
            p2_i = (p2_i + 1)%n_cities;
            p2 = ArrayAt(parent2, p2_i);
        }
        u32 *c = ArrayAt(child, crossover_length + i);
        *c = *p2;
        p2_i = (p2_i + 1)%n_cities;
    }

    assert(TourIsValid(child, NULL));
}

Tour GAIslandBestIndividual(GAIsland *island) {
    assert(GAIslandOkay(island));
    GAIslandUpdatePopulationFitness(island);
    Tour best_individual = TourArrayAt(
        &island->population,
        island->problem->n_cities,
        0
    );
    f64 best_fitness = island->population_fitness[0];
    for (u32 i = 1; i < island->parameters.population_size; i++) {
        Tour individual = TourArrayAt(
            &island->population,
            island->problem->n_cities,
            i
        );
        f64 individual_fitness = island->population_fitness[i];
        if (individual_fitness > best_fitness) {
            best_individual = individual;
            best_fitness = individual_fitness;
        }
    }
    return best_individual;
}

bool GAIslandPopulationIsValid(GAIsland *island) {
    return TourArrayIsValid(&island->population, island->problem->n_cities);
}
