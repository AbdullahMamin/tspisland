#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "solvers.h"
#include "worker.h"
#include "tomlc17.h"

#define DEFAULT_POPULATION_SIZE (100)
#define DEFAULT_MAX_GENERATIONS (100)
#define DEFAULT_MUTATION_RATE (0.01)
#define DEFAULT_MAX_MUTATION_STRENGTH (0.5)
#define DEFAULT_SEED_PERCENTAGE (0.1)
#define DEFAULT_EPOCH_LENGTH (10)
#define DEFAULT_MIGRATION_RATE (0.1)

int main(int argc, char *argv[]) {
    InitWorkers(&argc, &argv);
    SeedRNG();

    char *config_in = NULL;
    char *tsp_in = NULL;
    char *tour_out = NULL;
    struct option options[] = {
        {"config_in", required_argument, NULL, 0},
        {"tsp_in", required_argument, NULL, 0},
        {"tour_out", required_argument, NULL, 0}
    };

    int c;
    int option_index;
    while ((c = getopt_long_only(GetArgc(), GetArgv(), "", options, &option_index)) != -1) {
        if (c == 0) {
            switch (option_index) {
            case 0: {
                config_in = optarg;
            } break;
            case 1: {
                tsp_in = optarg;
            } break;
            case 2: {
                tour_out = optarg;
            } break;
            default: {
                MasterPrintf("getopt returned something weird!\n");
            } break;
            }
        } else if (c != '?') {
            MasterPrintf("getop returned something weird!\n");
        }
    }

    if (!config_in || !tsp_in || !tour_out) {
        MasterPrintf("usage: mpirun -np n_procs tspisland --config_in=config_file --tsp_in=tsp_file --tour_out=tour_file\n");
        return EXIT_FAILURE;
    }

    toml_result_t config_toml = toml_parse_file_ex(config_in);
    if (!config_toml.ok) {
        MasterPrintf("Error reading toml config!\n");
        return EXIT_FAILURE;
    }

    TSPInstance problem = TSPInstanceInit(tsp_in);
    if (!TSPInstanceOkay(&problem)) {
        MasterPrintf("Couldn't read TSP file!\n");
        return EXIT_FAILURE;
    }

    toml_datum_t method = toml_seek(config_toml.toptab, "global.method");
    if (method.type != TOML_STRING) {
        MasterPrintf("Invalid or no method provided in config!\n");
        return EXIT_FAILURE;
    }

    if (strcmp(method.u.s, "greedy") == 0) {
        toml_datum_t starting_city = toml_seek(config_toml.toptab, "global.starting_city");
        if (starting_city.type != TOML_INT64) {
            MasterPrintf("Starting city not entered!\n");
            return EXIT_FAILURE;
        }
        MasterDo(
            u32 *tour = SolveGreedy(&problem, starting_city.u.int64);
            TourWriteToFile(tour, problem.n_cities, "Tour", "Found by greedy method", tour_out);
            TourFree(tour);
        );
    } else if (strcmp(method.u.s, "genetic") == 0) {
        MasterDo(
            GAParameters ga_parameters = {0};
            ga_parameters.problem = &problem;

            toml_datum_t mutation_rate = toml_seek(config_toml.toptab, "global.mutation_rate");
            if (mutation_rate.type != TOML_FP64) {
                MasterPrintf("Mutation rate not entered!\n");
                return EXIT_FAILURE;
            }
            ga_parameters.mutation_rate = mutation_rate.u.fp64;

            toml_datum_t max_mutation_strength = toml_seek(config_toml.toptab, "global.max_mutation_strength");
            if (max_mutation_strength.type != TOML_FP64) {
                MasterPrintf("Max mutation strength not entered!\n");
                return EXIT_FAILURE;
            }
            ga_parameters.max_mutation_strength = max_mutation_strength.u.fp64;

            toml_datum_t population_size = toml_seek(config_toml.toptab, "global.population_size");
            if (population_size.type != TOML_INT64) {
                MasterPrintf("Population size not enetered!\n");
                return EXIT_FAILURE;
            }
            ga_parameters.population_size = population_size.u.int64;

            toml_datum_t max_generations = toml_seek(config_toml.toptab, "global.max_generations");
            if (max_generations.type != TOML_INT64) {
                MasterPrintf("Max generations not entered!\n");
                return EXIT_FAILURE;
            }
            ga_parameters.max_generations = max_generations.u.int64;

            u32 *tour = SolveGA(ga_parameters);
            TourWriteToFile(tour, problem.n_cities, "Tour", "Found by GA method", tour_out);
            TourFree(tour);
        );
    } else if (strcmp(method.u.s, "island") == 0) {
        toml_datum_t islands = toml_seek(config_toml.toptab, "island");
        if (islands.type != TOML_ARRAY) {
            MasterPrintf("Did not provide islands!\n");
            return EXIT_FAILURE;
        }

        toml_datum_t population_size = toml_seek(config_toml.toptab, "global.population_size");
        if (population_size.type != TOML_INT64) {
            MasterPrintf("Did not provide population size!\n");
            return EXIT_FAILURE;
        }

        toml_datum_t max_generations = toml_seek(config_toml.toptab, "global.max_generations");
        if (max_generations.type != TOML_INT64) {
            MasterPrintf("Did not provide max generations!\n");
            return EXIT_FAILURE;
        }

        toml_datum_t epoch_length = toml_seek(config_toml.toptab, "global.epoch_length");
        if (epoch_length.type != TOML_INT64) {
            MasterPrintf("Did not provide epoch length!\n");
            return EXIT_FAILURE;
        }

        toml_datum_t migration_rate = toml_seek(config_toml.toptab, "global.migration_rate");
        if (migration_rate.type != TOML_FP64) {
            MasterPrintf("Did not provide migration rate!\n");
            return EXIT_FAILURE;
        }

        i32 n_islands = islands.u.arr.size;
        if (n_islands > GetWorkerCount()) {
            MasterPrintf("Not enough workers for all the islands!\n");
            return EXIT_FAILURE;
        }

        WorkersDo(AllWorkers(), n_islands,
            GAParameters ga_parameters = {0};
            ga_parameters.problem = &problem;

            IslandParameters island_parameters = {0};

            toml_datum_t island = islands.u.arr.elem[WorkerRank()];
            if (island.type != TOML_TABLE) {
                WorkerPrintf(ANY_RANK, "Invalid island!\n");
                return EXIT_FAILURE;
            }

            bool no_src = false;
            toml_datum_t src_island = toml_seek(island, "src_island");
            if (src_island.type != TOML_INT64) {
                no_src = true;
            }

            bool no_dst = false;
            toml_datum_t dst_island = toml_seek(island, "dst_island");
            if (dst_island.type != TOML_INT64) {
                no_dst = true;
            }

            toml_datum_t mutation_rate = toml_seek(island, "mutation_rate");
            if (mutation_rate.type != TOML_FP64) {
                WorkerPrintf(ANY_RANK, "Did not provide mutation rate!\n");
                return EXIT_FAILURE;
            }

            toml_datum_t max_mutation_strength = toml_seek(island, "max_mutation_strength");
            if (max_mutation_strength.type != TOML_FP64) {
                WorkerPrintf(ANY_RANK, "Did not provide max mutation strength!\n");
                return EXIT_FAILURE;
            }

            if (!no_src && (src_island.u.int64 < 0 || src_island.u.int64 >= n_islands)) {
                WorkerPrintf(ANY_RANK, "src island out of range!\n");
                return EXIT_FAILURE;
            }

            if (!no_dst && (dst_island.u.int64 < 0 || dst_island.u.int64 >= n_islands)) {
                WorkerPrintf(ANY_RANK, "dst island out of range!\n");
                return EXIT_FAILURE;
            }

            ga_parameters.population_size = population_size.u.int64;
            ga_parameters.max_generations = max_generations.u.int64;
            ga_parameters.mutation_rate = mutation_rate.u.fp64;
            ga_parameters.max_mutation_strength = max_mutation_strength.u.fp64;

            island_parameters.epoch_length = epoch_length.u.int64;
            island_parameters.migration_rate = migration_rate.u.fp64;
            island_parameters.dst_ranks[0] = no_dst ? NONE_RANK : dst_island.u.int64;
            island_parameters.src_ranks[0] = no_src ? NONE_RANK : src_island.u.int64;
            island_parameters.n_src = 1;
            island_parameters.n_dst = 1;

            u32 *tour = SolveIsland(ga_parameters, island_parameters);
            // TourWriteToFile(tour, problem.n_cities, "Tour", "Found by island method", tour_out);
            TourFree(tour);
        );
    } else {
        MasterPrintf("Invalid method provided in config!\n");
    }

    TSPInstanceFree(&problem);
    toml_free(config_toml);
    DeinitWorkers();
    return EXIT_SUCCESS;
}
