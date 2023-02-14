#include <queue>
#include <iostream>

#include "utilities.h"
#include "simulator.h"

simulator::simulator(int n, double z0, double z1, double mean_exp) : n(n), z0(z0), z1(z1), Ttx(mean_exp) {
    initialize();
}

void simulator::initialize(){
    
    int slow = n*z0;
    int low_cpu = n*z1;

    high_hk=10/(n*z1+n*(1-z1)*10);
    low_hk=1/(n*z1+n*(1-z1)*10);

    std::vector<long long> p_coins;

    auto genesis_blk = std::make_shared<block>(-1);

    for (int i=0;i<n;i++){
        int coins = randint(10, 100);     // random coins between 10-100
        peers.emplace_back(std::make_shared<peer>(
            speed::FAST                   // initially set everyone to "fast" and "high cpu"
            , cpu_power::HIGH
            , coins));

        p_coins.emplace_back(coins);
        peers[i]->blkchain = std::make_unique<blockchain>(genesis_blk);
    }

    peer_coins = std::make_unique<std::vector<long long>>(p_coins);

    for (int i=0;i<n;i++){
        peers[i]->peer_coins = std::make_unique<std::vector<long long>>(p_coins);
    }

    // set random peers to "slow" and "low cpu"
    auto slow_ind = genshuf(slow, n);
    auto low_cpu_ind = genshuf(low_cpu, n);

    for (int ind : slow_ind){
        peers[ind]->set_label1(speed::SLOW);
    }

    for (int ind : low_cpu_ind){
        peers[ind]->set_label2(cpu_power::LOW);
    }

    rho.resize(n);
    link_speeds.resize(n);

    // generate random connected graph
    adj.resize(n);

    std::vector<std::set<int>> not_connected(n);
    std::set<int> s;
    for (int i=0;i<n;i++) s.insert(i);

    while (!net_is_connected()){
        
        for (int i=0;i<n;i++){
            adj[i].clear();
            not_connected[i] = s;
            not_connected[i].erase(i);
        }

        for (int i=0;i<n;i++){
            int degree = adj[i].size();                 // current degree
            int req = randint(4, 8)-degree;             // to get degree in range req

            if (degree >= 4) continue;

            if(req > not_connected[i].size()){          // can't get required #edges, break
                adj[i].clear();                       
                break;
            }

            auto shuffled = genshuf(req, not_connected[i].size());  // shuffled indices
            std::vector<int> neighbors(req);

            for (int j=0;j<req;j++){
                int v = *std::next(not_connected[i].begin(), shuffled[j]);
                neighbors[j] = v;
                adj[i].push_back(v);
                adj[v].push_back(i);

                if(adj[v].size()>=8){
                    //remove v from not_connected of nodes that are still to be checked, so its degree does not increase more
                    for(int k=i+1;k<n;k++){
                        not_connected[k].erase(v);
                    }
                }
            }

            if(adj[i].size()>=8){
                //remove i from not_connected of nodes that are still to be checked, so its degree does not increase more
                for(int j=i+1;j<n;j++){
                    not_connected[j].erase(i);
                }
            }

            for (int j=0;j<req;j++){
                int v = neighbors[j];
                not_connected[v].erase(i);
                not_connected[i].erase(v);
            }
        }
    }

    for (int i=0;i<n;i++){
        std::cout<<"Neighbors of "<<i<<" : ";
        rho[i].assign(n, 0);
        link_speeds[i].resize(n);

        for (int j : adj[i]){
            peers[i]->add_neighbor(peers[j]);
            std::cout<<j<<", ";
            if (rho[j].empty())                            
                rho[i][j] = randreal(0.01, 0.5);              // random real number between 10ms and 500ms

            link_speeds[i][j] = peers[i]->get_label1() == speed::FAST && peers[j]->get_label1() == speed::FAST ? 100 : 5;

        }   
        std::cout<<"\n";
    }

}

