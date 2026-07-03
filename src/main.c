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
            }
            ga_parameters.mutation_rate = mutation_rate.u.fp64;

            toml_datum_t max_mutation_strength = toml_seek(config_toml.toptab, "global.max_mutation_strength");
            if (max_mutation_strength.type != TOML_FP64) {
                MasterPrintf("Max mutation strength not entered!\n");
            }
            ga_parameters.max_mutation_strength = max_mutation_strength.u.fp64;

            toml_datum_t population_size = toml_seek(config_toml.toptab, "global.population_size");
            if (population_size.type != TOML_INT64) {
                MasterPrintf("Population size not enetered!\n");
            }
            ga_parameters.population_size = population_size.u.int64;

            toml_datum_t max_generations = toml_seek(config_toml.toptab, "global.max_generations");
            if (max_generations.type != TOML_INT64) {
                MasterPrintf("Max generations not entered!\n");
            }
            ga_parameters.max_generations = max_generations.u.int64;

            u32 *tour = SolveGA(ga_parameters);
            TourWriteToFile(tour, problem.n_cities, "Tour", "Found by GA method", tour_out);
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
