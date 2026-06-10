#include "Route.h"
#include "Context.h"
#include <random>
#include <chrono>

class SA{
    public:
        SA(Context &ctx, int numVehicles, double initialTemp, double finalTemp, double coolingRate, int iterationsPerTemp, int maxStagnation) :
             numVehicles(numVehicles), initialTemp(initialTemp), finalTemp(finalTemp), 
             coolingRate(coolingRate), iterationsPerTemp(iterationsPerTemp), maxStagnation(maxStagnation) {}

        Route run();

    private:
        Context &ctx;
        int numVehicles;
        double initalTemp;
        double finalTemp;
        double coolingRate;
        int iterPerTemp;
        int maxStagnation;
        
        
}