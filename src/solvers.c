#include "solvers.h"

u32 *SolveBasic(const TSPInstance *tsp_instance) {
    assert(TSPInstanceOkay(tsp_instance));
    u32 *tour = TourInit(tsp_instance->n_cities);
    if (!tour) {
        return NULL;
    }
    for (u32 i = 0; i < tsp_instance->n_cities; i++) {
        tour[i] = i;
    }
    return tour;
}

u32 *SolveGreedy(const TSPInstance *tsp_instance) {
    assert(TSPInstanceOkay(tsp_instance));
    u32 *tour = TourInit(tsp_instance->n_cities);
    if (!tour) {
        return NULL;
    }

    Table visited_table = TableInit(tsp_instance->n_cities);

    tour[0] = 0;
    TableInsert(&visited_table, 0);
    for (u32 i = 1; i < tsp_instance->n_cities; i++) {
        printf("Greedy at iteration %u...\n", i);
        u32 closest_neighbour = UINT32_MAX;
        f64 shortest_distance = INFINITY;
        for (u32 j = 0; j < tsp_instance->n_cities; j++) {
            if (TableHas(&visited_table, j)) {
                continue;
            }
            f64 distance = Vec2Distance(
                tsp_instance->city_positions[tour[i - 1]],
                tsp_instance->city_positions[j]
            );
            if (distance < shortest_distance) {
                closest_neighbour = j;
                shortest_distance = distance;
            }
        }
        assert(closest_neighbour != UINT32_MAX);
        tour[i] = closest_neighbour;
        TableInsert(&visited_table, closest_neighbour);
    }

    TableFree(&visited_table);

    return tour;
}

bool GASolverInit(GASolver *solver) {
    if (!TSPInstanceOkay(solver->problem)) {
        puts("Didn't provide valid TSP instance!");
        return false;
    }

    if (solver->population_size <= 1) {
        puts("Population must be at least 2!");
        return false;
    }

    if (solver->mutation_rate < 0.0 || solver->mutation_rate > 1.0) {
        puts("Mutation rate must be between 0.0 and 1.0!");
        return false;
    }

    if (solver->n_seeds > 0 && !solver->seed_tours) {
        puts("Seeds not provided!");
        return false;
    }

    if (solver->n_seeds > 0 && (solver->seed_percentage < 0.0 || solver->seed_percentage > 1.0)) {
        puts("Seed percentage needs to be between 0.0 and 1.0!");
        return false;
    }

    // + 1 for children that are created (TODO: use a whole different buffer for children)
    solver->population = TourArrayInit(solver->problem, solver->population_size + 1);
    if (!TourArrayOkay(&solver->population)) {
        puts("Couldn't allocate population!");
        return false;
    }

    // + 1 for children that are created (TODO: use a whole different buffer for children)
    solver->population_fitness = calloc(solver->population_size + 1, sizeof(*solver->population_fitness));
    if (!solver->population_fitness) {
        puts("Couldn't allocate population fitness buffer!");
        TourArrayFree(&solver->population);
        return false;
    }

    solver->r = calloc(solver->population_size, sizeof(*solver->r));
    if (!solver->r) {
        puts("Couldn't allocate population shuffle buffer!");
        free(solver->population_fitness);
        TourArrayFree(&solver->population);
        return false;
    }
    for (u32 i = 0; i < solver->population_size; i++) {
        solver->r[i] = i;
    }

    solver->child_city_table = TableInit(solver->problem->n_cities);
    if (!TableOkay(&solver->child_city_table)) {
        puts("Couldn't allocate child city table!");
        free(solver->r);
        free(solver->population_fitness);
        TourArrayFree(&solver->population);
        return false;
    }

    return true;
}

void GASolverFree(GASolver *solver) {
    TableFree(&solver->child_city_table);
    free(solver->r);
    free(solver->population_fitness);
    TourArrayFree(&solver->population);
}

// === Helper functions for GA ===

