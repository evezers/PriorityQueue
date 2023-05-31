#include <iostream>
#include <csignal>
#include <thread>

#include "../commons/PriorityQueue.hpp"

/*
 * TODO:
 * Тестирование для 10, 100, 1000 заявок по 2 секунды каждая.
 */

PriorityQueue priorityQueue;

void my_handler(int s){
    printf("\nCaught signal %d\n", s);

    priorityQueue.info->count = 0;

    priorityQueue.close();
    PriorityQueue::unlink();

    exit(0);
}

int main(){
    if (!priorityQueue.create()){
        std::cerr << "Cannot create shared memory regions for priority queue." << std::endl;
        return -1;
    }

    struct sigaction sigIntHandler{};

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);
    sigaction(SIGTERM, &sigIntHandler, nullptr);
    sigaction(SIGHUP, &sigIntHandler, nullptr);

    while (true) {
        std::cout << "\033c"; // clear screen

        if (!priorityQueue.increaseMemory()){
            std::cerr << "Memory can't be increased." << std::endl;
        }

        priorityQueue.sort();

        std::cout << priorityQueue;
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    priorityQueue.close();

    PriorityQueue::unlink();

    // Exit.
    return 0;
}

