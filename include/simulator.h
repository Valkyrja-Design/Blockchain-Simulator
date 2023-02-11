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

        double curr_time;

        std::vector<std::shared_ptr<peer>> peers;       // peers in the network
        std::vector<std::vector<int>> adj;              // adjacency list of network graph
        event_queue e_queue;

        simulator(int n, double z0, double z1, double mean_exp);
        void initialize();
        bool net_is_connected();
        void start();
};

#endif