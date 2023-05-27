#include <iostream>

#include "../commons/PriorityQueue.hpp"

/*
 * TODO:
 * Тестирование для 10, 100, 1000 заявок по 2 секунды каждая.
 */


int main(){
    PriorityQueue priorityQueue;

    if (!priorityQueue.create()){
        std::cerr << "Cannot create shared memory." << std::endl;
        return -1;
    }

    if (priorityQueue.memoryMap() == -1) {
        std::cout << "Cannot create mapping info." << std::endl;
        return -1;
    }

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

