#include "peer.h"
#include "utilities.h"

#include <iostream>
#include <algorithm>

int peer::peer_no = -1;

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

void peer::print_txn_pool() const {
    for (const auto& [id, txn] : txn_pool){
        std::cout<<txn->get_txn_name()<<"\n";
    }
}

bool peer::verify_txn(const std::shared_ptr<transaction>& txn) const {
    return (*peer_coins)[txn->id_x] >= txn->C;
}

bool peer::verify_block(std::shared_ptr<block> blk) const {
    for (const auto& txn : blk->txns){
        if (!verify_txn(txn)) return false;
    }
    return true;
}

bool peer::verify_chain(int prev_id) const {
    return prev_id == blkchain->get_curr_BlkID();
}

void peer::gen_blk(){
    std::vector<int> txn_ids;
    for (const auto&[id, txn] : txn_pool){
        txn_ids.push_back(id);
    }
    int cnt = randint(0, std::min((int)txn_ids.size(), MAX_BLOCK_SIZE));
    auto shuff_idx = genshuf(cnt, txn_ids.size());

    mining_blk = std::make_shared<block>(blkchain->get_curr_BlkID());
    mining_blk->add_coinbase(id);

    for (int i=0;i<cnt;i++){
        auto txn = txn_pool[txn_ids[shuff_idx[i]]];
        mining_blk->add_txn(txn);
    }
}

void peer::add_block(std::shared_ptr<block> blk){
    blkchain->add_block(blk, peer_coins);
}
