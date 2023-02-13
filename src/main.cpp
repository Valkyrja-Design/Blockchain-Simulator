#include "simulator.h"

int main(){
    int n=10;
    simulator s(n, 0.35, 0.42, 20); //20 sec avg time a peer generate new transaction
    if(n<8){
        std::cout<<"n should be greater then 8"<<std::endl;
        return 0;
    }
    s.simulation_time = 1200; //runs for 1hr
    s.blk_inter_arrival_time = 100;  //10 min avg time (whole network) generate new block 
    s.start();
}