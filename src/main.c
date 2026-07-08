#include "island.h"
#include "str.h"

i32 main(i32 argc, char *argv[]) {
    InitWorkers(&argc, &argv);
    SeedRNG((RANDOM_SEED + WorkerRank())*WorkerRank());
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
        FILE *fitness_summary_file = NULL;
        if (LogSummaryFromConfig(config)) {
            fitness_summary_file = fopen(StrConcatenate(4, argv[3], "/", island_name, ".summary.csv"), "w");
        }
        FILE *edge_profile_file = NULL;
        if (LogProfileFromConfig(config)) {
            edge_profile_file = fopen(StrConcatenate(4, argv[3], "/", island_name, ".profile"), "w");
        }

        parameters.fitness_summary_file = fitness_summary_file;
        parameters.edge_profile_file = edge_profile_file;

        GAIsland island = GAIslandInit(&problem, parameters);
        if (!GAIslandOkay(&island)) {
            WorkerPanicf(ANY_RANK, "Couldn't initialize island!\n");
        }

        u32 n_migrants = migration_rate*parameters.population_size;

        // (n_src + 1) buffers. One for outgoing and the rest for incoming migrants.
        TourArray migrants = TourArrayInit(problem.n_cities, n_migrants*(n_src + 1));
        if (!TourArrayOkay(&migrants)) {
            WorkerPanicf(ANY_RANK, "Couldn't initialize migration buffer!\n");
        }

        for (u32 epoch = 0; epoch < n_epochs - 1; epoch++) {
            WorkerPrintf(ANY_RANK, "Evolving for %u generations at epoch %u\n", epoch_length, epoch);
            GAIslandEvolve(&island, epoch_length);
            WorkerPrintf(ANY_RANK, "Evolution complete\n");

            // TODO: migration policy
            // TODO: some wasted steps here when n_src = 0 or n_dst = 0, but it's not a big deal.
            TourArray outgoing_migrants = TourArraySlice(&migrants, problem.n_cities, 0, n_migrants);
            GAIslandFillMigrants(&island, &outgoing_migrants);
            for (i32 i = 0; i < n_dst; i++) {
                MigrateTo(&outgoing_migrants, problem.n_cities, dst_islands[i]);
            }
            for (i32 i = 0; i < n_src; i++) {
                TourArray incoming_migrants = TourArraySlice(&migrants, problem.n_cities, i + 1, n_migrants);
                MigrateFrom(&incoming_migrants, problem.n_cities, src_islands[i]);
            }
            if (n_src > 0) {
                for (u32 i = 0; i < n_migrants; i++) {
                    i32 take_from = RandomInt(0, n_src - 1);
                    TourArray incoming_migrants = TourArraySlice(&migrants, problem.n_cities, take_from + 1, n_migrants);
                    Tour migrant = TourArrayAt(&incoming_migrants, problem.n_cities, i);
                    Tour dst = TourArrayAt(&island.population, problem.n_cities, i);
                    TourCopy(&dst, &migrant);
                }
            }
        }
        WorkerPrintf(ANY_RANK, "Last evolution for %u generations\n", epoch_length);
        GAIslandEvolve(&island, epoch_length);

        MPI_Barrier(MPI_COMM_WORLD);

        Tour best_tour = GAIslandBestIndividual(&island);
        TourWriteToFile(&best_tour, "TSP tour", "Found by GA island", StrConcatenate(4, argv[3], "/", island_name, ".tour"));

        WorkerPrintf(ANY_RANK, "Finished!\n");
        TourArrayFree(&migrants);
        GAIslandFree(&island);
        if (edge_profile_file) {
            fclose(edge_profile_file);
        }
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
