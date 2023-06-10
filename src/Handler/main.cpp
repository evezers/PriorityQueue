#include <iostream>
#include <thread>
#include <csignal>

#include "../commons/PriorityQueue.hpp"

PriorityQueue priorityQueue;

Request currentRequest;
bool requestProcessing = false;


void exitHandler(int s){
    printf("\nHandler signal %d\n", s);

    if (requestProcessing){
        printf("\nReturning back uncompleted request...\n");

        if (priorityQueue.push_back(currentRequest)){
            std::cout << "Added: " << currentRequest << std::endl;
        } else {
            std::cout << "Cannot add to queue." << std::endl;
        }
    }

    priorityQueue.close();

    exit(1);
}

int main(){
    if (!priorityQueue.open()) {
        std::cerr << "Cannot open shared memory regions for priority queue." << std::endl;
        return -1;
    }

    struct sigaction sigIntHandler{};

    sigIntHandler.sa_handler = exitHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);

    while (priorityQueue.info->count){
        currentRequest = priorityQueue.pop_back();

        std::cout << "Executing: " << currentRequest << ";";

        requestProcessing = true;

        for (unsigned i = 0; i < currentRequest.duration; ++i) {
            std::cout << ".";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        requestProcessing = false;

        std::cout << "; OK!" << std::endl;
    }

    exitHandler(0);

    return 0;
}