// Writes a fitness summary to a file
static void GAWriteGenerationSummary(GASolver *solver, Counter *edge_counter, FILE *log_file) {
    assert(solver->problem->n_cities <= MAX_CITIES_FOR_SUMMARY);
    // avg_fitness,stddev_fitness,worst_fitness,best_fitness,edge_entropy
    f64 worst_fitness = INFINITY;
    f64 best_fitness = -INFINITY;
    f64 avg_fitness = 0.0;
    for (u32 i = 0; i < solver->population_size; i++) {
        f64 fitness = solver->population_fitness[i];
        if (fitness < worst_fitness) {
            worst_fitness = fitness;
        }
        if (fitness > best_fitness) {
            best_fitness = fitness;
        }
        avg_fitness += fitness/solver->population_size;
    }

    f64 stddev_fitness = 0.0;
    for (u32 i = 0; i < solver->population_size; i++) {
        f64 difference = solver->population_fitness[i] - avg_fitness;
        stddev_fitness += difference*difference/solver->population_size;
    }
    stddev_fitness = sqrt(stddev_fitness);

    CounterClear(edge_counter);
    for (u32 i = 0; i < solver->population_size; i++) {
        for (u32 j = 0; j < solver->problem->n_cities; j++) {
            u32 from = TourArrayAt(&solver->population, i)[j];
            u32 to = TourArrayAt(&solver->population, i)[(j + 1)%solver->problem->n_cities];
            if (from > to) {
                u32 temp = from;
                from = to;
                to = temp;
            }
            u32 edge = (from << 16) | to;
            *CounterAt(edge_counter, edge) += 1;
        }
    }
    f64 total_entropy = 0.0;
    for (u32 i = 0; i < edge_counter->capacity; i++) {
        if (*CounterAt(edge_counter, i) == 0) {
            continue;
        }
        f64 p = (f64)*CounterAt(edge_counter, i)/(f64)solver->population_size;
        total_entropy -= p*log(p);
    }

    fprintf(log_file, "%f,%f,%f,%f,%f", avg_fitness, stddev_fitness, worst_fitness, best_fitness, total_entropy);

    // TODO: edge heat
    for (u32 i = 0; i < edge_counter->capacity; i++) {
        if (*CounterAt(edge_counter, i) == 0) {
            continue;
        }
        u32 from = (i&0xFFFF0000) >> 16;
        u32 to = i&0xFFFF;
        if (from > to) {
            u32 temp = from;
            from = to;
            to = temp;
        }
        fprintf(log_file, ",%u,%u,%f", from, to, (f64)*CounterAt(edge_counter, i)/(f64)solver->population_size);
    }
    fprintf(log_file, "\n");
}
// Calculate entire population fitness
static void GASolverCalculatePopulationFitness(GASolver *solver);
// Mutate an individual (TODO: study which to use)
static void GASolverMutate(GASolver *solver, u32 individual_idx);
// OX crossover (TODO: source)
static void GASolverCrossover(GASolver *solver, u32 p1_idx, u32 p2_idx, u32 c_idx);
// TODO: selection

// ===============================

u32 *GASolverSolve(GASolver *solver) {
    FILE *log_file = NULL;
    if (solver->log_path) {
        if (solver->problem->n_cities > MAX_CITIES_FOR_SUMMARY) {
            puts("Exceeded max problem size for summary logging!");
            return NULL;
        }
        log_file = fopen(solver->log_path, "w");
    }

    Counter edge_counter;
    if (log_file) {
        u32 max_edge = (solver->problem->n_cities - 1) | ((solver->problem->n_cities - 1) << 16);
        edge_counter = CounterInit(max_edge);
        if (!CounterOkay(&edge_counter)) {
            puts("Couldn't init edge counter for summary logging!");
            return NULL;
        }
    }

    // Initialize population randomly
    u32 seeded_population_size = floor(solver->seed_percentage*solver->population_size);
    u32 unseeded_population_size = solver->population_size - seeded_population_size;
    for (u32 i = 0; i < unseeded_population_size; i++) {
        TourRandomize(TourArrayAt(&solver->population, i), solver->problem->n_cities);
    }
    for (u32 i = 0; i < seeded_population_size; i++) {
        u32 seed_idx = i%solver->n_seeds;
        const u32 *seed_tour = &solver->seed_tours[solver->problem->n_cities*seed_idx];
        TourCopy(TourArrayAt(&solver->population, i + unseeded_population_size), seed_tour, solver->problem->n_cities);
    }
    GASolverCalculatePopulationFitness(solver);

    if (log_file) {
        GAWriteGenerationSummary(solver, &edge_counter, log_file);
    }

    // Evolutionary loop
    for (u32 generation = 1; generation <= solver->max_generations; generation++) {
        printf("GA at generation %u/%u........\r", generation, solver->max_generations);
        ShuffleArrayU32(solver->r, solver->population_size, 0, solver->population_size - 1);
        for (u32 i = 0; i < solver->population_size - 1; i++) {
            // Crossover
            u32 p1_idx = solver->r[i];
            u32 p2_idx = solver->r[i + 1];
            u32 c_idx = solver->population_size;
            GASolverCrossover(solver, p1_idx, p2_idx, c_idx);

            // TODO: better selection
            f64 p1_fitness = solver->population_fitness[p1_idx];
            f64 child_fitness = TourEvaluate(solver->problem, TourArrayAt(&solver->population, solver->population_size));
            f64 replacement_probability = child_fitness/(p1_fitness + child_fitness);
            if (child_fitness > p1_fitness && CoinFlip(replacement_probability)) {
                TourCopy(TourArrayAt(&solver->population, p1_idx), TourArrayAt(&solver->population, c_idx), solver->problem->n_cities);
            }
        }

        // Mutation
        for (u32 i = 0; i < solver->population_size; i++) {
            if (CoinFlip(solver->mutation_rate)) {
                GASolverMutate(solver, i);
            }
        }

        GASolverCalculatePopulationFitness(solver);

        if (log_file) {
            GAWriteGenerationSummary(solver, &edge_counter, log_file);
        }
    }
    printf("\n");

    if (log_file) {
        CounterFree(&edge_counter);
        fclose(log_file);
    }

    u32 *best_tour = NULL;
    f64 best_fitness = -INFINITY;
    for (u32 i = 0; i < solver->population_size; i++) {
        u32 *individual = TourArrayAt(&solver->population, i);
        f64 fitness = solver->population_fitness[i];
        if (fitness > best_fitness) {
            best_tour = individual;
            best_fitness = fitness;
        }
    }
    return best_tour;
}

