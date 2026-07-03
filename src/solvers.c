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
    assert(TSPInstanceOkay(tsp_instance));
    if (starting_city >= tsp_instance->n_cities) {
        starting_city = 0;
    }
    u32 *tour = TourInit(tsp_instance->n_cities);
    if (!tour) {
        return NULL;
    }

    Table visited_table = TableInit(tsp_instance->n_cities);

    tour[0] = starting_city;
    TableInsert(&visited_table, tour[0]);
    for (u32 i = 1; i < tsp_instance->n_cities; i++) {
        WorkerPrintf(ANY_RANK, "Greedy at iteration %u...\n", i);
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

    // For optional statistics
    FILE *summary_file;
    FILE *edge_entropy_file;
    FILE *edge_heat_file;
    Counter edge_counter; // for edge statistics

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
static void GASolverDoLogHeaders(GASolver *solver);
static void GASolverDoLogs(GASolver *solver);

u32 *SolveGA(GAParameters parameters) {
    GASolver solver = GASolverInit(parameters);
    if (!GASolverOkay(&solver)) {
        return NULL;
    }
    u32 *best_tour = TourInit(parameters.problem->n_cities);
    if (!best_tour) {
        WorkerPrintf(ANY_RANK, "Couldn't allocate tour buffer!\n");
        GASolverFree(&solver);
        return NULL;
    }

    GASolverDoLogHeaders(&solver);
    GASolverDoLogs(&solver);
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
        WorkerPrintf(ANY_RANK, "Didn't provide valid TSP instance!\n");
        return solver;
    }

    if (
        (parameters.edge_entropy_out || parameters.edge_heat_out) &&
        parameters.problem->n_cities > MAX_CITIES_FOR_EDGE_STATISTICS
    ) {
        WorkerPrintf(
            ANY_RANK,
            "Can't log edge statistics when problem size is greater than %u!\n",
            MAX_CITIES_FOR_EDGE_STATISTICS
        );
        return solver;
    }

    if (parameters.population_size <= 1) {
        WorkerPrintf(ANY_RANK, "Population must be at least 2!\n");
        return solver;
    }

    if (parameters.mutation_rate < 0.0 || parameters.mutation_rate > 1.0) {
        WorkerPrintf(ANY_RANK, "Mutation rate must be between 0.0 and 1.0!\n");
        return solver;
    }

    if (parameters.max_mutation_strength < 0.0 || parameters.max_mutation_strength > 1.0) {
        WorkerPrintf(ANY_RANK, "Max mutation strength must be between 0.0 and 1.0!\n");
        return solver;
    }

    if (parameters.seed_in && (parameters.seed_percentage < 0.0 || parameters.seed_percentage > 1.0)) {
        WorkerPrintf(ANY_RANK, "Seed percentage needs to be between 0.0 and 1.0!\n");
        return solver;
    }

    // + 1 for children that are created (TODO: use a whole different buffer for children)
    solver.population = TourArrayInit(parameters.problem, parameters.population_size + 1);
    if (!TourArrayOkay(&solver.population)) {
        WorkerPrintf(ANY_RANK, "Couldn't allocate population!\n");
        return solver;
    }

    // + 1 for children that are created (TODO: use a whole different buffer for children)
    solver.population_fitness = calloc(parameters.population_size + 1, sizeof(*solver.population_fitness));
    if (!solver.population_fitness) {
        WorkerPrintf(ANY_RANK, "Couldn't allocate population fitness buffer!\n");
        TourArrayFree(&solver.population);
        return solver;
    }

    solver.r = calloc(parameters.population_size, sizeof(*solver.r));
    if (!solver.r) {
        WorkerPrintf(ANY_RANK, "Couldn't allocate population shuffle buffer!\n");
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
        WorkerPrintf(ANY_RANK, "Couldn't allocate child city table!\n");
        free(solver.r);
        solver.r = NULL;
        free(solver.population_fitness);
        solver.population_fitness = NULL;
        TourArrayFree(&solver.population);
        return solver;
    }

    if (parameters.edge_entropy_out || parameters.edge_heat_out) {
        solver.edge_counter = CounterInit(
            parameters.problem->n_cities*(parameters.problem->n_cities - 1)/2
        );
        if (!CounterOkay(&solver.edge_counter)) {
            WorkerPrintf(ANY_RANK, "Couldn't allocate edge counter!\n");
            TableFree(&solver.child_city_table);
            free(solver.r);
            solver.r = NULL;
            free(solver.population_fitness);
            solver.population_fitness = NULL;
            TourArrayFree(&solver.population);
            return solver;
        }
    }

    for (u32 i = 0; i < parameters.population_size; i++) {
        GASolverRandomizeIndividual(&solver, i);
    }
    // Add seed
    if (parameters.seed_in) {
        // TODO: fix all this ugly nesting
        u32 *seed_tour = TourInit(parameters.problem->n_cities);
        if (!TourOkay(seed_tour)) {
            WorkerPrintf(ANY_RANK, "Couldn't allocate seed tour!\n");
        } else {
            if (!TourReadFromFile(seed_tour, parameters.problem->n_cities, parameters.seed_in)) {
                WorkerPrintf(ANY_RANK, "Couldn't allocate seed tour!\n");
            } else {
                u32 n_seeds = parameters.seed_percentage*parameters.population_size;
                for (u32 i = 0; i < n_seeds; i++) {
                    TourCopy(
                        TourArrayAt(&solver.population, i),
                        seed_tour,
                        parameters.problem->n_cities
                    );
                }
            }
            TourFree(seed_tour);
        }
    }
    GASolverCalculatePopulationFitness(&solver);

    if (parameters.summary_out) {
        solver.summary_file = fopen(parameters.summary_out, "w");
    }
    if (parameters.edge_entropy_out) {
        solver.edge_entropy_file = fopen(parameters.edge_entropy_out, "w");
    }
    if (parameters.edge_heat_out) {
        solver.edge_heat_file = fopen(parameters.edge_heat_out, "w");
    }

    return solver;
}

