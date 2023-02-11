#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <random>
#include <vector>
#include <algorithm>

std::default_random_engine generator;

int randint(int l, int r){
    std::uniform_int_distribution<int> distribution(l, r);
    return distribution(generator);
}

double randexp(double lambda){
    std::exponential_distribution<double> distribution(lambda);
    return distribution(generator);
}

// select m random indices of n
std::vector<int> genshuf(int m, int n){    
    int i,j;
    std::vector<int> x(n);

    for (i = 0; i < n; i++)
        x[i] = i;
    for (i = 0; i < m; i++) {
        j = randint(i, n-1);
        int t = x[i]; x[i] = x[j]; x[j] = t;
    }
    std::sort(x.begin(), x.begin()+m);
    
    return x;
}



#endif