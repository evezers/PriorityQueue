#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

#include "../commons/PriorityQueue.hpp"

/*
 * TODO:
 * Тестирование для 10, 100, 1000 заявок по 2 секунды каждая.
 */

PriorityQueue priorityQueue;

void exitScheduler(int s){
    printf("\nScheduler signal %d\n", s);

    priorityQueue.info->count = 0;

    priorityQueue.close();
    PriorityQueue::unlink();

    exit(0);
}

int main(int argc, char* argv[]){
    auto begin = std::chrono::steady_clock::now();

    if (!priorityQueue.create()){
        std::cerr << "Cannot create shared memory regions for priority queue." << std::endl;
        return -1;
    }

    struct sigaction sigIntHandler{};

    sigIntHandler.sa_handler = exitScheduler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);
    sigaction(SIGTERM, &sigIntHandler, nullptr);
    sigaction(SIGHUP, &sigIntHandler, nullptr);

    int requestsToProcess = (argc == 2) ? std::stoi(argv[1]) : -1;

    while (priorityQueue.info->processedCount < requestsToProcess) {
        std::cout << "\033c"; // clear screen

        if (!priorityQueue.increaseMemory()){
            std::cerr << "Memory can't be increased." << std::endl;
        }

        priorityQueue.sort();

//        std::cout << priorityQueue;
//        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "The time: " << elapsed_ms.count() << " ms\n";

    exitScheduler(0);

    // Exit.
    return 0;
}

