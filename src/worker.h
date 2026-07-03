// worker.h: utilities for distributed work
#ifndef WORKER_H
#define WORKER_H
#include <mpi.h>
#include <stdio.h>
#include "types.h"

#define ANY_RANK (-1)
#define MASTER_RANK (0)

#define WorkerDo(rank, ...) {if ((rank) == ANY_RANK || WorkerRank() == (rank)) {__VA_ARGS__}}
#define MasterDo(...) WorkerDo(MASTER_RANK, __VA_ARGS__)

#define WorkerPrintf(rank, ...) {printf("[%d]: ", WorkerRank()); printf(__VA_ARGS__);}
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

// Receive u32 array from another worker
void WorkerReceiveU32(u32 *array, size n_elements, int src_rank);

#endif // WORKER_H
