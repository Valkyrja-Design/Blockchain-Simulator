#include "transaction.h"

int transaction::txn_no = 0;

std::string transaction::get_txn_name() const {
    return "Txn"+std::to_string(txn_id)+": "
                        +std::to_string(id_x)+" Pays "+std::to_string(id_y)+" "
                        +std::to_string(C)+" coins";
}