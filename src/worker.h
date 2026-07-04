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

// Macros to make specific worker do something
#define WorkerDo(rank, ...) {if ((rank) == ANY_RANK || WorkerRank() == (rank)) {__VA_ARGS__}}
#define MasterDo(...) WorkerDo(MASTER_RANK, __VA_ARGS__)

// Macro to make a list of workers do something
#define WorkersDo(ranks, n_ranks, ...) {for (i32 _r = 0; _r < (n_ranks); _r++) WorkerDo((ranks)[_r], __VA_ARGS__)}

// Macros for making specific workers print something
#define WorkerPrintf(rank, ...) WorkerDo((rank), printf("[%d]: ", WorkerRank()); printf(__VA_ARGS__);)
#define MasterPrintf(...) WorkerPrintf(MASTER_RANK, __VA_ARGS__)
#define WorkerPanicf(rank, ...) WorkerDo((rank), printf("[%d] PANIC: ", WorkerRank()); printf(__VA_ARGS__); exit(EXIT_FAILURE);)
#define MasterPanicf(...) WorkerPanicf(MASTER_RANK, __VA_ARGS__)

// Initializes MPI and worker globals
void InitWorkers(i32 *argc, char ***argv);

// Closes MPI
void DeinitWorkers(void);

// Returns current worker rank
i32 WorkerRank(void);

// Easy access to argc where needed
i32 GetArgc(void);

// Easy access to argv where needed
char **GetArgv(void);

// Returns number of workers available
i32 WorkerCount(void);

// Send u32 array to another worker
void WorkerSendU32(u32 *array, size n_elements, i32 dst_rank);

// Send u32 array to another (non-blocking)
// Note: Waits until last non-blocking send finished if there was one
void WorkerISendU32(u32 *array, size n_elements, i32 dst_rank);

// Receive u32 array from another worker
void WorkerReceiveU32(u32 *array, size n_elements, i32 src_rank);

// Waits for all pending requests to finish.
void WorkerWaitForAllRequests(void);

// Returns array of all worker ranks
const i32 *AllWorkerRanks(void);

#endif // WORKER_H
