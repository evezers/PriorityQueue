#include <iostream>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "../commons/Request.hpp"
#include "../commons/Info.hpp"
#include "../commons/PrintQueue.hpp"

#include "QuickSort.hpp"

/*
 * TODO:
 * Тестирование для 10, 100, 1000 заявок по 2 секунды каждая.
 */


int main(){
    shm_unlink("priority_queue");
    int shm = shm_open("priority_queue", O_RDWR | O_CREAT, 0777);

    if (shm == -1){
        std::cerr << "Cannot open!" << std::endl;
    }

    // Create an empty message.
    if(lseek(shm, 0, SEEK_END) < sizeof(Info)) {
        // Fill with zero
        lseek(shm, 0, SEEK_SET);
        Info dummy{};
        if(write(shm, &dummy, sizeof(Info)) < sizeof(Info)) {
            std::cout << "Cannot initialize shm" << std::endl;
            return -1;
        }
    }

    Info *info;
    Request *requests;

    if (openPriorityQueue(shm, info, requests) == -1) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    while (true) {
        if (!info->dataSorted) {
            while (info->mutex.try_lock());

            print_queue(info, requests);

            std::qsort(requests, info->count, sizeof(Request),
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

            info->dataSorted = true;

            print_queue(info, requests);

            info->mutex.unlock();
        }
    }


    // Unmap and unlink the shared memory.
    munmap(info, sizeof(Info));
    close(shm);
    shm_unlink("priority_queue");

    // Exit.
    return 0;
}

