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

    priorityQueue.close();

    exit(1);
}

int main(){
    if (!priorityQueue.create()){
        std::cerr << "Cannot create shared memory." << std::endl;
        return -1;
    }

    if (priorityQueue.memoryMap() == -1) {
        std::cout << "Cannot create mapping info." << std::endl;
        return -1;
    }

    struct sigaction sigIntHandler{};

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);

    while (true) {
        if (!priorityQueue.info->dataSorted) {
            while (priorityQueue.info->mutex.try_lock());

            std::cout << priorityQueue;

            std::qsort(priorityQueue.requests, priorityQueue.info->count, sizeof(Request),
                       [](const void* x, const void* y)
                       {
                           const Request arg1 = *static_cast<const Request*>(x);
                           const Request arg2 = *static_cast<const Request*>(y);

                           const auto cmp = arg1 <=> arg2;
                           if (cmp < 0)
                               return -1;
                           if (cmp > 0)
                               return 1;
                           return 0;
                       }
                       );

            priorityQueue.info->dataSorted = true;

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
