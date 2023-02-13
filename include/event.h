#ifndef _EVENT_H
#define _EVENT_H

#include <memory>

#include "transaction.h"
#include "block.h"

enum event_type {TXN_GEN, TXN_GET, BLK_GEN, BLK_GET, BLK_MINE};

struct event{
    event_type type;
    double time_stamp;

    // use in case of TXN_GET 
    int from_peer;
    int txn_id;
    std::shared_ptr<transaction> txn;

    // use in case of BLK_GET
    std::shared_ptr<block> blk;

    int peer_id;

    event(event_type type, double time_stamp) : type(type), time_stamp(time_stamp) {}

    bool operator<(const event& e2) const {
        return time_stamp < e2.time_stamp;
    }
};

#endif