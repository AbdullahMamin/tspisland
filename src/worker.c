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
    WorkerPrintf(ANY_RANK, "Hello!\n");
}

void DeinitWorkers(void) {
    WorkerPrintf(ANY_RANK, "Goodbye!\n");
    MPI_Finalize();
}

int WorkerRank(void) {
    return g_rank;
}

int GetArgc(void) {
    return g_argc;
}

char **GetArgv(void) {
    return g_argv;
}

int GetWorkerCount(void) {
    return g_n_procs;
}

void WorkerSendU32(u32 *array, size n_elements, int dst_rank) {
    MPI_Send(
        array,
        n_elements,
        MPI_UNSIGNED,
        dst_rank,
        0,
        MPI_COMM_WORLD
    );
}

void WorkerReceiveU32(u32 *array, size n_elements, int src_rank) {
    MPI_Recv(
        array,
        n_elements,
        MPI_UNSIGNED,
        src_rank,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE
    );
}
