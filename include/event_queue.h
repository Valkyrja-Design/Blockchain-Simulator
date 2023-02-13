#ifndef _EVENT_QUEUE_H
#define _EVENT_QUEUE_H

#include "event.h"
#include <set>

class event_queue{
    private:
        std::multiset<event> queue;

    public:
        void push(const event& e);
        event pop();

};

#endif