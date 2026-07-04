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

typedef Array Tour;

// Allocate a single tour
Tour TourInit(u32 n_cities);

// Free tour
void TourFree(Tour *tour);

// Checks if tour allocation worked
bool TourOkay(const Tour *tour);

// Reads a tour from a TSP file and returns if it succeeded (tour must already be allocated)
bool TourReadFromFile(Tour *tour, const char *file_path);

// Writes a tour to TSP file
void TourWriteToFile(Tour *tour, const char *name, const char *comment, const char *file_path);

// Randomizes a tour
void TourRandomize(Tour *tour);

// Checks if a tour is valid
// Uses a table to make the complexity O(N)
// If the table is not provided, will create one on the fly and free it
// If the table is provided, will clear the table and fill it, but won't free it after
bool TourIsValid(Tour *tour, Table *table);

// Returns a "score" of a tour by taking the reciprocal of its length multiplied by the longest edge length
f64 TourEvaluate(const TSPInstance *tsp_instance, Tour *tour);

// Returns the length of a tour according to TSPLib95 rounding
f64 TourLength(const TSPInstance *tsp_instance, Tour *tour);

// Copies src tour into dst tour
void TourCopy(Tour *dst_tour, const Tour *src_tour);

// Debug print of tour
void TourPrint(Tour *tour);

typedef Array TourArray;

// Allocate tour array memory
TourArray TourArrayInit(u32 n_cities, size n_tours);

// Free tour array memory
void TourArrayFree(TourArray *tour_array);

// Check if tour array allocation worked
bool TourArrayOkay(const TourArray *tour_array);

// Copies tours from src array to dst array
void TourArrayCopy(TourArray *dst_array, const TourArray *src_array, u32 n_cities, size n_tours);

// Returns tour at specific index of array
Tour TourArrayAt(TourArray *tour_array, u32 n_cities, size idx);

#endif // TSP_H
