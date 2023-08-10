#include "blockchain.h"

#include <iostream>
#include <set>
#include <queue>
#include <iomanip>

int blockchain::get_total_blks() const {
    return 1+added_blocks.size(); 
}

int blockchain::get_owner_blks() {
    // traverse from head to genesis
    int cnt = head->source == owner;
    int prev_id = head->prev_BlkID;
    while (prev_id != genesis_block->BlkID){
        cnt += added_blocks[prev_id]->source == owner;
        prev_id = added_blocks[prev_id]->prev_BlkID;
    }
    return cnt;
}

bool blockchain::blk_exists(int blk_id) const{
    return (genesis_block->BlkID == blk_id || added_blocks.count(blk_id) || buffer.count(blk_id)); 
}

void blockchain::add_child(int blk_id, std::shared_ptr<block> child){
    children[blk_id].emplace_back(child);
}

void blockchain::revert_blk(std::shared_ptr<block> blk
                        , std::unique_ptr<std::vector<long long>>& peer_coins
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool) const
{
    (*peer_coins)[blk->coinbase->id_x] -= blk->coinbase->C;
    for (const auto& txn : blk->txns){
        (*peer_coins)[txn->id_x] += txn->C;
        (*peer_coins)[txn->id_y] -= txn->C;
        txn_pool[txn->txn_id] = txn;
    }

}

void blockchain::cash_blk(std::shared_ptr<block> blk
                        , std::unique_ptr<std::vector<long long>>& peer_coins
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool) const
{
    (*peer_coins)[blk->coinbase->id_x] += blk->coinbase->C;
    for (const auto& txn : blk->txns){
        (*peer_coins)[txn->id_x] -= txn->C;
        (*peer_coins)[txn->id_y] += txn->C;
        txn_pool.erase(txn->txn_id);
    }
}

void blockchain::orphan_chain(std::shared_ptr<block> head
                            , std::shared_ptr<block> tail
                            , std::unique_ptr<std::vector<long long>>& peer_coins
                            , std::map<int, std::shared_ptr<transaction>>& txn_pool)
{
    int curr_id = head->BlkID;
    while (curr_id != tail->BlkID){
        revert_blk(added_blocks[curr_id], peer_coins, txn_pool);
        curr_id = added_blocks[curr_id]->prev_BlkID;
    }
}

std::pair<int, int> blockchain::count_from_buffer(int blk_id){
    // max length chain we can get from buffer
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

std::vector<int> blockchain::add_from_buffer(std::unique_ptr<std::vector<long long>>& peer_coins
                                , bool cash
                                , std::map<int, std::shared_ptr<transaction>>& txn_pool
){
    std::vector<int> ids;   // need ids to forward blocks which were in the buffer till now
    while (true){
        bool flag = false;
        for (auto[id, blk] : buffer){
            // keep adding blocks from buffer until we find their parent in the current blockchain
            if (added_blocks.count(blk->prev_BlkID)){
                // add to added_blocks and set child of blk
                added_blocks[blk->BlkID] = blk;
                children[blk->prev_BlkID].emplace_back(blk);
                if (cash) cash_blk(blk, peer_coins, txn_pool);
                flag = true;
                buffer.erase(id);
                ids.push_back(id);
                break;
            }
        }
        if (!flag) break;
    }
    return ids;
}

std::vector<int> blockchain::add_block(std::shared_ptr<block> blk
                            , std::unique_ptr<std::vector<long long>>& peer_coins
                            , std::map<int, std::shared_ptr<transaction>>& txn_pool
                            , double arrival)
{

    if (blk_exists(blk->BlkID)) return {};

    arrival_times[blk->BlkID] = arrival;

    // std::cout<<"Adding block, ";
    // just add to head
    if (blk->prev_BlkID == head->BlkID){
        len++;
        added_blocks[blk->BlkID] = blk;

        add_child(head->BlkID, blk);
        head = blk;

        // update balances
        cash_blk(blk, peer_coins, txn_pool);
        auto buff_chain = count_from_buffer(blk->BlkID);
        
        auto ids = add_from_buffer(peer_coins, true, txn_pool);
        ids.push_back(blk->BlkID);

        if (buff_chain.second != -1)
            head = added_blocks[buff_chain.second];
            
        return ids;
    } else if (genesis_block->BlkID != blk->prev_BlkID && !added_blocks.count(blk->prev_BlkID)){    // prev_block doesn't exist
        buffer[blk->BlkID] = blk;   // buffer block

        return {};
    } else{

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
            if (prev_id == genesis_block->BlkID)
                prev_id = -1;
            else
                prev_id = added_blocks[prev_id]->prev_BlkID;
        }
        // also count blocks from buffer which become ready
        auto buff_chain = count_from_buffer(blk->BlkID);
        llc += buff_chain.first;

        // if this is greater than curr len then orphan curr chain till lca
        if (llc > len){
            len = llc;

            // now traverse backwards from head to find lca
            prev_id = head->prev_BlkID;
            while (ids.find(prev_id) == ids.end()){
                prev_id = added_blocks[prev_id]->prev_BlkID;
            }
            // prev_id is lca
            // orphan blocks from head till this block
            orphan_chain(head, prev_id==genesis_block->BlkID?genesis_block : added_blocks[prev_id], peer_coins, txn_pool);
            // and add blocks from buffer and cash them
            cash_blk(blk, peer_coins, txn_pool);
            auto ids = add_from_buffer(peer_coins, true, txn_pool);
            int new_head_id = buff_chain.second;

            if (new_head_id == -1){     // no blocks from buffer
                new_head_id = blk->BlkID;
            } 
            head = added_blocks[new_head_id];

            ids.push_back(blk->BlkID);
            return ids;
        } else{
            // just add blocks from buffer but don't cash
            auto ids = add_from_buffer(peer_coins, false, txn_pool);
            ids.push_back(blk->BlkID);
            return ids;
        }
    }
}

void blockchain::print_blockchain(std::fstream& file){
    file <<std::string(40, '-')<<"Blockchain of Peer "<<owner<<std::string(40, '-')<<"\n";
    std::queue<int> q;
    q.push(genesis_block->BlkID);
    std::map<int, bool> visited;
    visited[genesis_block->BlkID] = true;
    for (auto it : added_blocks)
        visited[it.first] = false;

    while (!q.empty()){
        auto u = q.front();
        q.pop();
        file << std::string(30, '-')<<"\n";
        file << std::fixed;
        file << std::setprecision(6);
        file <<"Blk"<<u<<"\t\t|\t\tArrival "<<(u == genesis_block->BlkID ? 0 : arrival_times[u])
                <<"\t\t|\t\tParent Blk"<<(u == genesis_block->BlkID ? -1 : added_blocks[u]->prev_BlkID)
                <<"\t\t|\t\tMiner Peer"<<(u == genesis_block->BlkID ? "" : std::to_string(added_blocks[u]->source))<<"\n";

        if (!children[u].empty()){
            file <<"Children : ";
        }
        for (const auto& blk : children[u]){
            if (!visited[blk->BlkID]){
                file <<"Blk"<<blk->BlkID<<" ";
                visited[blk->BlkID] = true;
                q.push(blk->BlkID);
            }
        }
        if (!children[u].empty()) file <<"\n";
    }
    file <<std::string(100, '-')<<"\n";
}

void blockchain::print_edges(std::fstream& file){
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
                file <<u<<" "<<blk->BlkID<<"\n";
                visited[blk->BlkID] = true;
                q.push(blk->BlkID);
            }
        }
    }
}

