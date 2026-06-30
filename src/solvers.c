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

u32 *SolveGreedy(const TSPInstance *tsp_instance, u32 starting_city) {
    assert(TSPInstanceOkay(tsp_instance) && starting_city < tsp_instance->n_cities);
    u32 *tour = TourInit(tsp_instance->n_cities);
    if (!tour) {
        return NULL;
    }

    Table visited_table = TableInit(tsp_instance->n_cities);

    tour[0] = starting_city;
    TableInsert(&visited_table, tour[0]);
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

// ==== GA SOLVER =====
typedef struct {
    GAParameters parameters;

    // GA data
    TourArray population;
    f64 *population_fitness;
    u32 *r; // for roulette selection
    Table child_city_table; // for crossover
} GASolver;

static GASolver GASolverInit(GAParameters parameters);
static void GASolverFree(GASolver *solver);
static bool GASolverOkay(const GASolver *solver);
static void GASolverEvolve(GASolver *solver, u32 n_generations);
static u32 *GASolverBestIndividual(GASolver *solver);
static void GASolverRandomizeIndividual(GASolver *solver, u32 index);
static void GASolverCalculatePopulationFitness(GASolver *solver);
static void GASolverMutateIndividual(GASolver *solver, u32 index, f64 strength);
static void GASolverCrossoverIndividuals(GASolver *solver, u32 p1_index, u32 p2_index, u32 c_index);

u32 *SolveGA(GAParameters parameters) {
    GASolver solver = GASolverInit(parameters);
    if (!GASolverOkay(&solver)) {
        return NULL;
    }
    u32 *best_tour = TourInit(parameters.problem->n_cities);
    if (!best_tour) {
        puts("Couldn't allocate tour buffer!");
        GASolverFree(&solver);
        return NULL;
    }

    GASolverEvolve(&solver, parameters.max_generations);
    TourCopy(best_tour, GASolverBestIndividual(&solver), parameters.problem->n_cities);

    GASolverFree(&solver);
    return best_tour;
}

static GASolver GASolverInit(GAParameters parameters) {
    GASolver solver = (GASolver){
        .parameters = parameters,
        .population_fitness = NULL,
        .r = NULL
    };

    if (!TSPInstanceOkay(parameters.problem)) {
        puts("Didn't provide valid TSP instance!");
        return solver;
    }

    if (
        (parameters.edge_entropy_out || parameters.edge_heat_out) &&
        parameters.problem->n_cities > MAX_CITIES_FOR_EDGE_STATISTICS
    ) {
        printf(
            "Can't log edge statistics when problem size is greater than %u!\n",
            MAX_CITIES_FOR_EDGE_STATISTICS
        );
        return solver;
    }

    if (parameters.population_size <= 1) {
        puts("Population must be at least 2!");
        return solver;
    }

    if (parameters.mutation_rate < 0.0 || parameters.mutation_rate > 1.0) {
        puts("Mutation rate must be between 0.0 and 1.0!");
        return solver;
    }

    if (parameters.max_mutation_strength < 0.0 || parameters.max_mutation_strength > 1.0) {
        puts("Max mutation strength must be between 0.0 and 1.0!");
        return solver;
    }

    if (parameters.seeds && !TourArrayOkay(parameters.seeds)) {
        puts("Seeds not provided!");
        return solver;
    }

    if (parameters.seeds && (parameters.seed_percentage < 0.0 || parameters.seed_percentage > 1.0)) {
        puts("Seed percentage needs to be between 0.0 and 1.0!");
        return solver;
    }

    // + 1 for children that are created (TODO: use a whole different buffer for children)
    solver.population = TourArrayInit(parameters.problem, parameters.population_size + 1);
    if (!TourArrayOkay(&solver.population)) {
        puts("Couldn't allocate population!");
        return solver;
    }

    // + 1 for children that are created (TODO: use a whole different buffer for children)
    solver.population_fitness = calloc(parameters.population_size + 1, sizeof(*solver.population_fitness));
    if (!solver.population_fitness) {
        puts("Couldn't allocate population fitness buffer!");
        TourArrayFree(&solver.population);
        return solver;
    }

    solver.r = calloc(parameters.population_size, sizeof(*solver.r));
    if (!solver.r) {
        puts("Couldn't allocate population shuffle buffer!");
        free(solver.population_fitness);
        solver.population_fitness = NULL;
        TourArrayFree(&solver.population);
        return solver;
    }
    for (u32 i = 0; i < parameters.population_size; i++) {
        solver.r[i] = i;
    }

    solver.child_city_table = TableInit(parameters.problem->n_cities);
    if (!TableOkay(&solver.child_city_table)) {
        puts("Couldn't allocate child city table!");
        free(solver.r);
        solver.r = NULL;
        free(solver.population_fitness);
        solver.population_fitness = NULL;
        TourArrayFree(&solver.population);
        return solver;
    }

    // TODO: add seeds
    for (u32 i = 0; i < parameters.population_size; i++) {
        GASolverRandomizeIndividual(&solver, i);
    }
    GASolverCalculatePopulationFitness(&solver);

    return solver;
}

static void GASolverFree(GASolver *solver) {
    assert(GASolverOkay(solver));
    TableFree(&solver->child_city_table);
    free(solver->r);
    solver->r = NULL;
    free(solver->population_fitness);
    solver->population_fitness = NULL;
    TourArrayFree(&solver->population);
}

static bool GASolverOkay(const GASolver *solver) {
    return solver->r != NULL;
}

static void GASolverEvolve(GASolver *solver, u32 n_generations) {
    for (u32 i = 0; i < n_generations; i++) {
        // TODO: better selection
        ShuffleArrayU32(
            solver->r,
            solver->parameters.population_size,
            0,
            solver->parameters.population_size - 1
        );
        for (u32 i = 0; i < solver->parameters.population_size - 1; i++) {
            // Crossover
            u32 p1_idx = solver->r[i];
            u32 p2_idx = solver->r[i + 1];
            u32 c_idx = solver->parameters.population_size;
            GASolverCrossoverIndividuals(
                solver,
                p1_idx,
                p2_idx,
                c_idx
            );

            f64 p1_fitness = solver->population_fitness[p1_idx];
            f64 child_fitness = TourEvaluate(
                solver->parameters.problem,
                TourArrayAt(&solver->population, solver->parameters.population_size)
            );
            f64 replacement_probability = child_fitness/(p1_fitness + child_fitness);
            if (child_fitness > p1_fitness && CoinFlip(replacement_probability)) {
                TourCopy(
                    TourArrayAt(&solver->population, p1_idx),
                    TourArrayAt(&solver->population, c_idx), 
                    solver->parameters.problem->n_cities
                );
            }
        }

        // Mutation
        for (u32 i = 0; i < solver->parameters.population_size; i++) {
            if (CoinFlip(solver->parameters.mutation_rate)) {
                GASolverMutateIndividual(
                    solver,
                    i,
                    RandomF64(0.0, solver->parameters.max_mutation_strength)
                );
            }
        }
        GASolverCalculatePopulationFitness(solver);

        // TODO: log stuff out
    }
}

static u32 *GASolverBestIndividual(GASolver *solver) {
    f64 best_fitness = solver->population_fitness[0];
    u32 *best_tour = TourArrayAt(&solver->population, 0);
    for (u32 i = 1; i < solver->parameters.population_size; i++) {
        f64 fitness = solver->population_fitness[i];
        if (fitness > best_fitness) {
            best_fitness = fitness;
            best_tour = TourArrayAt(&solver->population, i);
        }
    }
    return best_tour;
}

static void GASolverRandomizeIndividual(GASolver *solver, u32 index) {
    assert(index < solver->parameters.population_size);
    TourRandomize(
        TourArrayAt(&solver->population, index),
        solver->parameters.problem->n_cities
    );
}

static void GASolverCalculatePopulationFitness(GASolver *solver) {
    for (u32 i = 0; i < solver->parameters.population_size; i++) {
        solver->population_fitness[i] = TourEvaluate(
            solver->parameters.problem,
            TourArrayAt(&solver->population, i)
        );
    }
}

static void GASolverMutateIndividual(GASolver *solver, u32 index, f64 strength) {
    (void)strength;
    assert(0.0 <= strength && strength <= solver->parameters.max_mutation_strength);
    u32 *individual = TourArrayAt(&solver->population, index);

    // TODO: 1/2 of mutations being single swaps is arbitrary and should be a parameter
    if (CoinFlip(0.5)) {
        // Swap mutation
        u32 i = RandomU32(0, solver->parameters.problem->n_cities - 1);
        u32 j = RandomU32(0, solver->parameters.problem->n_cities - 1);
        u32 temp = individual[i];
        individual[i] = individual[j];
        individual[j] = temp;
    } else if (CoinFlip(0.5)) {
        // PSM mutation
        // TODO: incorporate strength
        u32 mutation_start = RandomI32(0, solver->parameters.problem->n_cities - 1);
        u32 mutation_end = RandomI32(0, solver->parameters.problem->n_cities - 1);
        ShuffleArrayU32(individual, solver->parameters.problem->n_cities, mutation_start, mutation_end);
    } else {
        // RSM mutation
        // TODO: incorporate strength
        u32 mutation_start = RandomI32(0, solver->parameters.problem->n_cities - 1);
        u32 mutation_end = RandomI32(0, solver->parameters.problem->n_cities - 1);
        ReverseArrayU32(individual, solver->parameters.problem->n_cities, mutation_start, mutation_end);
    }
}

static void GASolverCrossoverIndividuals(GASolver *solver, u32 p1_index, u32 p2_index, u32 c_index) {
    TableClear(&solver->child_city_table);

    u32 *parent1 = TourArrayAt(&solver->population, p1_index);
    u32 *parent2 = TourArrayAt(&solver->population, p2_index);
    u32 *child = TourArrayAt(&solver->population, c_index);

    // Copy from parent1
    i32 crossover_idx = RandomI32(0, solver->parameters.problem->n_cities - 1);
    u32 crossover_len = RandomI32(0, solver->parameters.problem->n_cities);
    for (u32 j = 0; j < crossover_len; j++) {
        u32 city = parent1[(crossover_idx + j)%solver->parameters.problem->n_cities];
        child[j] = city;
        TableInsert(&solver->child_city_table, city);
    }

    // Copy from parent2
    u32 cities_to_add = solver->parameters.problem->n_cities - crossover_len;
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
        parent2_idx = (parent2_idx + 1)%solver->parameters.problem->n_cities;
    }
}
// ====================
