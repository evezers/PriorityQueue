#include <iostream>
#include <mutex>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


#include "../commons/Request.hpp"
#include "../commons/Info.hpp"
#include "../commons/PrintQueue.hpp"

#include "QuickSort.hpp"

/*
 * Разработка системы, реализующей планировщик очереди.
    Существует процесс, генерирующий заявки различной вычилительной длины (выраженной в секундах)
        и имеющий различный приоритет.
    Существует процесс обработчик, который читает заявки из очереди и обрабатывает их
        в соответствии с заданными параметрами длины.
    Существует процесс - планировщик очереди.
        Его задача: при поступлении очередной заявки переупорядочить очередь в соответствии с приоритетами.

Очередь должна находиться в разделяемой памяти.
 */


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
    if(lseek(shm, 0, SEEK_END) < sizeof(Info)) {// Fill with zero
//    if (ftruncate(shm, 0x60000) == 0){

//        int result = lseek(mmapFd, size - 1, SEEK_SET);
//        if (result == -1)
//        {
//            perror("lseek mmapFd failed");
//            close(mmapFd);
//            return NULL;
//        }

        lseek(shm, 0, SEEK_SET);
        Info dummy{};
        if(write(shm, &dummy, sizeof(Info)) < sizeof(Info)) {
            std::cout << "Cannot initialize shm" << std::endl;
            return -1;
        }
    }

    // Map the file to memory and obtain a pointer to that region.
    void *map = nullptr;

    if((map = (mmap(nullptr, sizeof(Info), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0))) == MAP_FAILED) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    Info *info = static_cast<Info *>(map);

    for (uint8_t i = 10; i > 1; --i) {
        Request request(i, 7);

        auto writtten = write(shm, &request, sizeof(Request));
        if (writtten < sizeof(Request)) {
            std::cout << "Cannot initialize shm" << std::endl;
            return -1;
        }

        std::cout << writtten << " " << request << std::endl;

        info->count++;
    }

    while (true) {
        if (!info->dataSorted) {
            while (info->mutex.try_lock());


            // Map the file to memory and obtain a pointer to that region.
            Request *request = reinterpret_cast<Request *>(static_cast<char *>(map) + sizeof(Info));

            print_queue(info, request);


            //TODO: sort
            quickSortIterative(request, 0, info->count - 1);

            info->dataSorted = true;

            print_queue(info, request);

            munmap(request, sizeof(Request) * info->count);

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

