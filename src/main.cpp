#include "simulator.h"

#include <limits>

int main(){
    int n = 20;
    int z0 = 0.35;
    int z1 = 0.42;
    int Ttx = 200;
    int max_blocks = 200;
    simulator s(20, 0.35, 0.42, 40);
    s.simulation_time = std::numeric_limits<double>::max();
    s.blk_inter_arrival_time = 200;  //10 min avg time (whole network) generate new block 
    s.max_blocks = max_blocks;
    s.start();
}