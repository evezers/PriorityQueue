#include <iostream>
#include <csignal>

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
        if (!priorityQueue.info->dataSorted) {
            while (priorityQueue.info->mutex.try_lock());

            std::cout << priorityQueue;

            std::qsort(priorityQueue.requests, priorityQueue.info->count, sizeof(Request),
                       [](const void* x, const void* y)
                       {
                           const Request arg1 = *static_cast<const Request*>(x);
                           const Request arg2 = *static_cast<const Request*>(y);

                           return static_cast<int>(arg1 <=> arg2);
                       }
                       );

            priorityQueue.info->dataSorted = true;

            if (!priorityQueue.increaseMemory()){
                std::cerr << "Memory can't be increased." << std::endl;
            }

            std::cout << "\033c";
            std::cout << priorityQueue;

            priorityQueue.info->mutex.unlock();
        }
    }

    priorityQueue.close();

    PriorityQueue::unlink();

    // Exit.
    return 0;
}

