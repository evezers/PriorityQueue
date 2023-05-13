#include <iostream>
#include <mutex>
#include <sys/mman.h>
#include <fcntl.h>

#include <unistd.h>
#include <cstring>
#include <thread>


#include "../commons/Request.hpp"
#include "../commons/Info.hpp"
#include "../commons/PrintQueue.hpp"


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


Request request1;
bool requestProcessing = false;
int shm;
Info *info;


void my_handler(int s){
    if (requestProcessing){
        printf("\nReturn back\n");

        info->mutex.lock();

        lseek(shm, static_cast<__off_t>(sizeof(Info) + static_cast<unsigned long>(info->count) * sizeof(Request)), SEEK_SET);
//    Request newRequest(atoi(argv[1]), atoi(argv[2]));

        auto writtten = write(shm, &request1, sizeof(Request));
        if (writtten < sizeof(Request)) {
            std::cout << "Cannot initialize shm" << std::endl;
//            return -1;
        }

        std::cout << "Added: " << request1 << std::endl;

        info->dataSorted = false;
        info->count++;

        info->mutex.unlock();
    }

    printf("\nCaught signaldsfds %d\n",s);
    exit(1);
}




int main(){
    shm = shm_open("priority_queue", O_RDWR, 0777);

    if (shm == -1){
        std::cerr << "Cannot open!" << std::endl;
    }


    // Map the file to memory and obtain a pointer to that region.
    void *map;
    if((map = mmap(nullptr, sizeof(Info), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    info = static_cast<Info *>(map);

    // Map the file to memory and obtain a pointer to that region.
    Request *request = reinterpret_cast<Request *>(static_cast<char *>(map) + sizeof(Info));


    print_queue(info, request);


    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);



    while (info->count){


        info->mutex.lock();
        memcpy(&request1, request, sizeof(Request));

        memcpy(request, &request[1], sizeof(Request) * (info->count - 1));

        info->dataSorted = false;
        info->count--;

        info->mutex.unlock();

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
    munmap(request, sizeof(Request) * info->count);

    return 0;
}

