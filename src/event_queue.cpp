#include "event_queue.h"

void event_queue::push(const event& e){
    queue.insert(e);
}

event event_queue::pop(){
    event e = *queue.begin();
    queue.erase(queue.begin());
    return e;
}



