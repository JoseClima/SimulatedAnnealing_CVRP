#include "../include/Route.h"

void Route::addClient(int vehicle, int client, int position, Context& ctx){
    if(position == -1 || position == (int)routes[vehicle].size()){
        routes[vehicle].push_back(client);
    } else {
        routes[vehicle].insert(routes[vehicle].begin() + position, client);
    }

    usedCapacity[vehicle] += ctx.getClient(client).getDemand();
    RouteCost(vehicle, ctx);
}

void Route::removeClient(int vehicle, int position, Context& ctx){
    int clientToBeRemoved = routes[vehicle][position];
    routes[vehicle].erase(routes[vehicle].begin() + position);

    usedCapacity[vehicle] -= ctx.getClient(clientToBeRemoved).getDemand();
    RouteCost(vehicle, ctx);
}

bool Route::moveClient(int fromVehicle, int fromPosition,
                       int toVehicle,   int toPosition,  Context& ctx){
    int client = routes[fromVehicle][fromPosition];
    int demand = ctx.getClient(client).getDemand();

    if (fromVehicle != toVehicle) {
        if (usedCapacity[toVehicle] + demand > ctx.getVehicle(toVehicle).getCapacity())
            return false;
    }

    if (fromVehicle == toVehicle && toPosition > fromPosition) {
        toPosition--;
    }

    removeClient(fromVehicle, fromPosition, ctx);
    addClient(toVehicle, client, toPosition, ctx);
    return true;
}

void Route::RouteCost(int vehicle, Context& ctx){
    double cost = 0.0;
    vector<int>& r = routes[vehicle];

    if(r.empty()){ costPerVehicles[vehicle] = 0.0; return; }

    cost += ctx.getDistance(0, r[0]);
    for(size_t i = 1; i < r.size(); i++)
        cost += ctx.getDistance(r[i-1], r[i]);
    cost += ctx.getDistance(r.back(), 0);

    costPerVehicles[vehicle] = cost;
}

void Route::updateAllCosts(Context& ctx){
    for(size_t v = 0; v < routes.size(); v++) RouteCost(v, ctx);
}

double Route::getTotalCost() {
    double total = 0.0;
    for(double c : costPerVehicles) {
        total += c;
    }
    return total;
}

void Route::printRoute() {
    for (size_t v = 0; v < routes.size(); ++v) {
        printf("Route #%zu: ", v + 1);
        for (int clientIdx : routes[v]) {
            printf("%d ", clientIdx + 1);
        }
        printf("\n");
    }
    printf("Cost %.0f\n", getTotalCost());
}