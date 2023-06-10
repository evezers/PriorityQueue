//
// Created by evezers on 27/05/23.
//

#ifndef PRIORITYQUEUE_PRIORITYQUEUE_HPP
#define PRIORITYQUEUE_PRIORITYQUEUE_HPP

#include <mutex>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

class Info{
public:
    std::mutex mutex = std::mutex();
    size_t count = 0;
    size_t maxCount = 1;
    size_t processedCount = 0;
    short requestsId = 1;
    bool dataSorted = false;

    Info() = default;

    Info(const Info &info) : mutex(), count(info.count), maxCount(info.maxCount),
        requestsId(info.requestsId), dataSorted(info.dataSorted) {}

    Info& operator=(const Info &info) {
        count = info.count;
        maxCount = info.maxCount;
        requestsId = info.requestsId;
        dataSorted = info.dataSorted;

        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const Info &info) {
        os << "Priority queue info:" << std::endl
            << "Count: " << std::to_string(info.count) << "/" << std::to_string(info.maxCount)
            << ", processed: " << std::to_string(info.processedCount)
            << ", requestsId: " << std::to_string(info.requestsId)
            << ", dataSorted: " << (info.dataSorted ? "true" : "false");
        return os;
    }
};

class Request{
public:
    unsigned priority;
    unsigned duration;

public:
    auto operator<=>(const Request &rhs) const{
        return priority - rhs.priority;
    }

public:
    Request() :priority(0), duration(0) {}
    Request(unsigned priority, unsigned duration):priority(priority), duration(duration) {}

    friend std::ostream &operator<<(std::ostream &os, const Request &request) {
        os << "Priority: " << std::to_string(request.priority)
           << ", duration: " << std::to_string(request.duration);
        return os;
    }
};

class PriorityQueue {
public:
    Info *info = nullptr;
    Request *requests = nullptr;
    int prevRequestId = 0;

    bool checkRequestId() {
        if (prevRequestId != info->requestsId){
            if (!openRequests()) {
                return false;
            }

            prevRequestId = info->requestsId;
        }

        return true;
    }

    bool create(){
        shm_unlink("priorityQueueInfo");
        int shmInfo = shm_open("priorityQueueInfo", O_RDWR | O_CREAT, 0777);


        if (shmInfo == -1){
            return false;
        }

        memoryMapInfo(shmInfo);
        ::close(shmInfo);

        *info = Info();

        shm_unlink("priorityQueueRequests");
        int shmRequests = shm_open("priorityQueueRequests", O_RDWR | O_CREAT, 0777);

        if (shmRequests == -1){
            return false;
        }

        void *map;
        if((map = mmap(nullptr, info->maxCount * sizeof(Request), PROT_READ | PROT_WRITE,
                       MAP_SHARED, shmRequests, 0)) == MAP_FAILED) {
            return -1;
        }

        ::close(shmRequests);

        requests = reinterpret_cast<Request *>(map);

        return true;
    }

    bool open(){
        int shmInfo = shm_open("priorityQueueInfo", O_RDWR, 0777);

        if (shmInfo == -1){
            return false;
        }

        memoryMapInfo(shmInfo);

        ::close(shmInfo);

        return true;
    }

    bool openRequests(){
        int shmRequests = shm_open("priorityQueueRequests", O_RDWR, 0777);

        if (shmRequests == -1){
            return false;
        }

        memoryMapRequests(shmRequests);

        ::close(shmRequests);

        return true;
    }

    void sort() const{
        if (!info->dataSorted) {
            while (info->mutex.try_lock());

            std::qsort(requests, info->count, sizeof(Request),
                       [](const void* x, const void* y)
                       {
                           const Request arg1 = *static_cast<const Request*>(x);
                           const Request arg2 = *static_cast<const Request*>(y);

                           return static_cast<int>(arg1 <=> arg2);
                       }
            );

            info->dataSorted = true;

            info->mutex.unlock();
        }
    }

