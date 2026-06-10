#include <vector>
#include <cstdio>
#include "Context.h"
using std::vector;

class Route{
    public:
        vector<vector<int>> routes;
        vector <double> costPerVehicles;

        Route(int numVehicles)
            : routes(numVehicles), costPerVehicles(numVehicles, 0.0){}
        void addClient(int vehicle, int client, int position, Context& ctx);
        void removeClient(int vehicle, int position, Context& ctx);
        bool moveClient(int fromVehicle, int fromPosition, int toVehicle, int toPosition, Context& ctx);
        void RouteCost(int vehicle, Context& ctx);
        void updateAllCosts(Context &ctx);
        double getTotalCost();
        void printRoute();
};