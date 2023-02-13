#ifndef _PEER_H
#define _PEER_H

#include <vector>
#include <memory>
#include <map>
#include <set>

#include "transaction.h"
#include "blockchain.h"

enum speed {SLOW, FAST};
enum cpu_power {LOW, HIGH};

class peer{
    private:
        int id;
        speed label1;
        cpu_power label2;
        int coins;

    public:

        // maps peer_id to peer and set of txns forwarded to that peer
        std::map<int, std::pair<std::shared_ptr<peer>, std::set<int>>> neighbors;   
        std::map<int, std::set<int>> blks_frwded;                       // blocks forwarded to this neighbor peer
        std::map<int, std::shared_ptr<transaction>> txn_pool;
        std::shared_ptr<std::vector<int>> peer_coins;
        std::unique_ptr<blockchain> blkchain;                           // blockchain corresponding to this peer
        std::shared_ptr<block> mining_blk;                                // current block begin mined

        peer(int coins)
            : coins(coins) {}
        peer(speed label1, cpu_power label2, int coins) 
            : id(++peer::peer_no), label1(label1), label2(), coins(coins) {}

        int get_id() const { return id; }
        int get_coins() const { return coins; }
        speed get_label1() const { return label1; }
        cpu_power get_label2() const { return label2; }

        std::shared_ptr<transaction> get_txn(int txn_id){ 
            if (txn_pool.count(txn_id)) return txn_pool[txn_id];
            else return std::shared_ptr<transaction>();
        }

        void set_label1(speed label1){ this->label1 = label1; }
        void set_label2(cpu_power label2){ this->label2 = label2; }

        void add_coins(int addend){ coins += addend; }
        void add_neighbor(std::shared_ptr<peer> p);
        void add_txn(std::shared_ptr<transaction> txn);
        void add_block(std::shared_ptr<block> blk);

        int gen_txn();
        void gen_blk();                            

        bool verify_txn(const std::shared_ptr<transaction>& txn) const;
        bool verify_block(std::shared_ptr<block> blk) const;
        bool verify_chain(int prev_id) const;               // verify if longest chain still same
        
        void print_txn_pool() const;

        static int peer_no;
};

#endif