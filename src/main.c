#include "worker.h"

i32 main(i32 argc, char *argv[]) {
    (void)argc;
    (void)argv;
    InitWorkers(&argc, &argv);
    WorkerPrintf(ANY_RANK, "Hello!\n");
    DeinitWorkers();
    return EXIT_SUCCESS;
}
