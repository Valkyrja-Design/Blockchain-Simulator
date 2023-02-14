#include "blockchain.h"

#include <iostream>
#include <set>
#include <queue>

bool blockchain::blk_exists(int blk_id) const{
    return (genesis_block->BlkID == blk_id || added_blocks.count(blk_id) || buffer.count(blk_id)); 
}

void blockchain::add_child(int blk_id, std::shared_ptr<block> child){
    children[blk_id].emplace_back(child);
}

void blockchain::revert_blk(std::shared_ptr<block> blk, std::unique_ptr<std::vector<long long>>& peer_coins) const{
    (*peer_coins)[blk->coinbase->id_x] -= blk->coinbase->C;
    for (const auto& txn : blk->txns){
        (*peer_coins)[txn->id_x] += txn->C;
        (*peer_coins)[txn->id_y] -= txn->C;
    }
}

void blockchain::cash_blk(std::shared_ptr<block> blk, std::unique_ptr<std::vector<long long>>& peer_coins) const{
    (*peer_coins)[blk->coinbase->id_x] += blk->coinbase->C;
    for (const auto& txn : blk->txns){
        (*peer_coins)[txn->id_x] -= txn->C;
        (*peer_coins)[txn->id_y] += txn->C;
    }
}

void blockchain::orphan_chain(std::shared_ptr<block> head
                            , std::shared_ptr<block> tail
                            , std::unique_ptr<std::vector<long long>>& peer_coins)
{
    int curr_id = head->BlkID;
    while (curr_id != tail->BlkID){
        revert_blk(added_blocks[curr_id], peer_coins);
        curr_id = added_blocks[curr_id]->prev_BlkID;
    }
}

std::pair<int, int> blockchain::count_from_buffer(int blk_id){
    int ret = 0;
    int mx_id = -1;
    for (const auto&[id, blk] : buffer){
        int curr = 1;
        int curr_id = blk->BlkID;
        int prev_id = blk->prev_BlkID;
        while (buffer.count(prev_id)){
            curr++;
            prev_id = buffer[prev_id]->prev_BlkID;
        }
        if (blk_id == prev_id){
            ret = std::max(ret, curr);
            mx_id = curr_id;
        }
    }
    return {ret, mx_id};
}

void blockchain::add_from_buffer(std::unique_ptr<std::vector<long long>>& peer_coins, bool cash){
    while (true){
        bool flag = false;
        for (auto[id, blk] : buffer){
            if (added_blocks.count(blk->prev_BlkID)){
                // add to added_blocks and set child of blk
                added_blocks[blk->BlkID] = blk;
                children[blk->prev_BlkID].emplace_back(blk);
                if (cash) cash_blk(blk, peer_coins);
                flag = true;
            }
        }
        if (!flag) break;
    }
}

void blockchain::add_block(std::shared_ptr<block> blk, std::unique_ptr<std::vector<long long>>& peer_coins){

    if (blk_exists(blk->BlkID)) return;
    
    // just add to head
    if (blk->prev_BlkID == head->BlkID){
        len++;
        add_child(head->BlkID, blk);
        head = blk;
        added_blocks[head->BlkID] = head;
        // update balances
        cash_blk(blk, peer_coins);
        // std::cout<<"Blk"<<blk->BlkID<<" HEAD\n";
    } else if (genesis_block->BlkID != blk->prev_BlkID && !added_blocks.count(blk->prev_BlkID)){    // prev_block doesn't exist
        // std::cout<<"Blk"<<blk->BlkID<<" BUFFERED\n";
        buffer[blk->BlkID] = blk;   // buffer block
    } else{
        // std::cout<<"Blk"<<blk->BlkID<<" NOT_HEAD\n";
        // add block to tree
        added_blocks[blk->BlkID] = blk;
        add_child(blk->prev_BlkID, blk);

        // traverse from blk to genesis (root) and store the ids of blocks visited
        std::set<int> ids;
        ids.insert(blk->BlkID);
        int prev_id = blk->prev_BlkID;
        int llc = 1;
        while (prev_id != -1){
            ids.insert(prev_id);
            ++llc;
            prev_id = added_blocks[prev_id]->prev_BlkID;
        }
        // also count blocks from buffer which become ready
        auto buff_chain = count_from_buffer(blk->BlkID);
        llc += buff_chain.first;

        // if this is greater than curr len then orphan curr chain till lca
        if (llc > len){
            // now traverse backwards from head to find lca
            prev_id = head->prev_BlkID;
            while (ids.find(prev_id) == ids.end()){
                prev_id = added_blocks[prev_id]->prev_BlkID;
            }
            // prev_id is lca
            // orphan blocks from head till this block
            orphan_chain(head, added_blocks[prev_id], peer_coins);
            // and add blocks from buffer and cash them
            cash_blk(blk, peer_coins);
            add_from_buffer(peer_coins, true);
            int new_head_id = buff_chain.second;

            if (new_head_id == -1){     // no blocks from buffer
                new_head_id = blk->BlkID;
            } 
            head = added_blocks[new_head_id];
        } else{
            // just add blocks from buffer but don't cash
            add_from_buffer(peer_coins, false);
        }
    }
}

void blockchain::print_blockchain(){
    std::cout<<std::string(30, '-')<<"\n";
    std::cout<<"Current blockchain:\n";
    std::queue<int> q;
    q.push(genesis_block->BlkID);
    std::map<int, bool> visited;
    visited[genesis_block->BlkID] = true;
    for (auto it : added_blocks)
        visited[it.first] = false;

    while (!q.empty()){
        auto u = q.front();
        q.pop();
        for (const auto& blk : children[u]){
            if (!visited[blk->BlkID]){
                std::cout<<"(Blk"<<u<<", Blk"<<blk->BlkID<<"), ";
                visited[blk->BlkID] = true;
                q.push(blk->BlkID);
            }
        }
        std::cout<<"\n";
    }
    std::cout<<std::string(30, '-')<<"\n";
}