static void GASolverFree(GASolver *solver) {
    assert(GASolverOkay(solver));
    if (solver->edge_heat_file) {
        fclose(solver->edge_heat_file);
    }
    if (solver->edge_entropy_file) {
        fclose(solver->edge_entropy_file);
    }
    if (solver->summary_file) {
        fclose(solver->summary_file);
    }
    if (solver->parameters.edge_entropy_out || solver->parameters.edge_heat_out) {
        CounterFree(&solver->edge_counter);
    }
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
    WorkerPrintf(ANY_RANK, "Evolving for %u generations...\n", n_generations);
    for (u32 i = 0; i < n_generations; i++) {
        WorkerPrintf(ANY_RANK, "At generation: %u/%u\n", i + 1, n_generations);

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

        GASolverDoLogs(solver);
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
    // } else if (CoinFlip(0.5)) {
    //     // PSM mutation
    //     i32 mutation_point = RandomI32(0, solver->parameters.problem->n_cities - 1);
    //     i32 half_mutation_length = RoundNearest(0.5*strength*solver->parameters.problem->n_cities);
    //     i32 mutation_start = mutation_point - half_mutation_length;
    //     if (mutation_start < 0) {
    //         mutation_start += solver->parameters.problem->n_cities;
    //     }
    //     i32 mutation_end = (mutation_point + half_mutation_length)%solver->parameters.problem->n_cities;
    //     ShuffleArrayU32(individual, solver->parameters.problem->n_cities, mutation_start, mutation_end);
    } else {
        // RSM mutation
        i32 mutation_point = RandomI32(0, solver->parameters.problem->n_cities - 1);
        i32 half_mutation_length = RoundNearest(0.5*strength*solver->parameters.problem->n_cities);
        i32 mutation_start = mutation_point - half_mutation_length;
        if (mutation_start < 0) {
            mutation_start += solver->parameters.problem->n_cities;
        }
        i32 mutation_end = (mutation_point + half_mutation_length)%solver->parameters.problem->n_cities;
        ReverseArrayU32(individual, solver->parameters.problem->n_cities, mutation_start, mutation_end);
    }
}