static void GASolverCalculatePopulationFitness(GASolver *solver) {
    for (u32 i = 0; i < solver->population_size; i++) {
        solver->population_fitness[i] = TourEvaluate(solver->problem, TourArrayAt(&solver->population, i));
    }
}

// TODO: add mutation strength parameters
// TODO: mutation point selection could be biased
// TODO: since genes are circular, allow for shuffles that cross boundary
static void GASolverMutate(GASolver *solver, u32 individual_idx) {
    u32 *individual = TourArrayAt(&solver->population, individual_idx);

    if (CoinFlip(0.05)) {
        u32 mutation_start = RandomI32(0, solver->problem->n_cities - 1);
        u32 mutation_end = RandomI32(0, solver->problem->n_cities - 1);
        if (CoinFlip(0.5)) {
            ShuffleArrayU32(individual, solver->problem->n_cities, mutation_start, mutation_end);
        } else {
            ReverseArrayU32(individual, solver->problem->n_cities, mutation_start, mutation_end);
        }
    } else {
        u32 i = RandomI32(0, solver->problem->n_cities - 1);
        u32 j = RandomI32(0, solver->problem->n_cities - 1);
        u32 temp = individual[i];
        individual[i] = individual[j];
        individual[j] = temp;
    }
}

static void GASolverCrossover(GASolver *solver, u32 p1_idx, u32 p2_idx, u32 c_idx) {
    TableClear(&solver->child_city_table);

    u32 *parent1 = TourArrayAt(&solver->population, p1_idx);
    u32 *parent2 = TourArrayAt(&solver->population, p2_idx);
    u32 *child = TourArrayAt(&solver->population, c_idx);

    // Copy from parent1
    i32 crossover_idx = RandomI32(0, solver->problem->n_cities - 1);
    u32 crossover_len = RandomI32(0, solver->problem->n_cities);
    for (u32 j = 0; j < crossover_len; j++) {
        u32 city = parent1[(crossover_idx + j)%solver->problem->n_cities];
        child[j] = city;
        TableInsert(&solver->child_city_table, city);
    }

    // Copy from parent2
    u32 cities_to_add = solver->problem->n_cities - crossover_len;
    u32 parent2_idx = 0;
    while (crossover_len > 0 && parent2[parent2_idx] != child[crossover_len - 1]) {
        parent2_idx++;
    }
    u32 child_idx = crossover_len;
    while (cities_to_add > 0) {
        u32 city = parent2[parent2_idx];
        if (!TableHas(&solver->child_city_table, city)) {
            child[child_idx++] = city;
            cities_to_add--;
        }
        parent2_idx = (parent2_idx + 1)%solver->problem->n_cities;
    }
}
