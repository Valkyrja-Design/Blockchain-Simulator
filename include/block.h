#ifndef _BLOCK_H
#define _BLOCK_H
#define TRANSACTION_SIZE 1
#define MINING_FEE 50
#define MAX_BLK_SIZE 1000
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
#include "transaction.h"

class peer; //declaration

class block{
    public:
        int BlkID;     // Transaction id
        int prevBlkID;
        int BlkSize;   //in KB
        transaction* coinbase_txn;
        std::map<int, std::shared_ptr<transaction>> txns;
        std::vector<block*> next;
        std::shared_ptr<block> parent;
        std::shared_ptr<peer> miner;


        block(int block_id, std::shared_ptr<peer> miner) 
            : BlkID(block_id),miner(miner){
                coinbase_txn=NULL;
                prevBlkID=-1;
                BlkSize=0;
            };
        void add(std::shared_ptr<transaction> txn);
        void add_coinbase_txn(int mining_fee);
        void set_parent(std::shared_ptr<block> b);
        // std::string get_txn_name() const;

        // static int txn_no;
};

#endif