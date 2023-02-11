#ifndef _TRANSACTION_H
#define _TRANSACTION_H

#include <string>

class transaction{
    public:
        int txn_id;     // Transaction id
        int id_x;       // id of peer paying
        int id_y;       // id of peer paid to
        int C;          // #coins paid

        transaction(int id_x, int id_y, int coins) 
                : txn_id(++transaction::txn_no), id_x(id_x), id_y(id_y), C(coins){}

        std::string get_txn_name() const;

        static int txn_no;
};

#endif