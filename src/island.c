#include "island.h"

GAParameters GAIslandParametersFromConfig(toml_result_t config) {
    GAParameters parameters = {0};

    // === Global parameters ===
    // population_size
    toml_datum_t population_size = toml_seek(config.toptab, "global.population_size");
    if (population_size.type != TOML_INT64) {
        WorkerPanicf(ANY_RANK, "Invalid population size in config!\n");
    }
    parameters.population_size = population_size.u.int64;

    // === Island specifics ===
    toml_datum_t islands = toml_seek(config.toptab, "island");
    if (islands.type != TOML_ARRAY || WorkerRank() >= islands.u.arr.size) {
        WorkerPanicf(ANY_RANK, "Islands not setup properly in config!\n");
    }
    toml_datum_t island = islands.u.arr.elem[WorkerRank()];
    if (island.type != TOML_TABLE) {
        WorkerPanicf(ANY_RANK, "Island not setup properly in config!\n");
    }
    // mutation_rate
    toml_datum_t mutation_rate = toml_seek(island, "mutation_rate");
    if (mutation_rate.type != TOML_FP64) {
        WorkerPanicf(ANY_RANK, "Mutation rate not provided properly!\n");
    }
    parameters.mutation_rate = mutation_rate.u.fp64;
    // max_mutation_strength
    toml_datum_t max_mutation_strength = toml_seek(island, "max_mutation_strength");
    if (max_mutation_strength.type != TOML_FP64) {
        WorkerPanicf(ANY_RANK, "Max mutation strength not provided properly!\n");
    }
    parameters.max_mutation_strength = max_mutation_strength.u.fp64;

    return parameters;
}

f64 MigrationRateFromConfig(toml_result_t config) {
    toml_datum_t migration_rate = toml_seek(config.toptab, "global.migration_rate");
    if (migration_rate.type != TOML_FP64) {
        WorkerPanicf(ANY_RANK, "Migration rate not provided properly!\n");
    }
    f64 rate = migration_rate.u.fp64;
    if (rate < 0.0 || rate > 1.0) {
        WorkerPanicf(ANY_RANK, "Migration rate must be in the range [0.0, 1.0]!\n")
    }
    return rate;
}

u32 EpochLengthFromConfig(toml_result_t config) {
    toml_datum_t epoch_length = toml_seek(config.toptab, "global.epoch_length");
    if (epoch_length.type != TOML_INT64) {
        WorkerPanicf(ANY_RANK, "Epoch length not provided properly!\n");
    }
    u32 length = epoch_length.u.int64;
    if (length == 0) {
        WorkerPanicf(ANY_RANK, "Epoch length must be greater than 0!\n");
    }
    return length;
}

u32 EpochCountFromConfig(toml_result_t config) {
    toml_datum_t n_epochs = toml_seek(config.toptab, "global.n_epochs");
    if (n_epochs.type != TOML_INT64) {
        WorkerPanicf(ANY_RANK, "Epoch count not provided properly!\n");
    }
    u32 epochs = n_epochs.u.int64;
    if (epochs == 0) {
        WorkerPanicf(ANY_RANK, "Epoch count must be greater than 0!\n");
    }
    return epochs;
}

u32 IslandCountFromConfig(toml_result_t config) {
    toml_datum_t islands = toml_seek(config.toptab, "island");
    if (islands.type != TOML_ARRAY) {
        WorkerPanicf(ANY_RANK, "Islands not setup properly in config!\n");
    }
    u32 n_islands = islands.u.arr.size;
    if (n_islands > (u32)WorkerCount()) {
        WorkerPanicf(ANY_RANK, "Too many islands for the number of workers!\n");
    }
    return n_islands;
}

