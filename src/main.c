#include "ga.h"
#include "worker.h"

i32 main(i32 argc, char *argv[]) {
    InitWorkers(&argc, &argv);
    SeedRNG();
    if (argc != 3) {
        MasterPanicf("Usage: tspisland problem.tsp out.tour\n");
    }

    MasterDo(
        TSPInstance problem = TSPInstanceInit(argv[1]);
        MasterPrintf("%s\n", argv[1]);
        if (!TSPInstanceOkay(&problem)) {
            MasterPanicf("Couldn't initialize problem!\n");
        }

        FILE *fitness_summary_file = fopen("out/experiments/berlin52.summary.csv", "w");
        GAIsland island = GAIslandInit(
            &problem,
            (GAParameters){
                .population_size = 1000,
                .mutation_rate = 0.01,
                .fitness_summary_file = fitness_summary_file
            }
        );
        if (!GAIslandOkay(&island)) {
            MasterPanicf("Couldn't initialize island!\n");
        }

        GAIslandEvolve(&island, 1000);
        Tour tour = GAIslandBestIndividual(&island);
        TourWriteToFile(&tour, "TSP tour", "Found by GA", argv[2]);

        GAIslandFree(&island);
        fclose(fitness_summary_file);
        TSPInstanceFree(&problem);
    );

    DeinitWorkers();
    return EXIT_SUCCESS;
}
