#include "Route.h"
#include "Context.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cassert>

using RNG = std::mt19937;

//template: https://cp-algorithms.com/num_methods/simulated_annealing.html

bool probability(double price, double price_next, double t, RNG& rng);

class SA{
   public:
        Route route;

        //para construir a primeira solução
        SA(Context& ctx, int numVehicles);

        //
        SA(Context &ctx, int numVehicles, Route &route);

        SA next();

        double price();

        Route initialSolution();
        
        bool applyRandomMove(Route& route, Context& ctx, int numVehicles, RNG& rng);

    private:
            Context &ctx;
            int numVehicles;

};

 pair <double, SA> simAnneal(Context &ctx, int numVehicles,
                                    double initialTemp, double decayRate, double finalTemp);



