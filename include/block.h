#ifndef _BLOCK_H
#define _BLOCK_H

#include <vector>
#include <memory>

#include "transaction.h"

#define MAX_BLOCK_SIZE 1000         // can have maximum 1000 txns besides coinbase

class block{
    public:
        int BlkID;
        int prev_BlkID;                                     // ID of prev block in blockchain

        int block_size;                                     // block_size in #txns
        std::vector<std::shared_ptr<transaction>> txns;      
        std::unique_ptr<transaction> coinbase;              // included in every mined block

        block(int prev_id)
            : BlkID(++block::block_id)
            , prev_BlkID(prev_id)
            , block_size(0) {}

        void add_coinbase(int peer_id);
        bool add_txn(std::shared_ptr<transaction> txn);
        
        void print_blk() const;
        
        static int block_id;
};

#endif