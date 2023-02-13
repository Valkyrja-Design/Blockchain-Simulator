#include "blockchain.h"

#include <iostream>

void blockchain::add_block(std::shared_ptr<block> blk, std::shared_ptr<std::vector<int>> peer_coins){
    // get the length of longest chain on adding this block
    int llc = 1;
    int idx = -1;
    if (blk->prev_BlkID == genesis_block->BlkID){
        llc = 1 + count_from_buffer(blk->BlkID);
    } else {
        for (int i=0;i<chain.size();i++){
            if (chain[i]->BlkID == blk->prev_BlkID){
                llc += i+2;         // (i+1 normal blocks) + (1 genesis block)
                idx = i;
                // also count the blocks from buffer which may get added to chain
                llc += count_from_buffer(blk->BlkID);
                break;
            }
        }
    }
    
    if (llc == 1){                  // this means this block's prev_block hasn't arrived yet so add to buffer and return
        buffer.emplace_back(blk);
        return;
    }

    if (llc > len){                                      // new longest chain
        while (chain.size() > idx+1) chain.pop_back();   // orphan all blocks starting from (idx+1)
        chain.push_back(blk);                            // add this block
        update_peer_coins(blk, peer_coins);
        add_from_buffer(blk, peer_coins);                // add blocks from buffer
        len = 1 + chain.size();
    } 
}

void blockchain::add_from_buffer(std::shared_ptr<block> blk, std::shared_ptr<std::vector<int>> peer_coins){
    for (auto it = buffer.begin(); it != buffer.end(); ++it){
        if ((*it)->prev_BlkID == blk->BlkID){
            chain.emplace_back(*it);
            update_peer_coins(*it, peer_coins);
            len++;
            buffer.erase(it);
            add_from_buffer(chain.back(), peer_coins);
            return;
        }
    }
}

int blockchain::count_from_buffer(int prev_id) const {
    int ret = 0;
    while (true){
        bool flag = false;
        for (auto buff_blk : buffer){
            if (buff_blk->prev_BlkID == prev_id){
                prev_id = buff_blk->prev_BlkID;
                ret++;
                flag = true;
            }
        }
        if (!flag) return ret;
    }
}

void blockchain::update_peer_coins(std::shared_ptr<block> blk, std::shared_ptr<std::vector<int>> peer_coins) const {
    (*peer_coins)[blk->coinbase->id_x] += blk->coinbase->C;
    for (const auto& txn : blk->txns){
        (*peer_coins)[txn->id_x] -= txn->C;
        (*peer_coins)[txn->id_y] += txn->C;
    }
}

void blockchain::print_blockchain() const {
    std::cout<<std::string(10, '-')<<"\n";
    std::cout<<"Current blockchain:\n";
    std::cout<<"GB(Blk"<<genesis_block->BlkID<<")";
    for (const auto& blk : chain){
        std::cout<<" <- Blk";
        std::cout<<blk->BlkID;
    }
    std::cout<<"\n";
}