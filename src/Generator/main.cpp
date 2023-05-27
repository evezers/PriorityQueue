#include <iostream>

#include "../commons/PriorityQueue.hpp"

int main(int argc, char* argv[]){
    PriorityQueue priorityQueue;

    if (!priorityQueue.open()){
        std::cerr << "Cannot open shared memory 'priority_queue'!" << std::endl;
        return -1;
    }

    if (priorityQueue.memoryMap() == -1) {
        std::cout << "Cannot create mapping info." << std::endl;
        return -1;
    }

    std::cout << priorityQueue;

    if (argc >= 3){
        auto newRequest = Request(
            std::stoi(argv[1]),
            std::stoi(argv[2]));

        if (priorityQueue.push_back(newRequest)){
            std::cout << "Added: " << newRequest << std::endl;
        } else {
            std::cout << "Cannot initialize shm" << std::endl;
        }
    }

    priorityQueue.close();

    return 0;
}

