#ifndef _PEER_H
#define _PEER_H

#include <vector>
#include <memory>
#include <map>
#include <set>
#include <random>

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

        int blks_generated = 0;     // blocks generated from the start of simulation

        // maps peer_id to peer and set of txns forwarded to that peer
        std::map<int, std::pair<std::shared_ptr<peer>, std::set<int>>> neighbors;   
        std::map<int, std::set<int>> blks_frwded;                       // blocks forwarded to this neighbor peer
        std::map<int, std::shared_ptr<transaction>> txn_pool;
        std::unique_ptr<std::vector<long long>> peer_coins;             // coins of other peers
        std::unique_ptr<blockchain> blkchain;                           // blockchain corresponding to this peer
        std::shared_ptr<block> mining_blk;                                // current block begin mined
        std::exponential_distribution<double> mining_time;              // distribution for sampling mining_time
        std::exponential_distribution<double> txn_iat;                  // distribution for sampling txn interarrival time

        peer(int coins)
            : coins(coins) {}
        peer(speed label1, cpu_power label2, int coins) 
            : id(++peer::peer_no), label1(label1), label2(label2), coins(coins) {}

        int get_id() const { return id; }
        int get_coins() const { return coins; }
        int get_label1() const { return label1; }
        int get_label2() const { return label2; }

        std::shared_ptr<transaction> get_txn(int txn_id){ 
            if (txn_pool.count(txn_id)) return txn_pool[txn_id];
            else return std::shared_ptr<transaction>();
        }

        void set_label1(speed label1){ this->label1 = label1; }
        void set_label2(cpu_power label2){ this->label2 = label2; }
        void increment_blks(){ blks_generated++; }

        void add_coins(int addend){ coins += addend; }
        void add_neighbor(std::shared_ptr<peer> p);
        void add_txn(std::shared_ptr<transaction> txn);
        std::vector<std::shared_ptr<block>> add_block(std::shared_ptr<block> blk, double arrival);

        int gen_txn();
        void gen_blk();                            

        bool verify_txn(const std::shared_ptr<transaction>& txn) const;
        bool verify_block(std::shared_ptr<block> blk) const;
        bool verify_chain(int prev_id) const;               // verify if longest chain still same
        
        void print_txn_pool() const;

        static int peer_no;
};

#endif