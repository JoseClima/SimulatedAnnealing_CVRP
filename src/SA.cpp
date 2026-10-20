#include "../include/SA.h"


bool probability(double delta, double T, RNG& rng) {
    double prob = exp(-delta / T);
    b_distr d(prob);
    return d(rng);
}

SA::SA(Context &ctx, int numVehicles)
    : ctx(ctx), numVehicles(numVehicles), route(numVehicles) {
    route = initialSolution();
}

SA::SA(Context& ctx, int numVehicles, Route& existingRoute)
    : ctx(ctx), numVehicles(numVehicles), route(existingRoute) {}

Route SA::initialSolution(){
    Route newRoute(numVehicles);
    vector<bool> visited(ctx.getDimension(), false);
    visited[0] = true;

    int current, used;

    for (int v = 0; v < numVehicles; v++){
        current = 0;
        used = 0;

        while (true){
            int nextClient = -1;

            for(int neighbor : ctx.getClient(current).getNearest()){
                if(!visited[neighbor] &&
                   used + ctx.getClient(neighbor).getDemand() <= ctx.getVehicle(v).getCapacity()){
                    nextClient = neighbor;
                    break;
                }
            }

            if(nextClient == -1){
                double bestDist = numeric_limits<double>::max();
                for (int c = 1; c < ctx.getDimension(); c++){
                    if(!visited[c] &&
                       used + ctx.getClient(c).getDemand() <= ctx.getVehicle(v).getCapacity()){
                        if(ctx.getDistance(current, c) < bestDist){
                            nextClient = c;
                            bestDist = ctx.getDistance(current, c);
                        }
                    }
                }
            }
            if(nextClient == -1) break;

            newRoute.addClient(v, nextClient, -1, ctx);
            visited[nextClient] = true;
            used   += ctx.getClient(nextClient).getDemand();
            current = nextClient;
        }
    }

    newRoute.updateAllCosts(ctx);
    return newRoute;
}

bool SA::applyRandomMove(Route& route, Context& ctx, int numVehicles, RNG& rng){
    b_distr optChoice(0.5);
    bool OneOpt = optChoice(rng);

    uni_int_dist vehJ(0, numVehicles - 1);
    int antiloop = 0;

    int v1 = vehJ(rng);
    int v2 = vehJ(rng);

    do{
        v1 = (v1 + 1) % numVehicles;
        antiloop++;
    } while(antiloop < numVehicles && route.routeEmpty(v1));

    antiloop = 0;

    do{
        v2 = (v2 + 1) % numVehicles;
        antiloop++;
    } while(antiloop < numVehicles && (v1 == v2 || route.routeEmpty(v2)));

    if(v2 == v1) return false;

    if(OneOpt){ // 1-opt: move cliente de v1 para v2
        uni_int_dist pos1(0, (int)route.routeSize(v1) - 1);
        int fromPos = pos1(rng);

        uni_int_dist pos2(0, (int)route.routeSize(v2));
        int toPos = pos2(rng);

        return route.moveClient(v1, fromPos, v2, toPos, ctx);
    }
    else { // 2-opt: troca clientes entre v1 e v2
        uni_int_dist pos1(0, (int)route.routeSize(v1) - 1);
        uni_int_dist pos2(0, (int)route.routeSize(v2) - 1);

        int p1 = pos1(rng);
        int c1 = route.getClientAt(v1, p1);
        int d1 = ctx.getClient(c1).getDemand();

        int p2 = pos2(rng);
        int c2 = route.getClientAt(v2, p2);
        int d2 = ctx.getClient(c2).getDemand();

        if(route.getUsedCapacity(v1) - d1 + d2 > ctx.getVehicle(v1).getCapacity() ||
           route.getUsedCapacity(v2) - d2 + d1 > ctx.getVehicle(v2).getCapacity()){
            return false;
        }

        route.removeClient(v1, p1, ctx);
        route.removeClient(v2, p2, ctx);
        route.addClient(v1, c2, p1, ctx);
        route.addClient(v2, c1, p2, ctx);

        return true;
    }
}

pair<double, SA> simAnneal(Context &ctx, int numVehicles,
                           double initialTemp, double decayRate, double finalTemp,
                           int maxTempTrails){

    int r = 0, n, attempts;
    double T = initialTemp, deltaC, currentCost, bestCost;
    const int SoluPerTemp = 100;
    const int maxAttemps = SoluPerTemp * 2;
    bool result;
    RNG rng(chrono::now().time_since_epoch().count());

    SA instCurrent(ctx, numVehicles);
    Route instBest(numVehicles), instCandidate(numVehicles);
    instBest  = instCurrent.getRoute();
    bestCost  = instBest.getTotalCost();

    do{
        n = 0;
        attempts = 0;
        do{
            instCandidate = instCurrent.getRoute();
            result = instCurrent.applyRandomMove(instCandidate, ctx, numVehicles, rng);

            if(result){
                currentCost = instCurrent.getRoute().getTotalCost();
                deltaC = instCandidate.getTotalCost() - currentCost;

                if(deltaC < 0){
                    instCurrent.setRoute(instCandidate);
                    if(instCandidate.getTotalCost() < bestCost){
                        instBest = instCandidate;
                        bestCost = instBest.getTotalCost();
                    }
                }
                else if(probability(deltaC, T, rng)){
                    instCurrent.setRoute(instCandidate);
                }
                n++;
            }
            else {
                attempts++;
            }
        } while(n < SoluPerTemp && attempts < maxAttemps);

        r++;
        T = decayRate * T;

    } while(r < maxTempTrails && T > finalTemp);

    return {bestCost, SA(ctx, numVehicles, instBest)};
}