bool simulator::net_is_connected(){
    
    // just bfs
    int cnt = 1;
    std::queue<int> q;
    std::vector<bool> visited(n, false);
    visited[0] = true;
    q.push(0);

    while (!q.empty()){
        int u = q.front();
        q.pop();

        for (int v : adj[u]){
            if (!visited[v]){
                visited[v] = true;
                cnt++;
                q.push(v);
            }
        }
    }

    return cnt == n;
}

void simulator::start(){
    curr_time = 0.0;
    for (int i=0;i<n;i++){
        event e1{event_type::TXN_GEN, randexp(1.0/Ttx)};
        e1.peer_id = i;
        e_queue.push(e1);

        double ts = randexp((peers[i]->get_label2() == cpu_power::HIGH ? high_hk : low_hk)/blk_inter_arrival_time);
        // Block mined 
        event e2{event_type::BLK_MINE, ts};
        e2.peer_id = i;
        peers[i]->gen_blk();
        std::cout<<"BLK_MINE_SCHEDULED at "<<ts<<" on Peer "<<i<<"\n";
        e2.blk = peers[i]->mining_blk;
        e_queue.push(e2);

        // Block generate 
        event e3{event_type::BLK_GEN, blk_inter_arrival_time};
        e3.peer_id = i;
        e_queue.push(e3);
    }

    std::cout<<block::block_id<<"\n";

    while (curr_time <= simulation_time){
        std::cout<<std::string(10, '-')<<"\n";
        auto e = e_queue.pop();
        curr_time = e.time_stamp;
        std::cout<<"CURRENT TIME : "<<curr_time<<"\n";
        std::cout<<std::string(10, '-')<<"\n";
        std::cout<<"Next Event is ";

        if (e.type == event_type::TXN_GEN){
            std::cout<<"TXN_GEN at "<<e.peer_id<<", Txn";

            int txn_id = peers[e.peer_id]->gen_txn();

            // std::cout<<txn_id<<"\n";

            // GEN triggers another GEN after interarrival time
            double time_stamp = curr_time+randexp(1.0/Ttx);
            event next_event{event_type::TXN_GEN, time_stamp};
            next_event.peer_id = e.peer_id;
            e_queue.push(next_event);

            if (txn_id == -1) continue;
            
            // forward to neighbors
            for (auto u : adj[e.peer_id]){

                double d_ij = 96*1e-3/link_speeds[e.peer_id][u];
                double latency = rho[e.peer_id][u] + (TXN_SIZE)/(link_speeds[e.peer_id][u]*1000000) + randexp(1/d_ij);

                event next_event{event_type::TXN_GET, curr_time+latency}; 
                next_event.txn = peers[e.peer_id]->get_txn(txn_id);
                next_event.from_peer = e.peer_id;
                next_event.peer_id = u;
                next_event.txn_id = txn_id;

                peers[e.peer_id]->neighbors[u].second.insert(txn_id);

                e_queue.push(next_event);
            }         
        } else if (e.type == event_type::TXN_GET){
            std::cout<<"TXN_GET at "<<e.peer_id<<", Txn"<<e.txn_id<<" from "<<e.from_peer<<"\n";
            auto txn = e.txn;
            
            // verify txn
            if (peers[e.peer_id]->verify_txn(txn)){
                std::cout<<"Peer "<<e.peer_id<<" :: Txn valid, forwarding to peers...\n";

                peers[e.peer_id]->add_txn(txn);

                for (auto u : adj[e.peer_id]){
                    // check if need to forward
                    if (u == e.from_peer || peers[e.peer_id]->neighbors[u].second.count(e.txn_id)) continue;
                    peers[e.peer_id]->neighbors[u].second.insert(e.txn_id);

                    double d_ij = 96*1e-3/link_speeds[e.peer_id][u];
                    double latency = rho[e.peer_id][u] + (8)*1e-3/link_speeds[e.peer_id][u] + randexp(1/d_ij);

                    event next_event{event_type::TXN_GET, curr_time+latency}; 
                    next_event.txn = txn;
                    next_event.from_peer = e.peer_id;
                    next_event.peer_id = u;
                    next_event.txn_id = txn->txn_id;

                    e_queue.push(next_event);
                }    
                
            } else{
                std::cout<<"Peer "<<e.peer_id<<" :: Invalid Txn\n";
            }
        } else if (e.type == event_type::BLK_MINE){
            std::cout<<"BLK_MINE at "<<e.peer_id<<", Blk"<<e.blk->BlkID<<"\n";
            if (!peers[e.peer_id]->verify_chain(e.blk->prev_BlkID)){
                std::cout<<"Peer "<<e.peer_id<<" :: Longest chain not same as before. Aborting event...\n";
            } else{
                std::cout<<"Peer "<<e.peer_id<<" :: Longest chain same. Broadcasting block...\n";
                
                for (int i=0;i<n;i++){
                    std::cout<<"i = "<<i<<" "<<(*peer_coins)[i]<<"\n";
                }
                e.blk->print_blk();

                // add block to blockchain
                peers[e.peer_id]->add_block(e.blk);
                peers[e.peer_id]->blkchain->print_blockchain();

                // broadcast to neighbors
                for (auto u : adj[e.peer_id]){

                    double d_ij = 96*1e-3/link_speeds[e.peer_id][u];
                    double latency = rho[e.peer_id][u] + 
                                    (e.blk->block_size*TXN_SIZE)/(link_speeds[e.peer_id][u]*1000000) + randexp(1/d_ij);

                    event next_event{BLK_GET, curr_time+latency};
                    next_event.blk = e.blk;
                    next_event.peer_id = u;
                    next_event.from_peer = e.peer_id;

                    peers[e.peer_id]->blks_frwded[u].insert(e.blk->BlkID);

                    e_queue.push(next_event);
                }    
            }
        } else if (e.type == event_type::BLK_GET){
            std::cout<<"BLK_GET at "<<e.peer_id<<" , Blk"<<e.blk->BlkID<<" from "<<e.from_peer<<"\n";
            
            for (int i=0;i<n;i++){
                    std::cout<<"i = "<<i<<" "<<(*peer_coins)[i]<<"\n";
                }
                
            e.blk->print_blk();

            if (peers[e.peer_id]->verify_block(e.blk)){
                std::cout<<"Peer "<<e.peer_id<<" :: Block valid, forwarding to peers...\n";

                // add block to blockchain
                peers[e.peer_id]->add_block(e.blk);
                peers[e.peer_id]->blkchain->print_blockchain();

                for (auto u : adj[e.peer_id]){
                    // check if need to forward
                    if (u == e.from_peer || peers[e.peer_id]->blks_frwded[u].count(e.blk->BlkID)) continue;
                    peers[e.peer_id]->blks_frwded[u].insert(e.blk->BlkID);

                    double d_ij = 96*1e-3/link_speeds[e.peer_id][u];
                    double latency = rho[e.peer_id][u] + 
                                    (e.blk->block_size*TXN_SIZE)/(link_speeds[e.peer_id][u]*1000000) + randexp(1/d_ij);

                    event next_event{BLK_GET, curr_time+latency};
                    next_event.blk = e.blk;
                    next_event.peer_id = u;
                    next_event.from_peer = e.peer_id;

                    e_queue.push(next_event);
                }    
                
            } else{
                std::cout<<"Peer "<<e.peer_id<<" :: Invalid Block\n";
            }
        } else if (e.type == BLK_GEN){
            std::cout<<"BLK_GEN at "<<e.peer_id<<", Blk";

            // Block mine
            event e1{event_type::BLK_MINE, curr_time+randexp((peers[e.peer_id]->get_label2() == cpu_power::HIGH ? high_hk : low_hk)/blk_inter_arrival_time)};
            e1.peer_id = e.peer_id;
            peers[e.peer_id]->gen_blk();
            e1.blk = peers[e.peer_id]->mining_blk;
            std::cout<<e1.blk->BlkID<<"\n";
            e_queue.push(e1);

            // Block generate 
            event e2{event_type::BLK_GEN, curr_time+blk_inter_arrival_time};
            e2.peer_id = e.peer_id;
            e_queue.push(e2);
        }
    }
}
