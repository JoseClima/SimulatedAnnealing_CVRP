#include "Route.h"
#include "Context.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cassert>

using RNG = std::mt19937;
using std::exp;
using b_distr = std::bernoulli_distribution;

//template: https://cp-algorithms.com/num_methods/simulated_annealing.html

bool probability(double currentCost, double nextCost, double temperature, RNG& rng);

class SA{
   public:
        Route route;

        //para construir a primeira solução
        SA(Context& ctx, int numVehicles);

        //para construir as proximas soluções
        SA(Context &ctx, int numVehicles, Route &route);

        SA next();

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



