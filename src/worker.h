// worker.h: utilities for distributed work
#ifndef WORKER_H
#define WORKER_H
#include <mpi.h>
#include <stdio.h>

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

#endif // WORKER_H
