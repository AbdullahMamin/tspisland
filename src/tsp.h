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
TSPInstance TSPInstanceInitFromFile(const char *tsp_path);

// Delete TSP instance
void TSPInstanceDelete(TSPInstance tsp_instance);

// Check if TSP instance reading worked
bool TSPInstanceOkay(TSPInstance tsp_instance);

typedef struct {
    u32 n_tours;
    u32 n_cities;
    u32 *tours;
} TourBuffer;

// Allocate tour buffer memory
TourBuffer TourBufferAlloc(TSPInstance tsp_instance, u32 n_tours);

// Free tour buffer memory
void TourBufferFree(TourBuffer tour_buffer);

// Check if tour buffer allocation worked
bool TourBufferOkay(TourBuffer tour_buffer);

// Copies tours from src buffer to dst buffer
void TourBufferCopy(TourBuffer dst_buffer, TourBuffer src_buffer);

// Returns tour at specific index
u32 *TourAt(TourBuffer tour_buffer, u32 index);

// Randomizes a tour
void TourRandomize(u32 *tour, u32 n_cities);

// Checks if a tour is valid
// Uses a table to make the complexity O(N)
// If the table is not provided, will create one on the fly and free it
// If the table is provided, will clear the table and fill it, but won't free it after
bool TourIsValid(u32 *tour, u32 n_cities, Table *table);

// Returns a "score" of a tour by comparing it against an upper bound
f64 TourEvaluate(TSPInstance tsp_instance, u32 *tour);

// Returns the length of a tour
f64 TourLength(TSPInstance tsp_instance, u32 *tour);

// Copies src tour into dst tour
void TourCopy(u32 *tour_dst, u32 *tour_src, u32 n_cities);

// Allocate single tour
u32 *TourAlloc(u32 n_cities);

// Reads a tour from a TSP file and returns if it succeeded
bool TourReadFromFile(u32 *tour, u32 n_cities, const char *file_path);

// Writes found tour to TSP file
void TourWriteToFile(u32 *tour, u32 n_cities, const char *name, const char *comment, const char *file_path);

#endif // TSP_H
