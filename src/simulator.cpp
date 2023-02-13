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
    block_no=0;
    high_hk=10/(n*z1+n*(1-z1)*10);
    low_hk=1/(n*z1+n*(1-z1)*10);
    
    //set this genesis block in each peer
    for(int i=0;i<n;i++){
        int coins = randint(10, 100);
        peers.emplace_back(std::make_shared<peer>(
            speed::FAST                   // initially set everyone to "fast" and "high cpu"
            , cpu_power::HIGH
            , coins));
    }

    std::shared_ptr<block> genesis_block=std::make_shared<block>(-1, nullptr);//blk_id=-1 and No miner
    for(int i=0;i<n;i++){
        peers[i]->genesis_block=genesis_block;
        peers[i]->cur_block=genesis_block;
    }

    for(int i=0;i<n;i++){
        peers[i]->peer_coins.resize(n);
    }
    
    rho.resize(n);
    std::vector<int> temp(n,0);
    std::fill(rho.begin(), rho.end(), temp);

    for (int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            peers[j]->peer_coins[i]=peers[i]->get_coins();
        }
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

    std::vector<std::set<int>> not_connected(n); // not_connected[i]=set of peers not connected to i
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

            if(req>not_connected[i].size()){
                adj[i].clear();//so net_is_connected return false as i will have no edge connected
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
                    for(int j=i+1;j<n;j++){
                        not_connected[j].erase(v);
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
        for (int j : adj[i]){
            peers[i]->add_neighbor(peers[j]);
            std::cout<<j<<", ";
            if(rho[i][j]==0){
                rho[i][j]=randint(10,500);  
            }
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
        // each peer generate a transaction
        event e{event_type::TXN_GEN, randexp(1/Ttx)};
        e.peer_id = i;
        e_queue.push(e);

        // each peer start to mine blocks
        event blk_gen{event_type::BLK_GEN, randexp((peers[i]->get_label2()==cpu_power::HIGH?high_hk:low_hk)/(blk_inter_arrival_time))};
        blk_gen.peer_id=i;
        block* temp=new block(++block_no, peers[i]);
        temp->add_coinbase_txn(MINING_FEE);
        temp->set_parent(peers[i]->cur_block); //mines on curent block
        //No transaction added as initially they have empty transaction pools
        blk_gen.blk=temp;
        e_queue.push(blk_gen);
    }

    while (curr_time <= simulation_time){
        auto e = e_queue.pop();
        curr_time = e.time_stamp;
        std::cout<<"CURRENT TIME: "<<curr_time<<"\n";
        std::cout<<"Next Event: \n";

        if (e.type == event_type::TXN_GEN){
            std::cout<<"TXN_GEN: at "<<e.peer_id<<", Txn";
            int txn_id = peers[e.peer_id]->gen_txn();
            std::cout<<txn_id<<"\n";

            // GEN triggers another GEN after interarrival time
            event next_event{event_type::TXN_GEN, curr_time+randexp(1/Ttx)};
            next_event.peer_id = e.peer_id;
            e_queue.push(next_event);
            // forward to neighbors
            for (auto u : adj[e.peer_id]){
                // send from e.peer_id to u
                int c;
                if(peers[e.peer_id]->get_label1()==speed::FAST && peers[u]->get_label1()==speed::FAST){
                    c=100;
                }
                else{
                    c=5;
                }
                double t=rho[e.peer_id][u]*1e-3 + 8*1e3/(c*1e6)+randexp(1/(96*1e-3/c)); //rho_ij+|m|/cij+d_ij //update |m|
                event next_event{event_type::TXN_GET, curr_time+randexp(1/t)};  // temporary, need to simulate latency
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
                    int c;
                    if(peers[e.peer_id]->get_label1()==FAST && peers[e.from_peer]->get_label1()==FAST){
                        c=100;
                    }
                    else{
                        c=5;
                    }
                    double t=rho[e.from_peer][e.peer_id]*1e-3 + 8*1e3/(c*1e6)+randexp(1/(96*1e-3/c));  //rho_ij+|m|/cij+d_ij
                    event next_event{event_type::TXN_GET, curr_time+randexp(t)};  // temporary, need to simulate latency
                    next_event.from_peer = e.peer_id;
                    next_event.peer_id = u;
                    next_event.txn_id = txn->txn_id;

                    e_queue.push(next_event);
                }
            } else{
                    std::cout<<e.peer_id<<" :: Invalid Txn, not forwarding\n";
            }
        }    
                
        else if(e.type==event_type::BLK_GEN){
            //Should I broadcast? Verify if prevBlkId present in event block is still the last block of valid chain.
            if(e.blk->prevBlkID==peers[e.peer_id]->cur_block->BlkID){
                std::cout<<"BLK_GEN: at "<<e.peer_id+1<<", Blk";
            }
            else{
                //Blk Rejected and again starts mining new block on last block of blockchain
                event blk_gen{event_type::BLK_GEN, curr_time+randexp((peers[e.peer_id]->get_label2()==cpu_power::HIGH?high_hk:low_hk)/(blk_inter_arrival_time))};
                blk_gen.peer_id=e.peer_id;
                block* temp=new block(++block_no, peers[e.peer_id]);
                temp->add_coinbase_txn(MINING_FEE);
                temp->set_parent(peers[e.peer_id]->cur_block); //mines on curent block
                //Take random number of transactions from transaction pool
                
                blk_gen.blk=temp;
                e_queue.push(blk_gen);
            }
                
        } else if(e.type==event_type::BLK_GET){

        }    
    }
}
