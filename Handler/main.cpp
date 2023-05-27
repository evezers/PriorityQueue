#include <iostream>
#include <cstring>
#include <thread>
#include <csignal>

#include "../commons/PriorityQueue.hpp"

PriorityQueue priorityQueue;

Request request1;
bool requestProcessing = false;


void my_handler(int s){
    printf("\nCaught signal %d\n", s);

    if (requestProcessing){
        printf("\nReturning back uncompleted request...\n");

        if (priorityQueue.push_back(request1)){
            std::cout << "Added: " << request1 << std::endl;
        } else {
            std::cout << "Cannot initialize shm" << std::endl;
        }
    }

    exit(1);
}

int main(){
    if (!priorityQueue.open()) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    if (priorityQueue.memoryMap() == -1) {
        std::cout << "Cannot create mapping info." << std::endl;
        return -1;
    }

    std::cout << priorityQueue;

    struct sigaction sigIntHandler{};

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);

    while (priorityQueue.info->count){
        request1 = priorityQueue.pop_back();

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

    priorityQueue.close();

    return 0;
}

