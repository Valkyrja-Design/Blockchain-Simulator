#ifndef _BLOCKCHAIN_H
#define _BLOCKCHAIN_H

#include "block.h"

class blockchain{
    public:
        int len;
        std::shared_ptr<block> genesis_block;
        std::vector<std::shared_ptr<block>> chain;
        std::vector<std::shared_ptr<block>> buffer;             // temporary pool

        blockchain(std::shared_ptr<block> genesis) : len(1), genesis_block(genesis) {}

        void add_block(std::shared_ptr<block> blk,
                        std::shared_ptr<std::vector<int>> peer_coins);       // add a block to the blockchain if possible
        // #blocks we can add from buffer starting at prev_id
        int count_from_buffer(int prev_id) const;                        
        // check if adding a block made buffered blocks ready
        void add_from_buffer(std::shared_ptr<block> blk, std::shared_ptr<std::vector<int>> peer_coins);            
        void update_peer_coins(std::shared_ptr<block> blk, std::shared_ptr<std::vector<int>> peer_coins) const;

        int get_curr_BlkID() const { return chain.empty() ? genesis_block->BlkID : chain.back()->BlkID; };

        void print_blockchain() const;
};


#endif