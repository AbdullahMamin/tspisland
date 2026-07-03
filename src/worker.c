#include "worker.h"

// Worker globals
static int g_argc;
static char **g_argv;
static int g_n_procs;
static int g_rank;
static int g_workers[MAX_WORKERS];
static MPI_Request g_isend_requests[MAX_WORKERS];

void InitWorkers(int *argc, char ***argv) {
    MPI_Init(argc, argv);
    g_argc = *argc;
    g_argv = *argv;
    MPI_Comm_size(MPI_COMM_WORLD, &g_n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
    WorkerPrintf(ANY_RANK, "Hello!\n");
    MasterDo(
        if (g_n_procs > MAX_WORKERS) {
            MasterPrintf("Too many workers!\n");
            exit(EXIT_FAILURE);
        }
    )
    for (int i = 0; i < g_n_procs; i++) {
        g_workers[i] = i;
    }
    for (int i = 0; i < g_n_procs; i++) {
        g_isend_requests[i] = MPI_REQUEST_NULL;
    }
}

void DeinitWorkers(void) {
    // Make sure all pending sends are finished
    WorkerWaitAllRequests();
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
    assert(0 <= dst_rank && dst_rank < g_n_procs);
    MPI_Send(
        array,
        n_elements,
        MPI_UNSIGNED,
        dst_rank,
        0,
        MPI_COMM_WORLD
    );
}

void WorkerISendU32(u32 *array, size n_elements, int dst_rank) {
    assert(0 <= dst_rank && dst_rank < g_n_procs);
    // Wait for previous send to finish (if it exists)
    MPI_Wait(&g_isend_requests[dst_rank], MPI_STATUS_IGNORE);
    MPI_Isend(
        array,
        n_elements,
        MPI_UNSIGNED,
        dst_rank,
        0,
        MPI_COMM_WORLD,
        &g_isend_requests[dst_rank]
    );
}

void WorkerReceiveU32(u32 *array, size n_elements, int src_rank) {
    assert(0 <= src_rank && src_rank < g_n_procs);
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

void WorkerWaitAllRequests(void) {
    for (int i = 0; i < g_n_procs; i++) {
        MPI_Wait(&g_isend_requests[i], MPI_STATUS_IGNORE);
    }
}

const int *AllWorkers(void) {
    return g_workers;
}
