#pragma once

#include "Context.h"
#include "Route.h"
#include "SA.h"

struct ExecutionStat {
    double timeSeconds;
    double cost;

    ExecutionStat(double t, double c)
        : timeSeconds(t), cost(c) {}
};

class Statistics {
    private:
        Context& ctx;
        int numVehicles;
        int numExecutions;
        double initialTemp;
        double decayRate;
        double finalTemp;

        vector<ExecutionStat> results;

        Route bestRoute;
        double bestCost;
        double initialCost;

        void saveStatistics(string& outputFile);

    public:
        Statistics(Context& ctx, int numVehicles, int numExecutions = 30,
                   double initialTemp = 1000000, double decayRate = 0.9999, double finalTemp = 1);

        void run(string& outputFile);
};