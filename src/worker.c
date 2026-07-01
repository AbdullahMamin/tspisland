#include "worker.h"

// Worker globals
static int g_argc;
static char **g_argv;
static int g_n_procs;
static int g_rank;

void InitWorkers(int *argc, char ***argv) {
    MPI_Init(argc, argv);
    g_argc = *argc;
    g_argv = *argv;
    MPI_Comm_size(MPI_COMM_WORLD, &g_n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
    WorkerPrintf("Hello!\n");
}

void DeinitWorkers(void) {
    WorkerPrintf("Goodbye!\n");
    MPI_Finalize();
}

int WorkerRank(void) {
    return g_rank;
}

void MasterDo(void (*work)(void)) {
    if (g_rank == MASTER_RANK) {
        work();
    }
}

void SlaveDo(void (*work)(void)) {
    if (g_rank != MASTER_RANK) {
        work();
    }
}

int GetArgc(void) {
    return g_argc;
}

char **GetArgv(void) {
    return g_argv;
}
