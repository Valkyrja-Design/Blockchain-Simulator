#ifndef _EVENT_H
#define _EVENT_H

enum event_type {TXN_GEN, TXN_GET};

struct event{
    event_type type;
    double time_stamp;

    // use in case of TXN_GET 
    int from_peer;
    int txn_id;

    int peer_id;

    event(event_type type, double time_stamp) : type(type), time_stamp(time_stamp) {}

    bool operator<(const event& e2) const {
        return time_stamp < e2.time_stamp;
    }
};

#endif