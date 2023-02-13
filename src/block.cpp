#include "block.h"
#include "peer.h"
/* add txn in this block */
void block::add(std::shared_ptr<transaction> txn) {
    txns[txn->txn_id]=txn;
    BlkSize += TRANSACTION_SIZE; // size of every txn = 1 KB
}

/* set parent of this block to b */
void block::set_parent(std::shared_ptr<block> b) {
    parent = b;
    // b == NULL indicates genesis block
    prevBlkID = (b == NULL) ? -1 : b->BlkID;
}
void block::add_coinbase_txn(int mining_fee){
    coinbase_txn=new transaction(-1, miner->get_id(), mining_fee); // "-1" pays to miner (mining_fee)
    BlkSize+=TRANSACTION_SIZE;
}
