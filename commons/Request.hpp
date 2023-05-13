//
// Created by evezers on 12/05/23.
//

#ifndef PRIORITYQUEUE_REQUEST_HPP
#define PRIORITYQUEUE_REQUEST_HPP


class Request{
public:
    uint8_t priority;
    uint8_t duration;

public:
    bool operator<(const Request &rhs) const {
        return priority < rhs.priority;
    }

    bool operator>(const Request &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const Request &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const Request &rhs) const {
        return !(*this < rhs);
    }

public:
    Request() :priority(0), duration(0) {};
    Request(uint8_t priority, uint8_t duration):priority(priority), duration(duration) {}

    friend std::ostream &operator<<(std::ostream &os, const Request &request) {
        os << "p: " << std::to_string(request.priority)
           << ", d: " << std::to_string(request.duration);
        return os;
    }
};


#endif //PRIORITYQUEUE_REQUEST_HPP
