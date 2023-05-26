#include <iostream>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "../commons/Request.hpp"
#include "../commons/Info.hpp"
#include "../commons/PrintQueue.hpp"

int main(int argc, char* argv[]){
    int shm = shm_open("priority_queue", O_RDWR, 0777);

    if (shm == -1){
        std::cerr << "Cannot open!" << std::endl;
    }

    Info *info;
    Request *request;

    if (openPriorityQueue(shm, info, request) == -1) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    print_queue(info, request);

    size_t sharedMemoryLength = sizeof(Info) + (info->count * sizeof(Request));


    if (argc >= 3){
        info->mutex.lock();

        auto memoryEnd = static_cast<__off_t>(sharedMemoryLength);

        lseek(shm, memoryEnd, SEEK_SET);
        Request newRequest(std::stoi(argv[1]), std::stoi(argv[2]));

        auto written = write(shm, &newRequest, sizeof(Request));
        if (written < sizeof(Request)) {
            std::cout << "Cannot initialize shm" << std::endl;
            return -1;
        }

        std::cout << "Added: " << newRequest << std::endl;

        info->dataSorted = false;
        info->count++;

        info->mutex.unlock();
    }

    munmap(info, sharedMemoryLength);

    close(shm);

    return 0;
}

