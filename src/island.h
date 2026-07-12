// island.h: our island GA method
#ifndef ISLAND_H
#define ISLAND_H
#include "ga.h"
#include "worker.h"
#include "tomlc17.h"

GAParameters GAIslandParametersFromConfig(toml_result_t config);

f64 MigrationRateFromConfig(toml_result_t config);
u32 EpochLengthFromConfig(toml_result_t config);
u32 EpochCountFromConfig(toml_result_t config);
u32 IslandCountFromConfig(toml_result_t config);
bool LogSummaryFromConfig(toml_result_t config);
bool LogProfileFromConfig(toml_result_t config);
bool LogEntropyFromConfig(toml_result_t config);

i32 *IslandSourcesFromConfig(toml_result_t config, i32 *n_src);
i32 *IslandDestinationsFromConfig(toml_result_t config, i32 *n_dst);

void GAIslandFillMigrants(GAIsland *island, TourArray *migrants);

void MigrateTo(TourArray *migrants, u32 n_cities, i32 dst_rank);
void MigrateFrom(TourArray *migrants, Table *table, u32 n_cities, i32 src_rank);

#endif // ISLAND_H
