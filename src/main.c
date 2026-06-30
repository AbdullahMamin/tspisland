#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "solvers.h"

#define DEFAULT_POPULATION_SIZE (100)
#define DEFAULT_MAX_GENERATIONS (100)
#define DEFAULT_MUTATION_RATE (0.01)
#define DEFAULT_MAX_MUTATION_STRENGTH (0.5)

int main(int argc, char *argv[]) {
    SeedRNG();

    char *tsp_in = NULL;
    char *tour_out = NULL;
    char *summary_out = NULL;
    u32 population_size = DEFAULT_POPULATION_SIZE;
    u32 max_generations = DEFAULT_MAX_GENERATIONS;
    f64 mutation_rate = DEFAULT_MUTATION_RATE;
    f64 max_mutation_strength = DEFAULT_MAX_MUTATION_STRENGTH;
    struct option options[] = {
        {"tsp_in", required_argument, NULL, 0},
        {"tour_out", required_argument, NULL, 0},
        {"summary_out", required_argument, NULL, 0},
        {"population_size", required_argument, NULL, 0},
        {"max_generations", required_argument, NULL, 0},
        {"mutation_rate", required_argument, NULL, 0},
        {"max_mutation_strength", required_argument, NULL, 0}
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
            case 6: {
                max_mutation_strength = atof(optarg);
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

    TSPInstance problem = TSPInstanceInit(tsp_in);
    if (!TSPInstanceOkay(&problem)) {
        puts("Couldn't load TSP problem instance!");
        return EXIT_FAILURE;
    }


    u32 *tour = SolveGA(
        (GAParameters){
            .summary_out = summary_out,
            .edge_entropy_out = NULL,
            .edge_heat_out = NULL,
            .problem = &problem,
            .population_size = population_size,
            .max_generations = max_generations,
            .mutation_rate = mutation_rate,
            .max_mutation_strength = max_mutation_strength
        }
    );
    if (!tour) {
        puts("Couldn't get a tour from GA!");
        TSPInstanceFree(&problem);
        return EXIT_FAILURE;
    }
    if (tour_out) {
        TourWriteToFile(tour, problem.n_cities, "TSP tour", "Found by GA", tour_out);
    }

    TourFree(tour);
    TSPInstanceFree(&problem);
    return EXIT_SUCCESS;
}