    /**
     * Maps priority queue to shared memory.
     *
     * @return current size of memory of priority queue, -1 otherwise.
     */
    ssize_t memoryMapInfo(int shmInfo){
        ftruncate(shmInfo, sizeof(Info));

        // Map the file to memory and obtain a pointer to that region.
        void *map;
        if((map = mmap(nullptr, sizeof(Info), PROT_READ | PROT_WRITE,
                       MAP_SHARED, shmInfo, 0)) == MAP_FAILED) {
            return -1;
        }

        char *sharedMemory = static_cast<char *>(map);

        info = reinterpret_cast<Info *>(sharedMemory);

        return static_cast<ssize_t>(sizeof(Info));
    }

    /**
     * Maps priority queue to shared memory.
     *
     * @return current size of memory of priority queue, -1 otherwise.
     */
    ssize_t memoryMapRequests(int shmRequests){
        auto memorySize = static_cast<off_t>(info->maxCount * sizeof(Request));

        if (ftruncate(shmRequests, memorySize) == -1)
        {
            perror("Error in ftruncate");
            return EXIT_FAILURE;
        }


        // Map the file to memory and obtain a pointer to that region.
        void *map;
        if((map = mmap(nullptr, memorySize, PROT_READ | PROT_WRITE,
                       MAP_SHARED, shmRequests, 0)) == MAP_FAILED) {
            return -1;
        }

        char *sharedMemory = static_cast<char *>(map);

        requests = reinterpret_cast<Request *>(sharedMemory);

        return static_cast<ssize_t>(info->maxCount * sizeof(Request));
    }

    friend std::ostream &operator<<(std::ostream &os, const PriorityQueue &queue) {
        os << *queue.info << std::endl;

        if (queue.info->count){
            std::cout << "Pending requests: " << std::endl;
        } else {
            std::cout << "No pending requests" << std::endl;
        }

        for (size_t i = 0; i < queue.info->count; ++i) {
            os << queue.requests[i] << std::endl;
        }

        os << std::endl;

        return os;
    }

    [[nodiscard]] bool increaseMemory(){
        if (info->count >= info->maxCount){
            while (info->mutex.try_lock());

            int shmRequests = shm_open("priorityQueueRequests", O_RDWR, 0777);

            if (shmRequests == -1){
                return false;
            }

            size_t newCount = info->maxCount * 2 + 1;

            size_t oldSize = info->maxCount * sizeof(Request);
            size_t newSize = newCount * sizeof(Request);

            void *map;
            if((map = mremap(requests, oldSize, newSize, MREMAP_MAYMOVE)) == MAP_FAILED) {
                return false;
            }

            ::close(shmRequests);

            if (map != requests){
                requests = static_cast<Request *>(map);
                info->requestsId++;
            }

            info->maxCount = newCount;

            info->mutex.unlock();
        }

        return true;
    }

    [[nodiscard]] bool push_back(const Request &request) {
        if (!checkRequestId()){
            return false;
        }

        if (info->count == info->maxCount){
            return false;
        }

        while (info->mutex.try_lock());

        requests[info->count] = request;

        info->dataSorted = false;
        info->count++;

        info->mutex.unlock();

        return true;
    }

    [[nodiscard]] Request pop_back() {
        checkRequestId();

        while (info->mutex.try_lock());

        Request request;

        std::copy(requests, &requests[1], &request);
        std::copy(&requests[1], &requests[info->count], requests);

        info->count--;
        info->processedCount++;

        info->mutex.unlock();

        return request;
    }

    void close() const{
        munmap(requests, info->maxCount * sizeof(Request));
        munmap(info, sizeof(Info));
    }

    static void unlink(){
        shm_unlink("priorityQueueRequests");
        shm_unlink("priorityQueueInfo");
    }
};


#endif //PRIORITYQUEUE_PRIORITYQUEUE_HPP
