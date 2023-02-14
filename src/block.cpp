#include "block.h"
#include <iostream>

int block::block_id = 0;

bool block::add_txn(std::shared_ptr<transaction> txn){
    if (block_size == MAX_BLOCK_SIZE) return false;
    txns.emplace_back(txn);
    block_size++;
    
    return true;
}

void block::add_coinbase(int peer_id){
    coinbase = std::make_unique<transaction>(peer_id, MINING_FEE);
    block_size++;
}

void block::print_blk() const {
    std::cout<<std::string(30, '-')<<"\n";
    std::cout<<"Blk"<<BlkID<<" with txns as follows:\n";
    std::cout<<coinbase->get_txn_name()<<"\n";
    for (const auto& txn : txns){
        std::cout<<txn->get_txn_name()<<"\n";
    }
    std::cout<<std::string(30, '-')<<"\n";
}