static void GASolverCrossoverIndividuals(GASolver *solver, u32 p1_index, u32 p2_index, u32 c_index) {
    TableClear(&solver->child_city_table);

    u32 *parent1 = TourArrayAt(&solver->population, p1_index);
    u32 *parent2 = TourArrayAt(&solver->population, p2_index);
    u32 *child = TourArrayAt(&solver->population, c_index);

    // Copy from parent1
    u32 crossover_idx = RandomU32(0, solver->parameters.problem->n_cities - 1);
    u32 crossover_len = RandomU32(0, solver->parameters.problem->n_cities);
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

static void GASolverDoLogHeaders(GASolver *solver) {
    if (solver->summary_file) {
        fprintf(
            solver->summary_file,
            "avg,stddev,min,max\n"
        );
    }

    if (solver->edge_entropy_file) {
        fprintf(
            solver->edge_entropy_file,
            "entropy\n"
        );
    }
}

static void GASolverDoLogs(GASolver *solver) {
    if (solver->summary_file) {
        f64 min = solver->population_fitness[0];
        f64 max = min;
        f64 avg = min;
        for (u32 i = 1; i < solver->parameters.population_size; i++) {
            f64 fit = solver->population_fitness[i];
            if (fit < min) {
                min = fit;
            }
            if (fit > max) {
                max = fit;
            }
            avg += fit;
        }
        avg /= solver->parameters.population_size;

        f64 stddev = 0.0;
        for (u32 i = 0; i < solver->parameters.population_size; i++) {
            f64 fit = solver->population_fitness[i];
            f64 diff_sq = (fit - avg)*(fit - avg);
            stddev += diff_sq;
        }
        stddev = sqrt(stddev/solver->parameters.population_size);

        fprintf(solver->summary_file, "%f,%f,%f,%f\n", avg, stddev, min, max);
    }

    if (solver->edge_entropy_file || solver->edge_heat_file) {
        CounterClear(&solver->edge_counter);
        for (u32 i = 0; i < solver->parameters.population_size; i++) {
            u32 *tour = TourArrayAt(&solver->population, i);
            for (u32 j = 0; j < solver->parameters.problem->n_cities; j++) {
                u32 from = tour[j];
                u32 to = tour[(j + 1)%solver->parameters.problem->n_cities];
                if (from > to) {
                    u32 temp = from;
                    from = to;
                    to = temp;
                }
                u32 row = solver->parameters.problem->n_cities - from - 2;
                u32 col = to - from - 1;
                u32 edge_idx = row*(row + 1)/2 + col;
                CounterIncrement(&solver->edge_counter, edge_idx, 1);
            }
        }
    }

    if (solver->edge_entropy_file) {
        f64 entropy = 0.0;
        u32 max_edges = solver->parameters.problem->n_cities*(solver->parameters.problem->n_cities - 1)/2;
        for (u32 i = 0; i < max_edges; i++) {
            u32 edge_count = CounterCount(&solver->edge_counter, i);
            if (edge_count == 0) {
                continue;
            }
            f64 p = (f64)edge_count/(f64)solver->parameters.population_size;
            entropy -= p*log(p);
        }
        fprintf(solver->edge_entropy_file, "%f\n", entropy);
   }

    if (solver->edge_heat_file) {
        for (u32 from = 0; from < solver->parameters.problem->n_cities - 1; from++) {
            for (u32 to = from + 1; to < solver->parameters.problem->n_cities; to++) {
                u32 row = solver->parameters.problem->n_cities - from - 2;
                u32 col = to - from - 1;
                u32 edge_idx = row*(row + 1)/2 + col;
                u32 edge_count = CounterCount(&solver->edge_counter, edge_idx);
                if (edge_count == 0) {
                    continue;
                }
                f64 p = (f64)edge_count/(f64)solver->parameters.population_size;
                // + 1 since TSPLib indexes cities starting at 1
                fprintf(solver->edge_heat_file, "%u\n%u\n%f\n", from + 1, to + 1, p);
            }
        }
        fprintf(solver->edge_heat_file, "---\n");
    }
}
// ====================

// ==== ISLAND SOLVER ====
typedef struct {
    IslandParameters parameters;
    GASolver ga;
} IslandSolver;

static IslandSolver IslandSolverInit(GAParameters ga_parameters, IslandParameters island_parameters);
static void IslandSolverFree(IslandSolver *solver);
static bool IslandSolverOkay(const IslandSolver *solver);
static void IslandSolverDoMigrations(IslandSolver *solver);

u32 *SolveIsland(GAParameters ga_parameters, IslandParameters island_parameters) {
    IslandSolver solver = IslandSolverInit(ga_parameters, island_parameters);
    if (!IslandSolverOkay(&solver)) {
        return NULL;
    }
    u32 *best_tour = TourInit(ga_parameters.problem->n_cities);
    if (!best_tour) {
        WorkerPrintf(ANY_RANK, "Couldn't allocate tour buffer!\n");
        IslandSolverFree(&solver);
        return NULL;
    }

    u32 generations_left = ga_parameters.max_generations;
    while (generations_left > 0) {
        if (generations_left > island_parameters.epoch_length) {
            GASolverEvolve(&solver.ga, island_parameters.epoch_length);
            IslandSolverDoMigrations(&solver);
            GASolverCalculatePopulationFitness(&solver.ga); // TODO: wasting some time?
            generations_left -= island_parameters.epoch_length;
        } else {
            GASolverEvolve(&solver.ga, generations_left);
            generations_left = 0;
        }
    }
    TourCopy(best_tour, GASolverBestIndividual(&solver.ga), ga_parameters.problem->n_cities);

    IslandSolverFree(&solver);
    return best_tour;
}

static IslandSolver IslandSolverInit(GAParameters ga_parameters, IslandParameters island_parameters) {
    // TODO: check island_parameters
    if (island_parameters.epoch_length >= ga_parameters.max_generations) {
        WorkerPrintf(ANY_RANK, "Migration epoch must be less than max generations of island!\n");
        exit(EXIT_FAILURE);
    }
    if (island_parameters.migration_rate < 0.0 || island_parameters.migration_rate > 1.0) {
        WorkerPrintf(ANY_RANK, "Migration rate must be between 0.0 and 1.0!\n");
        exit(EXIT_FAILURE);
    }
    return (IslandSolver){
        .parameters = island_parameters,
        .ga = GASolverInit(ga_parameters)
    };
}

static void IslandSolverFree(IslandSolver *solver) {
    assert(IslandSolverOkay(solver));
    GASolverFree(&solver->ga);
}

static bool IslandSolverOkay(const IslandSolver *solver) {
    return GASolverOkay(&solver->ga);
}

static void IslandSolverDoMigrations(IslandSolver *solver) {
    // TODO: migration policies...
    assert(IslandSolverOkay(solver));
    i32 dst_rank = solver->parameters.dst_rank;
    i32 src_rank = solver->parameters.src_rank;
    u32 migration_amount = solver->parameters.migration_rate*solver->ga.parameters.population_size;
    u32 n_cities = solver->ga.parameters.problem->n_cities;
    u32 *array = solver->ga.population.tours;
    if (dst_rank != NONE_RANK) {
        // TODO: send
        WorkerPrintf(ANY_RANK, "Sending individuals to %d\n", dst_rank);
        WorkerISendU32(
            array,
            migration_amount*n_cities,
            dst_rank
        );
    }
    if (src_rank != NONE_RANK) {
        // TODO: receive
        WorkerPrintf(ANY_RANK, "Receiving individuals from %d\n", src_rank);
        WorkerReceiveU32(
            array,
            migration_amount*n_cities,
            src_rank
        );
    }
}
// =======================
