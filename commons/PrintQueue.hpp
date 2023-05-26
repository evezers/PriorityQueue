//
// Created by evezers on 13/05/23.
//

#ifndef PRIORITYQUEUE_PRINTQUEUE_HPP
#define PRIORITYQUEUE_PRINTQUEUE_HPP

#include "../commons/Request.hpp"
#include "../commons/Info.hpp"

void print_queue(Info *info, Request *request){
    std::cout << *info << std::endl;

    if (info->count){
        std::cout << "Pending requests: " << std::endl;
    } else {
        std::cout << "No pending requests" << std::endl;
    }

    for (int i = 0; i < info->count; ++i) {
        std::cout << request[i] << std::endl;
    }

    std::cout << std::endl;
}

/**
 * Maps priority queue to shared memory.
 *
 * @param shm Shared memory descriptor.
 * @param info Information header of priority queue.
 * @param requests Requsts of priority queue.
 * @return current size of memory of priority queue, -1 otherwise.
 */
size_t openPriorityQueue(int shm, Info* &info, Request* &requests){
    // Map the file to memory and obtain a pointer to that region.
    void *map;
    if((map = mmap(nullptr, sizeof(Info), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
        return -1;
    }

    char *sharedMemory = static_cast<char *>(map);

    info = reinterpret_cast<Info *>(sharedMemory);
    requests = reinterpret_cast<Request *>(sharedMemory + sizeof(Info));

    return sizeof(Info) + (info->count * sizeof(Request));
}

#endif //PRIORITYQUEUE_PRINTQUEUE_HPP
