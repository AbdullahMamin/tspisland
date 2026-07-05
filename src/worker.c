#include "worker.h"

// Worker globals
static i32 g_argc;
static char **g_argv;
static i32 g_n_workers;
static i32 g_rank;
static i32 g_workers[MAX_WORKERS];
static MPI_Request g_requests[MAX_WORKERS];

void InitWorkers(i32 *argc, char ***argv) {
    MPI_Init(argc, argv);
    g_argc = *argc;
    g_argv = *argv;
    MPI_Comm_size(MPI_COMM_WORLD, &g_n_workers);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
    if (g_n_workers > MAX_WORKERS) {
        MasterPanicf("Too many workers!\n");
    }
    for (i32 i = 0; i < g_n_workers; i++) {
        g_workers[i] = i;
    }
    for (i32 i = 0; i < g_n_workers; i++) {
        g_requests[i] = MPI_REQUEST_NULL;
    }
}

void DeinitWorkers(void) {
    // Make sure all pending sends are finished
    WorkerWaitForAllRequests();
    MPI_Finalize();
}

i32 WorkerRank(void) {
    return g_rank;
}

i32 GetArgc(void) {
    return g_argc;
}

char **GetArgv(void) {
    return g_argv;
}

i32 WorkerCount(void) {
    return g_n_workers;
}

void WorkerSendArray(Array *array, i32 dst_rank) {
    assert(0 <= dst_rank && dst_rank < g_n_workers);
    assert(ArrayOkay(array));
    MPI_Send(
        array->data,
        array->capacity,
        MPI_UNSIGNED,
        dst_rank,
        0,
        MPI_COMM_WORLD
    );
}

void WorkerISendArray(Array *array, i32 dst_rank) {
    assert(0 <= dst_rank && dst_rank < g_n_workers);
    assert(ArrayOkay(array));
    // Wait for previous send to finish (if it exists)
    MPI_Wait(&g_requests[dst_rank], MPI_STATUS_IGNORE);
    MPI_Isend(
        array->data,
        array->capacity,
        MPI_UNSIGNED,
        dst_rank,
        0,
        MPI_COMM_WORLD,
        &g_requests[dst_rank]
    );
}

void WorkerReceiveArray(Array *array, i32 src_rank) {
    assert(0 <= src_rank && src_rank < g_n_workers);
    assert(ArrayOkay(array));
    MPI_Recv(
        array->data,
        array->capacity,
        MPI_UNSIGNED,
        src_rank,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE
    );
}

void WorkerWaitForAllRequests(void) {
    for (i32 i = 0; i < g_n_workers; i++) {
        MPI_Wait(&g_requests[i], MPI_STATUS_IGNORE);
    }
}

const i32 *AllWorkerRanks(void) {
    return g_workers;
}
