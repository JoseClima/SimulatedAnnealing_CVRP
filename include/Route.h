#pragma once

#include <vector>
#include <cstdio>
#include "Context.h"
using std::vector;

class Route {
    private:
        vector<vector<int>> routes;
        vector<double> costPerVehicles;
        vector<int> usedCapacity;

    public:
        Route(int numVehicles)
            : routes(numVehicles),
              costPerVehicles(numVehicles, 0.0),
              usedCapacity(numVehicles, 0) {}

        void addClient(int vehicle, int client, int position, Context& ctx);
        void removeClient(int vehicle, int position, Context& ctx);
        bool moveClient(int fromVehicle, int fromPosition, int toVehicle, int toPosition, Context& ctx);
        void RouteCost(int vehicle, Context& ctx);
        void updateAllCosts(Context& ctx);
        double getTotalCost();
        void printRoute();

        int getNumVehicles() {
            return (int)routes.size();
        }

        vector<int> &getRoute(int vehicle) {
            return routes[vehicle];
        }

        int routeSize(int vehicle) {
            return (int)routes[vehicle].size();
        }

        bool routeEmpty(int vehicle) {
            return routes[vehicle].empty();
        }

        int getClientAt(int vehicle, int pos) {
            return routes[vehicle][pos];
        }

        int getUsedCapacity(int vehicle) {
            return usedCapacity[vehicle];
        }
};