#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "solvers.h"

#define DEFAULT_POPULATION_SIZE (100)
#define DEFAULT_MAX_GENERATIONS (100)
#define DEFAULT_MUTATION_RATE (0.01)

int main(int argc, char *argv[]) {
    SeedRNG();

    char *tsp_in = NULL;
    char *tour_out = NULL;
    char *summary_out = NULL;
    u32 population_size = DEFAULT_POPULATION_SIZE;
    u32 max_generations = DEFAULT_MAX_GENERATIONS;
    f64 mutation_rate = DEFAULT_MUTATION_RATE;
    struct option options[] = {
        {"tsp_in", required_argument, NULL, 0},
        {"tour_out", required_argument, NULL, 0},
        {"summary_out", required_argument, NULL, 0},
        {"population_size", required_argument, NULL, 0},
        {"max_generations", required_argument, NULL, 0},
        {"mutation_rate", required_argument, NULL, 0}
    };

    int c;
    int option_index;
    while ((c = getopt_long_only(argc, argv, "", options, &option_index)) != -1) {
        switch (c) {
        case 0: {
            switch (option_index) {
            case 0: {
                tsp_in = optarg;
            } break;
            case 1: {
                tour_out = optarg;
            } break;
            case 2: {
                summary_out = optarg;
            } break;
            case 3: {
                population_size = atoll(optarg);
            } break;
            case 4: {
                max_generations = atoll(optarg);
            } break;
            case 5: {
                mutation_rate = atof(optarg);
            } break;
            default: {
                puts("getopt returned something weird!");
            } break;
            }
        } break;
        case '?': break;
        default: {
            puts("getop returned something weird!");
        } break;
        }
    }

    TSPInstance problem = TSPInstanceInitFromFile(tsp_in);
    if (!TSPInstanceOkay(problem)) {
        puts("Couldn't load TSP problem instance!");
        return EXIT_FAILURE;
    }

    GASolver ga_solver = (GASolver){
        .log_path = summary_out,
        .problem = problem,
        .population_size = population_size,
        .max_generations = max_generations,
        .mutation_rate = mutation_rate,
        .seed_tours = NULL
    };
    if (!GASolverInit(&ga_solver)) {
        puts("Couldn't initialize GA!");
        TSPInstanceDelete(problem);
        return EXIT_FAILURE;
    }

    u32 *tour = GASolverSolve(&ga_solver);
    assert(tour);
    if (tour_out) {
        // TODO: name comment as command arguments
        TourWriteToFile(tour, problem.n_cities, "TSP tour", "Found by GA", tour_out);
    }

    GASolverFree(&ga_solver);
    TSPInstanceDelete(problem);
    return EXIT_SUCCESS;
}
