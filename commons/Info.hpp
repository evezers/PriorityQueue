//
// Created by evezers on 12/05/23.
//

#ifndef PRIORITYQUEUE_INFO_HPP
#define PRIORITYQUEUE_INFO_HPP


class Info{
public:
    std::mutex mutex;
    int count = 0;
    bool dataSorted = false;
};


#endif //PRIORITYQUEUE_INFO_HPP
