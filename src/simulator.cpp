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

    for (int i=0;i<n;i++){
        int coins = randint(10, 100);     // random coins between 10-100
        peers.emplace_back(std::make_shared<peer>(
            speed::FAST                   // initially set everyone to "fast" and "high cpu"
            , cpu_power::HIGH
            , coins));
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
            auto shuffled = genshuf(req, not_connected[i].size());  // shuffled indices
            std::vector<int> neighbors(req);

            for (int j=0;j<req;j++){
                int v = *std::next(not_connected[i].begin(), shuffled[j]);
                neighbors[j] = v;
                adj[i].push_back(v);
                adj[v].push_back(i);
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
        for (int j : adj[i]){
            peers[i]->add_neighbor(peers[j]);
            std::cout<<j<<", ";
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
        event e{event_type::TXN_GEN, 0.0};
        e.peer_id = i;
        e_queue.push(e);
    }

    while (curr_time <= simulation_time){
        auto e = e_queue.pop();
        curr_time = e.time_stamp;
        std::cout<<"CURRENT TIME: "<<curr_time<<"\n";
        std::cout<<"Next Event: \n";

        if (e.type == event_type::TXN_GEN){
            std::cout<<"TXN_GEN: at "<<e.peer_id+1<<", Txn";

            int txn_id = peers[e.peer_id]->gen_txn();
            std::cout<<txn_id<<"\n";

            // GEN triggers another GEN after interarrival time
            event next_event{event_type::TXN_GEN, curr_time+randexp(Ttx)};
            next_event.peer_id = e.peer_id;
            e_queue.push(next_event);

            // forward to neighbors
            for (auto u : adj[e.peer_id]){
                event next_event{event_type::TXN_GET, curr_time+randexp(Ttx)};  // temporary, need to simulate latency
                next_event.from_peer = e.peer_id;
                next_event.peer_id = u;
                next_event.txn_id = txn_id;

                e_queue.push(next_event);
            }         
        } else if (e.type == event_type::TXN_GET){
            std::cout<<"TXN_GET: at "<<e.peer_id<<", Txn"<<e.txn_id<<" from "<<e.from_peer<<"\n";
            auto txn = peers[e.from_peer]->get_txn(e.txn_id);
            
            // verify txn
            if (peers[txn->id_x]->get_coins() >= txn->C){
                std::cout<<e.peer_id<<" :: Txn valid, forwarding to neighbors\n";

                peers[e.peer_id]->add_txn(txn);

                for (auto u : adj[e.peer_id]){
                    // check if need to forward
                    if (u == e.from_peer || peers[e.peer_id]->neighbors[u+1].second.count(e.txn_id)) continue;
                    peers[e.peer_id]->neighbors[u+1].second.insert(e.txn_id);

                    event next_event{event_type::TXN_GET, curr_time+randexp(Ttx)};  // temporary, need to simulate latency
                    next_event.from_peer = e.peer_id;
                    next_event.peer_id = u;
                    next_event.txn_id = txn->txn_id;

                    e_queue.push(next_event);
                }    
                
            } else{
                std::cout<<e.peer_id<<" :: Invalid Txn, not forwarding\n";
            }
            
        }
    }
}
