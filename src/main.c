#include "island.h"
#include "str.h"

i32 main(i32 argc, char *argv[]) {
    InitWorkers(&argc, &argv);
    SeedRNG(RANDOM_SEED);
    StrInit(1024*1024); // 1 MB should be enough

    if (argc != 4) {
        MasterPanicf("Usage: tspisland config.toml problem.tsp out_dir\n");
    }

    toml_result_t config = toml_parse_file_ex(argv[1]);
    if (!config.ok) {
        WorkerPanicf(ANY_RANK, "Couldn't parse config toml!\n");
    }

    u32 n_islands = IslandCountFromConfig(config);
    WorkersDo(AllWorkerRanks(), (i32)n_islands,
        TSPInstance problem = TSPInstanceInit(argv[2]);
        if (!TSPInstanceOkay(&problem)) {
            WorkerPanicf(ANY_RANK, "Couldn't parse tsp problem!\n");
        }

        GAParameters parameters = GAIslandParametersFromConfig(config);
        f64 migration_rate = MigrationRateFromConfig(config);
        u32 epoch_length = EpochLengthFromConfig(config);
        u32 n_epochs = EpochCountFromConfig(config);
        i32 n_src;
        i32 *src_islands = IslandSourcesFromConfig(config, &n_src);
        i32 n_dst;
        i32 *dst_islands = IslandDestinationsFromConfig(config, &n_dst);

        char island_name[256];
        sprintf(island_name, "island_%d", WorkerRank());
        FILE *fitness_summary_file = fopen(StrConcatenate(4, argv[3], "/", island_name, ".summary.csv"), "w");
        parameters.fitness_summary_file = fitness_summary_file;

        GAIsland island = GAIslandInit(&problem, parameters);
        if (!GAIslandOkay(&island)) {
            WorkerPanicf(ANY_RANK, "Couldn't initialize island!\n");
        }

        u32 n_migrants = migration_rate*parameters.population_size;

        // TODO: migration buffer needed so last src island doesn't overwrite everything
        // TODO: could also make islands self configure to know how much to send and receive
        ///      to/from each, which means we won't need a seperate migration buffer
        for (u32 epoch = 0; epoch < n_epochs - 1; epoch++) {
            WorkerPrintf(ANY_RANK, "Evolving for %u generations\n", epoch_length);
            GAIslandEvolve(&island, epoch_length);
            WorkerPrintf(ANY_RANK, "Evolution complete\n");
            for (i32 i = 0; i < n_dst; i++) {
                GAIslandMigrateTo(&island, dst_islands[i], n_migrants);
            }
            for (i32 i = 0; i < n_src; i++) {
                GAIslandMigrateFrom(&island, src_islands[i], n_migrants);
            }
        }
        WorkerPrintf(ANY_RANK, "Last evolution for %u generations\n", epoch_length);
        GAIslandEvolve(&island, epoch_length);

        Tour best_tour = GAIslandBestIndividual(&island);
        TourWriteToFile(&best_tour, "TSP tour", "Found by GA island", StrConcatenate(4, argv[3], "/", island_name, ".tour"));

        WorkerPrintf(ANY_RANK, "Finished!\n");
        GAIslandFree(&island);
        if (fitness_summary_file) {
            fclose(fitness_summary_file);
        }
        TSPInstanceFree(&problem);
    );

    toml_free(config);
    StrDeinit();
    DeinitWorkers();
    return EXIT_SUCCESS;
}
