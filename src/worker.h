// worker.h: utilities for distributed work
#ifndef WORKER_H
#define WORKER_H
#include <mpi.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "types.h"

#define MAX_WORKERS (1024)

#define NONE_RANK (-2)
#define ANY_RANK (-1)
#define MASTER_RANK (0)

#define WorkerDo(rank, ...) {if ((rank) == ANY_RANK || WorkerRank() == (rank)) {__VA_ARGS__}}
#define MasterDo(...) WorkerDo(MASTER_RANK, __VA_ARGS__)

#define WorkersDo(ranks, n_ranks, ...) {for (int _r = 0; _r < (n_ranks); _r++) WorkerDo((ranks)[_r], __VA_ARGS__)}

#define WorkerPrintf(rank, ...) WorkerDo((rank), printf("[%d]: ", WorkerRank()); printf(__VA_ARGS__);)
#define MasterPrintf(...) WorkerPrintf(MASTER_RANK, __VA_ARGS__)

// Initializes MPI and worker globals
void InitWorkers(int *argc, char ***argv);

// Closes MPI
void DeinitWorkers(void);

// Returns global worker rank
int WorkerRank(void);

// Easy access to argc where needed
int GetArgc(void);

// Easy access to argv where needed
char **GetArgv(void);

// Returns n_procs
int GetWorkerCount(void);

// Send u32 array to another worker
void WorkerSendU32(u32 *array, size n_elements, int dst_rank);

// Send u32 array to another (non-blocking)
// Note: Waits until last non-blocking send finished if there was one
void WorkerISendU32(u32 *array, size n_elements, int dst_rank);

// Receive u32 array from another worker
void WorkerReceiveU32(u32 *array, size n_elements, int src_rank);

// Returns array of all worker ranks
const int *AllWorkers(void);

#endif // WORKER_H
