#include <queue>
#include <iostream>
#include <fstream>
#include <climits>

#include "utilities.h"
#include "simulator.h"

simulator::simulator(int n, double z0, double z1, double mean_exp) : n(n), z0(z0), z1(z1), Ttx(mean_exp) {
}

void simulator::initialize(){
    
    int slow = n*z0;
    int low_cpu = n*z1;

    high_hk=10/(n*z1+n*(1-z1)*10);
    low_hk=1/(n*z1+n*(1-z1)*10);

    std::vector<long long> p_coins;

    auto genesis_blk = std::make_shared<block>(-1);
    genesis_blk->set_id();

    for (int i=0;i<n;i++){
        int coins = randint(100, 1000);     // random coins between 100-1000
        peers.emplace_back(std::make_shared<peer>(
            speed::FAST                   // initially set everyone to "fast" and "high cpu"
            , cpu_power::HIGH
            , coins));

        p_coins.emplace_back(coins);
        peers[i]->blkchain = std::make_unique<blockchain>(genesis_blk);
        peers[i]->blkchain->owner = i;
    }

    peer_coins = std::make_unique<std::vector<long long>>(p_coins);

    for (int i=0;i<n;i++){
        peers[i]->peer_coins = std::make_unique<std::vector<long long>>(p_coins);
    }

    // set random peers to "slow" and "low cpu"
    auto slow_ind = genshuf(slow, n);
    auto low_cpu_ind = genshuf(low_cpu, n);

    for (int i=0;i<slow;i++){
        peers[slow_ind[i]]->set_label1(speed::SLOW);
    }

    for (int i=0;i<low_cpu;i++){
        peers[low_cpu_ind[i]]->set_label2(cpu_power::LOW);
    }
    
    for (int i=0;i<n;i++){
        double hash_power = (peers[i]->get_label2() == cpu_power::LOW ? low_hk : high_hk);
        peers[i]->mining_time = std::exponential_distribution<double>(hash_power / blk_inter_arrival_time);
        peers[i]->txn_iat = std::exponential_distribution<double>(1.0 / Ttx);
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
        rho[i].assign(n, 0);
        link_speeds[i].resize(n);

        for (int j : adj[i]){
            peers[i]->add_neighbor(peers[j]);
            if (rho[j].empty())                            
                rho[i][j] = randreal(0.01, 0.5);              // random real number between 10ms and 500ms

            link_speeds[i][j] = peers[i]->get_label1() == speed::FAST && peers[j]->get_label1() == speed::FAST ? 100 : 5;

        }   
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
        double rand = peers[i]->txn_iat(generator);
        event e1{event_type::TXN_GEN, rand};
        e1.peer_id = i;
        e_queue.push(e1);

        double ts = peers[i]->mining_time(generator);
        // Block mined 
        event e2{event_type::BLK_MINE, ts};
        e2.peer_id = i;
        peers[i]->gen_blk();
        e2.blk = peers[i]->mining_blk;
        e_queue.push(e2);

    }

    bool process_remain = false;

    while (curr_time <= simulation_time || process_remain){
        if (e_queue.empty()) break;

        auto e = e_queue.pop();
        curr_time = e.time_stamp;

        if (e.type == event_type::TXN_GEN && !process_remain){
            int txn_id = peers[e.peer_id]->gen_txn();

            // GEN triggers another GEN after interarrival time
            double time_stamp = curr_time + peers[e.peer_id]->txn_iat(generator);
            event next_event{event_type::TXN_GEN, time_stamp};
            next_event.peer_id = e.peer_id;
            e_queue.push(next_event);

            if (txn_id == -1) continue;
            
            // forward to neighbors
            for (auto u : adj[e.peer_id]){

                double d_ij = 96*1e-3/(double)link_speeds[e.peer_id][u];
                double latency = rho[e.peer_id][u] + (TXN_SIZE)/((double)link_speeds[e.peer_id][u]*1000000) + randexp(1/d_ij);

                event next_event{event_type::TXN_GET, curr_time+latency}; 
                next_event.txn = peers[e.peer_id]->get_txn(txn_id);
                next_event.from_peer = e.peer_id;
                next_event.peer_id = u;
                next_event.txn_id = txn_id;

                peers[e.peer_id]->neighbors[u].second.insert(txn_id);

                e_queue.push(next_event);
            }         
        } else if (e.type == event_type::TXN_GET){
            auto txn = e.txn;

            if (peers[e.peer_id]->txn_pool.count(txn->txn_id)){
                continue;
            }
            
            // verify txn
            if (peers[e.peer_id]->verify_txn(txn)){

                peers[e.peer_id]->add_txn(txn);

                for (auto u : adj[e.peer_id]){
                    // check if need to forward
                    if (u == e.from_peer || peers[e.peer_id]->neighbors[u].second.count(e.txn_id)) continue;
                    peers[e.peer_id]->neighbors[u].second.insert(e.txn_id);

                    double d_ij = 96*1e-3/(double)link_speeds[e.peer_id][u];
                    double latency = rho[e.peer_id][u] + (8)*1e-3/link_speeds[e.peer_id][u] + randexp(1/d_ij);

                    event next_event{event_type::TXN_GET, curr_time+latency}; 
                    next_event.txn = txn;
                    next_event.from_peer = e.peer_id;
                    next_event.peer_id = u;
                    next_event.txn_id = txn->txn_id;

                    e_queue.push(next_event);
                }    
                
            } else{
                // std::cout<<"Peer "<<e.peer_id<<" :: Invalid Txn\n";
            }
        } else if (e.type == event_type::BLK_MINE && !process_remain){

            if (!peers[e.peer_id]->verify_chain(e.blk->prev_BlkID)){
                // std::cout<<"Peer "<<e.peer_id<<" :: Longest chain not same as before. Aborting event...\n";
            } else{

                e.blk->set_id();
                peers[e.peer_id]->increment_blks();

                // add block to blockchain and forward added blocks
                auto blocks = peers[e.peer_id]->add_block(e.blk, curr_time);
                
                // broadcast to neighbors
                for (auto frwd_block : blocks){
                    for (auto u : adj[e.peer_id]){

                        double d_ij = 96*1e-3/(double)link_speeds[e.peer_id][u];
                        double latency = rho[e.peer_id][u] + 
                                        (frwd_block->block_size*TXN_SIZE)/((double)link_speeds[e.peer_id][u]*1000000) + randexp(1/d_ij);

                        event next_event{BLK_GET, curr_time+latency};
                        next_event.blk = frwd_block;
                        next_event.peer_id = u;
                        next_event.source = frwd_block->source;
                        next_event.from_peer = e.peer_id;

                        peers[e.peer_id]->blks_frwded[u].insert(frwd_block->BlkID);

                        e_queue.push(next_event);
                    }  
                }  

                double ts = peers[e.peer_id]->mining_time(generator);
                // Block mined 
                event e2{event_type::BLK_MINE, curr_time + ts};
                e2.peer_id = e.peer_id;
                peers[e.peer_id]->gen_blk();
                e2.blk = peers[e.peer_id]->mining_blk;
                e_queue.push(e2);
            }

        } else if (e.type == event_type::BLK_GET){

            if (peers[e.peer_id]->blkchain->blk_exists(e.blk->BlkID)) continue;

            if (peers[e.peer_id]->verify_block(e.blk)){

                // add block to blockchain
                int prev_id = peers[e.peer_id]->blkchain->get_curr_BlkID();
                auto blocks = peers[e.peer_id]->add_block(e.blk, curr_time);
                int new_id = peers[e.peer_id]->blkchain->get_curr_BlkID();

                for (auto frwd_block : blocks){
                    // std::cout<<"Blk"<<frwd_block->BlkID<<", ";
                    for (auto u : adj[e.peer_id]){
                        // check if need to forward
                        if (u == e.from_peer || frwd_block->source == u || peers[e.peer_id]->blks_frwded[u].count(frwd_block->BlkID)) continue;
                        peers[e.peer_id]->blks_frwded[u].insert(frwd_block->BlkID);

                        double d_ij = 96*1e-3/(double)link_speeds[e.peer_id][u];
                        double latency = rho[e.peer_id][u] + 
                                        (frwd_block->block_size*TXN_SIZE)/((double)link_speeds[e.peer_id][u]*1000000) + randexp(1/d_ij);

                        event next_event{BLK_GET, curr_time+latency};
                        next_event.blk = frwd_block;
                        next_event.peer_id = u;
                        next_event.source = frwd_block->source;
                        next_event.from_peer = e.peer_id;

                        e_queue.push(next_event);
                    }   
                } 

                // start mining again only if new chain is created
                if (prev_id != new_id && !process_remain){
                    double ts = peers[e.peer_id]->mining_time(generator);
                    // Block mined 
                    event e2{event_type::BLK_MINE, curr_time + ts};
                    e2.peer_id = e.peer_id;
                    peers[e.peer_id]->gen_blk();
                    e2.blk = peers[e.peer_id]->mining_blk;
                    e_queue.push(e2);
                }
                
            } else{
                // std::cout<<"Peer "<<e.peer_id<<" :: Invalid Block\n";
                // std::cout<<std::string(30, '-')<<"\n";
                // std::cout<<"Peer "<<e.peer_id<<"\n";
                // peers[e.peer_id]->blkchain->print_blockchain();
            }
        } 

        // process remaining events ignoring GEN events 
        if (curr_time > simulation_time || max_blocks <= block::block_id || max_txns <= transaction::txn_no){
            process_remain = true;
        }
    }

    dump_trace();
    dump_edges();
    dump_stats();
}

