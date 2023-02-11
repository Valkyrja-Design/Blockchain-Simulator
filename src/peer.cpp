#include "peer.h"
#include <iostream>

int peer::peer_no = 0;

void peer::add_neighbor(std::shared_ptr<peer> p){
    neighbors[p->id] = {p, std::set<int>()};
}

void peer::add_txn(std::shared_ptr<transaction> txn){
    txn_pool[txn->txn_id] = txn;
}

int peer::gen_txn(){
    std::shared_ptr<transaction> txn = std::make_shared<transaction>(id, neighbors.begin()->second.first->get_id(), coins/2);
    add_txn(txn);
    return txn->txn_id;
}

void peer::print_txn_pool(){
    for (const auto& [id, txn] : txn_pool){
        std::cout<<txn->get_txn_name()<<"\n";
    }
}