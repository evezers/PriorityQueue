//
// Created by evezers on 12/05/23.
//

#ifndef PRIORITYQUEUE_INFO_HPP
#define PRIORITYQUEUE_INFO_HPP

#include <mutex>

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


#endif //PRIORITYQUEUE_INFO_HPP
