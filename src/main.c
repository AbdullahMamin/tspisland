#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "solvers.h"
#include "worker.h"

#define DEFAULT_POPULATION_SIZE (100)
#define DEFAULT_MAX_GENERATIONS (100)
#define DEFAULT_MUTATION_RATE (0.01)
#define DEFAULT_MAX_MUTATION_STRENGTH (0.5)
#define DEFAULT_SEED_PERCENTAGE (0.1)

void MasterWork(void);
void SlaveWork(void);

int main(int argc, char *argv[]) {
    InitWorkers(&argc, &argv);
    SeedRNG();
    MasterDo(MasterWork);
    SlaveDo(SlaveWork);
    DeinitWorkers();
    return EXIT_SUCCESS;
}

void MasterWork(void) {
    WorkerPrintf("Hello from master work!\n");
    char *tsp_in = NULL;
    char *tour_out = NULL;
    char *summary_out = NULL;
    char *entropy_out = NULL;
    char *heat_out = NULL;
    char *seed_in = NULL;
    u32 population_size = DEFAULT_POPULATION_SIZE;
    u32 max_generations = DEFAULT_MAX_GENERATIONS;
    f64 mutation_rate = DEFAULT_MUTATION_RATE;
    f64 max_mutation_strength = DEFAULT_MAX_MUTATION_STRENGTH;
    f64 seed_percentage = DEFAULT_SEED_PERCENTAGE;
    struct option options[] = {
        {"tsp_in", required_argument, NULL, 0},
        {"tour_out", required_argument, NULL, 0},
        {"summary_out", required_argument, NULL, 0},
        {"entropy_out", required_argument, NULL, 0},
        {"heat_out", required_argument, NULL, 0},
        {"seed_in", required_argument, NULL, 0},
        {"population_size", required_argument, NULL, 0},
        {"max_generations", required_argument, NULL, 0},
        {"mutation_rate", required_argument, NULL, 0},
        {"max_mutation_strength", required_argument, NULL, 0},
        {"seed_percentage", required_argument, NULL, 0}
    };

    int c;
    int option_index;
    while ((c = getopt_long_only(GetArgc(), GetArgv(), "", options, &option_index)) != -1) {
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
                entropy_out = optarg;
            } break;
            case 4: {
                heat_out = optarg;
            } break;
            case 5: {
                seed_in = optarg;
            } break;
            case 6: {
                population_size = atoll(optarg);
            } break;
            case 7: {
                max_generations = atoll(optarg);
            } break;
            case 8: {
                mutation_rate = atof(optarg);
            } break;
            case 9: {
                max_mutation_strength = atof(optarg);
            } break;
            case 10: {
                seed_percentage = atof(optarg);
            } break;
            default: {
                WorkerPrintf("getopt returned something weird!\n");
            } break;
            }
        } break;
        case '?': break;
        default: {
            WorkerPrintf("getop returned something weird!\n");
        } break;
        }
    }

    TSPInstance problem = TSPInstanceInit(tsp_in);
    if (!TSPInstanceOkay(&problem)) {
        WorkerPrintf("Couldn't load TSP problem instance!\n");
        exit(EXIT_FAILURE);
    }

    u32 *tour = SolveGA(
        (GAParameters){
            .summary_out = summary_out,
            .edge_entropy_out = entropy_out,
            .edge_heat_out = heat_out,
            .seed_in = seed_in,
            .problem = &problem,
            .population_size = population_size,
            .max_generations = max_generations,
            .mutation_rate = mutation_rate,
            .max_mutation_strength = max_mutation_strength,
            .seed_percentage = seed_percentage
        }
    );
    if (!tour) {
        WorkerPrintf("Couldn't get a tour from GA!\n");
        TSPInstanceFree(&problem);
        exit(EXIT_FAILURE);
    }
    if (tour_out) {
        TourWriteToFile(tour, problem.n_cities, "TSP tour", "Found by GA", tour_out);
    }

    TourFree(tour);
    TSPInstanceFree(&problem);
}

void SlaveWork(void) {
    WorkerPrintf("Hello!\n");
}
