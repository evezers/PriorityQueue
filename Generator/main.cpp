#include <iostream>
#include <mutex>
#include <sys/mman.h>
#include <fcntl.h>

#include <unistd.h>

#include "../commons/Request.hpp"
#include "../commons/Info.hpp"
#include "../commons/PrintQueue.hpp"

int main(int argc, char* argv[]){
    int shm = shm_open("priority_queue", O_RDWR, 0777);

    if (shm == -1){
        std::cerr << "Cannot open!" << std::endl;
    }


    // Map the file to memory and obtain a pointer to that region.
    void *map = nullptr;
    if((map = (mmap(nullptr, sizeof(Info), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0))) == MAP_FAILED) {
        std::cout << "Cannot create mapping info" << std::endl;
        return -1;
    }

    Info *info = static_cast<Info *>(map);

    // Map the file to memory and obtain a pointer to that region.
    Request *request = reinterpret_cast<Request *>(static_cast<char *>(map) + sizeof(Info));


    print_queue(info, request);


    munmap(request, sizeof(Request) * info->count);


    if (argc >= 3){
        info->mutex.lock();

        lseek(shm, static_cast<__off_t>(sizeof(Info) + static_cast<unsigned long>(info->count) * sizeof(Request)), SEEK_SET);
        Request newRequest(atoi(argv[1]), atoi(argv[2]));

        auto writtten = write(shm, &newRequest, sizeof(Request));
        if (writtten < sizeof(Request)) {
            std::cout << "Cannot initialize shm" << std::endl;
            return -1;
        }

        std::cout << "Added: " << newRequest << std::endl;

        info->dataSorted = false;
        info->count++;

        info->mutex.unlock();
    }

    close(shm);

    return 0;
}

