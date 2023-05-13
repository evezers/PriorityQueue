//
// Created by evezers on 13/05/23.
//

#ifndef PRIORITYQUEUE_PRINTQUEUE_HPP
#define PRIORITYQUEUE_PRINTQUEUE_HPP

#include "../commons/Request.hpp"
#include "../commons/Info.hpp"

void print_queue(Info *info, Request *request){
    std::cout << "count: " << info->count << ", sorted: " << info->dataSorted << std::endl;
    std::cout << std::endl;

    for (int i = 0; i < info->count; ++i) {
        std::cout << request[i] << std::endl;
    }
}

#endif //PRIORITYQUEUE_PRINTQUEUE_HPP
