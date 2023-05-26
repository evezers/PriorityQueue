#include <iostream>
#include <cstring>
#include <thread>
#include <csignal>

#include <sys/mman.h>
#include <fcntl.h>

#include "../commons/Request.hpp"
#include "../commons/Info.hpp"
#include "../commons/PrintQueue.hpp"

Request request1;
bool requestProcessing = false;
int shm;
Info *pInfo;


void my_handler(int s){
    printf("\nCaught signal %d\n", s);

    if (requestProcessing){
        printf("\nReturning back uncompleted request...\n");

        if (addRequest(shm, *pInfo, request1)){
            std::cout << "Cannot initialize shm" << std::endl;
        } else {
            std::cout << "Added: " << request1 << std::endl;
        }
    }

    exit(1);
}



int main(){
    shm = shm_open("priority_queue", O_RDWR, 0777);

    if (shm == -1){
        std::cerr << "Cannot open shared memory 'priority_queue'!" << std::endl;
        return -1;
    }

    Request *request;

    if (openPriorityQueue(shm, pInfo, request) == -1) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    print_queue(pInfo, request);

    struct sigaction sigIntHandler{};

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);

    while (pInfo->count){
        pInfo->mutex.lock();
        memcpy(&request1, request, sizeof(Request));

        memcpy(request, &request[1], sizeof(Request) * (pInfo->count - 1));

        pInfo->dataSorted = false;
        pInfo->count--;

        pInfo->mutex.unlock();

        std::cout << "Executing: " << request1 << ";";

        requestProcessing = true;

        for (int i = 0; i < request1.duration; ++i) {
            std::cout << ".";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        requestProcessing = false;

        std::cout << "; OK!" << std::endl;
    }

    close(shm);
    munmap(pInfo, sizeof(Info) + (sizeof(Request) * pInfo->count));

    return 0;
}

