// solvers.h: some TSP solvers for testing and evaluation purposes
#ifndef SOLVERS_H
#define SOLVERS_H
#include "tsp.h"

// Just go from 1 -> 2 -> ... -> n
Tour SolveBasic(const TSPInstance *tsp_instance);

// Greedy, nearest neighbour solver TODO: optimize
Tour SolveGreedy(const TSPInstance *tsp_instance, u32 starting_city);

#endif // SOLVERS_H
