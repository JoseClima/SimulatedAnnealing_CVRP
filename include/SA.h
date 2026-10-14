#pragma once

#include "Route.h"
#include "Context.h"
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

bool probability(double delta, double T, RNG& rng);

class SA {
    private:
        Context& ctx;
        int numVehicles;
        Route route;

    public:
        SA(Context& ctx, int numVehicles);
        SA(Context& ctx, int numVehicles, Route& existingRoute);

        Route initialSolution();
        bool applyRandomMove(Route& route, Context& ctx, int numVehicles, RNG& rng);

        Route& getRoute() {
            return route;
        }

        void setRoute(Route& r) {
            route = r;
        }
};

pair<double, SA> simAnneal(Context& ctx, int numVehicles,
                           double initialTemp = 1000000,
                           double decayRate = 0.9999,
                           double finalTemp = 1);