void simulator::dump_trace(){
    std::fstream file(trace_file, std::ios::out | std::ios::binary);
    if (file.is_open()){
        for (int i=0;i<n;i++){
            peers[i]->blkchain->print_blockchain(file);
        }
    } else{
        std::cerr<<"Failed to open file!\n";
    }
}

void simulator::dump_edges(){
    std::fstream file(graph_file, std::ios::out | std::ios::binary);
    if (file.is_open()){
        // since blockchain same for everyone at equilibrium
        peers[0]->blkchain->print_edges(file);
    } else{
        std::cerr<<"Failed to open file!\n";
    }
}

void simulator::dump_stats(){
    std::fstream file(stats_file, std::ios::out | std::ios::binary);
    std::fstream data("data", std::ios::out | std::ios::binary);

    if (file.is_open()){
        int slow_low_cnt = 0;
        double slow_low_ratio = 0;
        int slow_high_cnt = 0;
        double slow_high_ratio = 0;
        int fast_low_cnt = 0;
        double fast_low_ratio = 0;
        int fast_high_cnt = 0;
        double fast_high_ratio = 0;

        file <<"Total blocks : "<<peers[0]->blkchain->get_total_blks()<<"\n";
        file <<"Longest chain length : "<<peers[0]->blkchain->len<<"\n";
        file <<"Longest chain length / Total blocks : "
                <<(peers[0]->blkchain->len/(double)peers[0]->blkchain->get_total_blks())<<"\n";

        // branch len stats
        int total_branches = 0;
        int longest_branch = 0;
        double mean = 0;
        int smallest_branch = INT_MAX;

        std::map<int, bool> visited;
        std::map<int, int> d;
        std::queue<int> q;
        int root_id = peers[0]->blkchain->genesis_block->BlkID;
        visited[root_id] = true;
        d[root_id] = 1;
        q.push(root_id);

        while (!q.empty()){
            auto u = q.front();
            q.pop();
            for (const auto& blk : peers[0]->blkchain->children[u]){
                int ID = blk->BlkID;
                if (!visited.count(ID)){
                    visited[ID] = true;
                    d[ID] = 1 + d[u];
                    q.push(ID);
                }
            }
            if (peers[0]->blkchain->children[u].empty()){
                total_branches++;
                longest_branch = std::max(longest_branch, d[u]);
                smallest_branch = std::min(smallest_branch, d[u]);
                mean += d[u];
            }
        }

        mean /= (double)total_branches;

        file << "Total Branches : " << total_branches << "\n";
        file << "Branch lengths : " << "Longest : " << longest_branch 
            << " Smallest : " << smallest_branch
            << " Mean : " << mean << "\n";

        for (int i=0;i<n;i++){
            double ratio = peers[i]->blks_generated == 0 ? 0 : ((double)peers[i]->blkchain->get_owner_blks())/(peers[i]->blks_generated);
            file << "Peer "<<i<<" Fraction of my blocks in Longest Chain "<<ratio<<"\n";
            
            if (visualize)
                data << "Hash_Power "<< i << " " << (peers[i]->get_label2() == cpu_power::LOW ? low_hk : high_hk) << "\n";

            if (peers[i]->get_label1() == speed::SLOW && peers[i]->get_label2() == cpu_power::LOW){
                slow_low_cnt++;
                slow_low_ratio += ratio;
                if (visualize)
                    data << "SL " << i << " "<< ratio << "\n";
            } else if (peers[i]->get_label1() == speed::SLOW && peers[i]->get_label2() == cpu_power::HIGH){
                slow_high_cnt++;
                slow_high_ratio += ratio;
                if (visualize)
                    data << "SH " << i << " " << ratio << "\n";
            } else if (peers[i]->get_label1() == speed::FAST && peers[i]->get_label2() == cpu_power::LOW){
                fast_low_cnt++;
                fast_low_ratio += ratio;
                if (visualize)
                    data << "FL " << i << " " << ratio << "\n";
            } else if (peers[i]->get_label1() == speed::FAST && peers[i]->get_label2() == cpu_power::HIGH){
                fast_high_cnt++;
                fast_high_ratio += ratio;
                if (visualize)
                    data << "FH " << i << " " << ratio << "\n";
            } 
        }
        slow_low_ratio /= slow_low_cnt == 0 ? -1 : slow_low_cnt;
        slow_high_ratio /= slow_high_cnt == 0 ? -1 : slow_high_cnt;
        fast_low_ratio /= fast_low_cnt == 0 ? -1 : fast_low_cnt;
        fast_high_ratio /= fast_high_cnt == 0 ? -1 : fast_high_cnt;

        file << "SLOW And LOW CPU, Mean of fraction of blocks: "<<slow_low_ratio<<"\n";
        file << "SLOW And HIGH CPU, Mean of fraction of blocks: "<<slow_high_ratio<<"\n";
        file << "FAST And LOW CPU, Mean of fraction of blocks: "<<fast_low_ratio<<"\n";
        file << "FAST And HIGH CPU, Mean of fraction of blocks: "<<fast_high_ratio<<"\n";

    } else{
        std::cerr<<"Failed to open file!\n";
    }
}