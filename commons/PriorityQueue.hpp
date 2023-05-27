//
// Created by evezers on 27/05/23.
//

#ifndef PRIORITYQUEUE_PRIORITYQUEUE_HPP
#define PRIORITYQUEUE_PRIORITYQUEUE_HPP

#include <ostream>
#include <mutex>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

class Info{
public:
    std::mutex mutex;
    size_t count = 0;
    bool dataSorted = false;

    friend std::ostream &operator<<(std::ostream &os, const Info &info) {
        os << "Priority queue info:" << std::endl
           << "Count: " << std::to_string(info.count)
           << ", dataSorted: " << (info.dataSorted ? "true" : "false");
        return os;
    }
};

class Request{
public:
    uint8_t priority;
    uint8_t duration;

public:
    auto operator<=>(const Request &rhs) const{
        return priority - rhs.priority;
    }

public:
    Request() :priority(0), duration(0) {};
    Request(uint8_t priority, uint8_t duration):priority(priority), duration(duration) {}

    friend std::ostream &operator<<(std::ostream &os, const Request &request) {
        os << "Priority: " << std::to_string(request.priority)
           << ", duration: " << std::to_string(request.duration);
        return os;
    }
};

class PriorityQueue {
public:
    int shm = 0;
    Info *info = nullptr;
    Request *requests = nullptr;

    bool create(){
        shm_unlink("priority_queue");
        shm = shm_open("priority_queue", O_RDWR | O_CREAT, 0777);

        if (shm == -1){
            return false;
        }

        // Create an empty message.
        if(lseek(shm, 0, SEEK_END) < sizeof(Info)) {
            // Fill with zero
            lseek(shm, 0, SEEK_SET);
            Info dummy{};
            if(write(shm, &dummy, sizeof(Info)) < sizeof(Info)) {
                return false;
            }
        }

        return true;
    }

    bool open(){
        shm = shm_open("priority_queue", O_RDWR, 0777);

        if (shm == -1){
            return false;
        }

        return true;
    }

    /**
     * Maps priority queue to shared memory.
     *
     * @param shm Shared memory descriptor.
     * @param info Information header of priority queue.
     * @param requests Requsts of priority queue.
     * @return current size of memory of priority queue, -1 otherwise.
     */
    size_t memoryMap(){
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

    friend std::ostream &operator<<(std::ostream &os, const PriorityQueue &queue) {
        os << *queue.info << std::endl;

        if (queue.info->count){
            std::cout << "Pending requests: " << std::endl;
        } else {
            std::cout << "No pending requests" << std::endl;
        }

        for (int i = 0; i < queue.info->count; ++i) {
            os << queue.requests[i] << std::endl;
        }

        os << std::endl;

        return os;
    }

    [[nodiscard]] bool add(const Request &request) const{
        info->mutex.lock();

        size_t sharedMemoryLength = sizeof(Info) + (info->count * sizeof(Request));

        auto memoryEnd = static_cast<__off_t>(sharedMemoryLength);

        lseek(shm, memoryEnd, SEEK_SET);

        auto written = write(shm, &request, sizeof(Request));
        if (written < sizeof(Request)) {
            return false;
        }

        info->dataSorted = false;
        info->count++;

        info->mutex.unlock();

        return true;
    }

    void close() const{
        size_t sharedMemoryLength = sizeof(Info) + (info->count * sizeof(Request));

        munmap(info, sharedMemoryLength);

        ::close(shm);
    }

    static void unlink(){
        shm_unlink("priority_queue");
    }
};


#endif //PRIORITYQUEUE_PRIORITYQUEUE_HPP
