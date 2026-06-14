#include <SA.h>

bool probability(double currentCost, double nextCost, double temperature, RNG &rng){
    double prob = exp(-(nextCost-currentCost)/temperature);
    if(prob > 1){
        return true;
    }
    else{
        b_distr d(prob);
        return d(rng);
    }
}

//primeiro construtor
SA::SA(Context &ctx, int numVehicles) : ctx(ctx), numVehicles(numVehicles){
    route = initialSolution();
}

//construtor para as outras vezes
SA::SA(Context &ctx, int numVehicles, Route &route) : ctx(ctx), numVehicles(numVehicles){
    this->route = route;
}

Route SA::initialSolution(){
    Route newRoute(numVehicles);
    vector<bool> visited(ctx.dimension, false);
    visited[0] = true;

    
}