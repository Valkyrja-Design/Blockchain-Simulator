#include "simulator.h"

int main(){
    simulator s(10, 0.35, 0.42, 50);
    s.simulation_time = 3600;
    s.blk_inter_arrival_time = 100;  //10 min avg time (whole network) generate new block 
    s.start();
}