#ifndef _BLOCKCHAIN_H
#define _BLOCKCHAIN_H

#include "block.h"

#include <map>
#include <fstream>

class blockchain{
    public:
        int len;
        int owner;                                          // owner of the blockchain
        std::shared_ptr<block> genesis_block;               // root of tree
        std::shared_ptr<block> head;                        // head of Longest Chain
        std::map<int, std::shared_ptr<block>> added_blocks; // blocks already added
        std::map<int, std::shared_ptr<block>> buffer;
        std::map<int, std::vector<std::shared_ptr<block>>> children;           // children of a block
        std::map<int, double> arrival_times;                            // arrival times of blocks in the blockchain

        blockchain(std::shared_ptr<block> genesis) : len(1), genesis_block(genesis), head(genesis) {}

        bool blk_exists(int blk_id) const;                              // block already added?
        void add_child(int blk_id, std::shared_ptr<block> child);       // add child to a block
        std::vector<int> add_block(std::shared_ptr<block> blk           // add block to this blockchain
                        , std::unique_ptr<std::vector<long long>>& peer_coins
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool
                        , double arrival);
        
        // check if there are blocks ready to be included from buffer
        std::vector<int> add_from_buffer(std::unique_ptr<std::vector<long long>>& peer_coins, bool cash
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool);

        // new longest chain found, orphan previous chain
        void orphan_chain(std::shared_ptr<block> head
                        , std::shared_ptr<block> tail
                        , std::unique_ptr<std::vector<long long>>& peer_coins
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool);

        // update balances of peers
        void cash_blk(std::shared_ptr<block> blk
                        , std::unique_ptr<std::vector<long long>>& peer_coins
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool) const;

        // update balances of peers
        void revert_blk(std::shared_ptr<block> blk
                        , std::unique_ptr<std::vector<long long>>& peer_coins
                        , std::map<int, std::shared_ptr<transaction>>& txn_pool) const;

        // count how many blocks we can add from buffer after adding this block
        std::pair<int, int> count_from_buffer(int blk_id);

        // ID of current head of the blockchain
        int get_curr_BlkID() const { return head->BlkID; }
        // Total blocks in the blockchain
        int get_total_blks() const;
        // Get blocks corresponding to the owner in the Longest Chain
        int get_owner_blks();

        void print_blockchain(std::fstream& file);
        void print_edges(std::fstream& file);
};


#endif