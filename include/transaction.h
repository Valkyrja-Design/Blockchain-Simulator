#ifndef _TRANSACTION_H
#define _TRANSACTION_H

#include <string>

#define TXN_SIZE 8000   // Transaction size in bits
#define MINING_FEE 50

class transaction{
    public:
        int txn_id;     // Transaction id
        int id_x;       // id of peer paying
        int id_y;       // id of peer paid to
        int C;          // #coins paid

        bool is_coinbase;               // is this a coinbase transaction?
        transaction(int id_x, int id_y, int coins)      // normal transaction
                : txn_id(++transaction::txn_no), id_x(id_x), id_y(id_y), C(coins), is_coinbase(false) {}

        transaction(int id_x, int mining_fee)           // coinbase 
                : txn_id(++transaction::txn_no), C(mining_fee), is_coinbase(true) {}

        std::string get_txn_name() const;

        static int txn_no;
};

#endif