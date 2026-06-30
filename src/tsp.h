// tsp.h: TSP data and utilities
#ifndef TSP_H
#define TSP_H
#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "dsa.h"
#include "vector.h"

// TODO: other kinds of TSP instances (not just 2D euclidean)

// We don't store adjacency matrix since memory cost is quadratic
typedef struct {
    u32 n_cities;
    vec2 *city_positions;
    f64 shortest_edge_length;
    f64 longest_edge_length;
} TSPInstance;

// Read and allocate TSP instance from a .tsp file
TSPInstance TSPInstanceInit(const char *tsp_path);

// Free TSP instance memory
void TSPInstanceFree(TSPInstance *tsp_instance);

// Check if TSP instance reading worked
bool TSPInstanceOkay(const TSPInstance *tsp_instance);

typedef struct {
    u32 n_tours;
    u32 n_cities;
    u32 *tours;
} TourArray;

// Allocate tour array memory
TourArray TourArrayInit(const TSPInstance *tsp_instance, u32 n_tours);

// Free tour array memory
void TourArrayFree(TourArray *tour_array);

// Check if tour array allocation worked
bool TourArrayOkay(const TourArray *tour_array);

// Copies tours from src array to dst array
void TourArrayCopy(TourArray *dst_array, const TourArray *src_array);

// Returns tour at specific index of array
u32 *TourArrayAt(TourArray *tour_array, u32 i);

// Randomizes a tour
void TourRandomize(u32 *tour, u32 n_cities);

// Checks if a tour is valid
// Uses a table to make the complexity O(N)
// If the table is not provided, will create one on the fly and free it
// If the table is provided, will clear the table and fill it, but won't free it after
bool TourIsValid(const u32 *tour, u32 n_cities, Table *table);

// Returns a "score" of a tour by taking the reciprocal of its length multiplied by the longest edge length
f64 TourEvaluate(const TSPInstance *tsp_instance, const u32 *tour);

// Returns the length of a tour according to TSPLib95 rounding
f64 TourLength(const TSPInstance *tsp_instance, const u32 *tour);

// Copies src tour into dst tour
void TourCopy(u32 *dst_tour, const u32 *src_tour, u32 n_cities);

// Allocate a single tour
u32 *TourInit(u32 n_cities);

// Free tour
void TourFree(u32 *tour);

// Checks if tour allocation worked
bool TourOkay(const u32 *tour);

// Reads a tour from a TSP file and returns if it succeeded (tour must already be allocated)
bool TourReadFromFile(u32 *tour, u32 n_cities, const char *file_path);

// Writes a tour to TSP file
void TourWriteToFile(const u32 *tour, u32 n_cities, const char *name, const char *comment, const char *file_path);

#endif // TSP_H
