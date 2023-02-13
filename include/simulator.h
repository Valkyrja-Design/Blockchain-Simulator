#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include <iostream>

#include "peer.h"
#include "event_queue.h"

class simulator{
    public:
        int n;                                          // number of peers
        double z0;                                         // fraction of peers "slow"                                    
        double z1;                                         // fraction of peers "low cpu"
        double Ttx;                                     // mean of interarrival time of txns
        double simulation_time; 
        double blk_inter_arrival_time;
        double high_hk;                                 // hashing fraction of high cpu node
        double low_hk;                                 // hashing fraction of low cpu node
        int mining_fee;

        double curr_time;

        std::vector<std::shared_ptr<peer>> peers;       // peers in the network
        std::vector<std::vector<int>> adj;              // adjacency list of network graph
        std::vector<std::vector<double>> rho;              // speed of light propagation delays
        std::vector<std::vector<int>> link_speeds;       
        std::shared_ptr<std::vector<int>> peer_coins;

        event_queue e_queue;

        simulator(int n, double z0, double z1, double mean_exp);
        void initialize();
        bool net_is_connected();
        void start();
};

#endif