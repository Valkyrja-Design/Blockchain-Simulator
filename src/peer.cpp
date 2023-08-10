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
    if ((*peer_coins)[id] == 0)
        return -1;
    // int coins = randint(1, (*peer_coins)[id]/);
    int coins = 1;
    int idx = randint(0, neighbors.size()-1);
    auto it = std::next(neighbors.begin(), idx);
    std::shared_ptr<transaction> txn = std::make_shared<transaction>(id, it->first, coins);

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
    auto new_balances = std::vector<long long>(*peer_coins);
    for (const auto& txn : blk->txns){
        new_balances[txn->id_x] -= txn->C;
        new_balances[txn->id_y] += txn->C;
        if (new_balances[txn->id_x] < 0) return false;
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
    auto shuff_idx = genshuf(txn_ids.size(), txn_ids.size());
    if(miner!=type::HONEST){
        mining_blk = std::make_shared<block>(lead>0?privatePool.back()->BlkID:topBlockToAttack->BlkID);
    }
    else{
        mining_blk = std::make_shared<block>(blkchain->get_curr_BlkID());
    }
    mining_blk->add_coinbase(id);
    mining_blk->source = id;

    auto temp_balances = std::vector<long long>(*peer_coins);

    // valid block generation
    for (int i=0;i<txn_ids.size();i++){
        auto txn = txn_pool[txn_ids[shuff_idx[i]]];

        if (mining_blk->block_size > 1000 || cnt <= 0)
            return;
        // auto txn = txn_pool[txn_ids[shuff_idx[i]]];

        if (temp_balances[txn->id_x] >= txn->C){
            mining_blk->add_txn(txn);
            temp_balances[txn->id_x] -= txn->C;
            temp_balances[txn->id_y] += txn->C;
            cnt--;
        }
    }
}

std::vector<std::shared_ptr<block>> peer::add_block(std::shared_ptr<block> blk, double arrival){
    auto ids = blkchain->add_block(blk, peer_coins, txn_pool, arrival);
    auto ret = std::vector<std::shared_ptr<block>>();
    for (auto id : ids){
        ret.emplace_back(blkchain->added_blocks[id]);
    }
    return ret;
}

void peer::setMiner(type minerType){
    miner=minerType;
    lead=0;
    topBlockToAttack=blkchain->head; //Start to mine on the head 
}

void peer::addToPrivatePool(std::shared_ptr<block> blk){
    if(lead==0){
        if(topBlockToAttack->BlkID==blk->prev_BlkID){
            lead+=1;
            blk->set_id();
            increment_blks();
            privatePool.emplace_back(blk);
        }
    }
    else if(lead>0){
        if(privatePool.back()->BlkID==blk->prev_BlkID){            
            lead+=1;
            blk->set_id();
            increment_blks();
            privatePool.emplace_back(blk);
        }
    }
}
void peer::print_pvt_pool(std::fstream& file) const{
    file<<"Private pool of "<<id<<" ";
    for(auto blk:privatePool){
        file<<blk->BlkID<<" ";
    }
    file<<"\n";
}