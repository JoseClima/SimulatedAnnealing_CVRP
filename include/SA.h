#pragma once

#include "../include/Route.h"
#include "../include/Context.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cassert>

using RNG = std::mt19937;
using chrono = std::chrono::steady_clock;
using uni_int_dist = std::uniform_int_distribution<int>;
using std::exp;
using b_distr = std::bernoulli_distribution;
using std::numeric_limits;

//template: https://cp-algorithms.com/num_methods/simulated_annealing.html

bool probabilityV1(double currentCost, double nextCost, double temperature, RNG& rng);
bool probabilityV2(double delta, double T, RNG& rng);

class SA{
   public:
        Route route;

        //para construir a primeira solução
        SA(Context& ctx, int numVehicles);

        //para construir as proximas soluções
        SA(Context &ctx, int numVehicles, Route &existingRroute);

        Route initialSolution();
        
        bool applyRandomMove(Route& route, Context& ctx, int numVehicles, RNG& rng);


        
    private:
            Context &ctx;
            int numVehicles;

};

//parametros do template
pair <double, SA> simAnneal(Context &ctx, int numVehicles,
                                    double initialTemp = 1000, 
                                    double decayRate = 0.995, 
                                    double finalTemp = 1); 