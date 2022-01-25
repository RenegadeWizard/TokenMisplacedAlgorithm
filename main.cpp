#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include "util/Logger.h"
#include "service/TokenCommunication.h"

int main(int argc, char **argv) {
    int thr;
    MPI_Comm thread_comm;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &thr);
    MPI_Comm_dup(MPI_COMM_WORLD, &thread_comm);

    int size, id;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    TokenCommunication* process;

    if (id == 0) {
        process = new TokenCommunication(id, size, true);
    } else {
        process = new TokenCommunication(id, size);
    }

    while (true) {
        process->waitForToken();
        Logger::info(id, "Critical section");
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        process->sendToken();
    }

    return 0;
}
