#include "simulator.h"

int main(){
    simulator s(20, 0.35, 0.42, 100);
    s.simulation_time = 7200;
    s.blk_inter_arrival_time = 100;  //10 min avg time (whole network) generate new block 
    s.start();
}