i32 *IslandSourcesFromConfig(toml_result_t config, i32 *n_src) {
    static i32 g_src_islands[MAX_WORKERS];

    toml_datum_t islands = toml_seek(config.toptab, "island");
    if (islands.type != TOML_ARRAY || WorkerRank() >= islands.u.arr.size) {
        WorkerPanicf(ANY_RANK, "Islands not setup properly in config!\n");
    }
    toml_datum_t island = islands.u.arr.elem[WorkerRank()];
    if (island.type != TOML_TABLE) {
        WorkerPanicf(ANY_RANK, "Island not setup properly in config!\n");
    }

    toml_datum_t src_islands = toml_seek(island, "src_islands");
    if (src_islands.type != TOML_ARRAY) {
        WorkerPanicf(ANY_RANK, "Source islands not setup properly in config!\n");
    }
    *n_src = src_islands.u.arr.size;

    for (i32 i = 0; i < *n_src; i++) {
        toml_datum_t src_island = src_islands.u.arr.elem[i];
        if (src_island.type != TOML_INT64) {
            WorkerPanicf(ANY_RANK, "Source islands not setup properly in config!\n")
        }
        i32 src = src_island.u.int64;
        if (src == WorkerRank() || src < 0 || src >= (i32)IslandCountFromConfig(config)) {
            WorkerPanicf(ANY_RANK, "Some source islands are out of range!\n");
        }
        g_src_islands[i] = src;
    }

    return g_src_islands;
}

i32 *IslandDestinationsFromConfig(toml_result_t config, i32 *n_dst) {
    static i32 g_dst_islands[MAX_WORKERS];

    toml_datum_t islands = toml_seek(config.toptab, "island");
    if (islands.type != TOML_ARRAY || WorkerRank() >= islands.u.arr.size) {
        WorkerPanicf(ANY_RANK, "Islands not setup properly in config!\n");
    }
    toml_datum_t island = islands.u.arr.elem[WorkerRank()];
    if (island.type != TOML_TABLE) {
        WorkerPanicf(ANY_RANK, "Island not setup properly in config!\n");
    }

    toml_datum_t dst_islands = toml_seek(island, "dst_islands");
    if (dst_islands.type != TOML_ARRAY) {
        WorkerPanicf(ANY_RANK, "Destination islands not setup properly in config!\n");
    }
    *n_dst = dst_islands.u.arr.size;

    for (i32 i = 0; i < *n_dst; i++) {
        toml_datum_t dst_island = dst_islands.u.arr.elem[i];
        if (dst_island.type != TOML_INT64) {
            WorkerPanicf(ANY_RANK, "Destination islands not setup properly in config!\n")
        }
        i32 dst = dst_island.u.int64;
        if (dst == WorkerRank() || dst < 0 || dst >= (i32)IslandCountFromConfig(config)) {
            WorkerPanicf(ANY_RANK, "Some destination islands are out of range!\n");
        }
        g_dst_islands[i] = dst;
    }

    return g_dst_islands;
}

void GAIslandFillMigrants(GAIsland *island, TourArray *migrants) {
    u32 n_migrants = migrants->capacity/island->problem->n_cities;
    TourArrayCopy(migrants, &island->population, island->problem->n_cities, n_migrants);
}

void MigrateTo(TourArray *migrants, u32 n_cities, i32 dst_rank) {
    assert(TourArrayIsValid(migrants, n_cities));
    WorkerISendArray(migrants, dst_rank);
    u32 n_migrants = migrants->capacity/n_cities;
    WorkerPrintf(ANY_RANK, "Sent %u individuals to %d\n", n_migrants, dst_rank);
}

void MigrateFrom(TourArray *migrants, u32 n_cities, i32 src_rank) {
    assert(TourArrayOkay(migrants));
    WorkerReceiveArray(migrants, src_rank);
    assert(TourArrayIsValid(migrants, n_cities));
    u32 n_migrants = migrants->capacity/n_cities;
    WorkerPrintf(ANY_RANK, "Received %u individuals from %d\n", n_migrants, src_rank);
}
