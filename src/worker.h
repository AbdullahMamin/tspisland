// worker.h: utilities for distributed work
#ifndef WORKER_H
#define WORKER_H
#include <mpi.h>
#include <stdio.h>

#define MASTER_RANK (0)

#define WorkerPrintf(...) {printf("[%d]: ", WorkerRank()); printf(__VA_ARGS__);}

// Initializes MPI and worker globals
void InitWorkers(int *argc, char ***argv);

// Closes MPI
void DeinitWorkers(void);

// Returns global worker rank
int WorkerRank(void);

// Make the master do some work
void MasterDo(void (*work)(void));

// Make non-masters do some work
void SlaveDo(void (*work)(void));

// Easy access to argc where needed
int GetArgc(void);

// Easy access to argv where needed
char **GetArgv(void);

// Returns n_procs
int GetWorkerCount(void);

#endif // WORKER_H
