#include <iostream>
#include <random>

#include "../commons/PriorityQueue.hpp"

int main(int argc, char* argv[]){
    if (argc != 3) {
        std::cout << "usage: " << argv[0] << " count duration" << std::endl;
        return 1;
    }

    int requestCount = std::stoi(argv[1]);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution dist(1, requestCount);


    PriorityQueue priorityQueue;

    if (!priorityQueue.open()){
        std::cerr << "Cannot open shared memory 'priority_queue'!" << std::endl;
        return -1;
    }

    for (int i = 0; i < requestCount; ++i){
        auto newRequest = Request(
            dist(mt),
            std::stoi(argv[2]));

        if (priorityQueue.push_back(newRequest)){
            std::cout << "Added: " << newRequest << std::endl;
        } else {
            std::cerr << "Cannot write into shm" << std::endl;
        }
    }

    priorityQueue.close();

    return 0;
}

