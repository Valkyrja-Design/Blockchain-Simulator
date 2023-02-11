#include "simulator.h"

int main(){
    simulator s(10, 0.35, 0.42, 3.5);
    s.simulation_time = 2.0;
    s.start();
}