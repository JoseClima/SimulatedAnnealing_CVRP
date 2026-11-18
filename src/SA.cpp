#include "../include/SA.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>


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


namespace {

bool buildNearestNeighbor(Context& ctx, int numVehicles, double fitFactor, Route& out) {
    int n = ctx.getDimension();
    vector<bool> visited(n, false);
    visited[0] = true;
    int remaining = n - 1;

    for (int v = 0; v < numVehicles && remaining > 0; v++) {
        int capacity = ctx.getVehicle(v).getCapacity();
        int current = 0;
        int used = 0;

        while (remaining > 0) {
            int next = -1;

            bool tight = false;
            if (fitFactor > 0.0) {
                int maxDemand = 0;
                for (int c = 1; c < n; c++) {
                    if (!visited[c]) maxDemand = std::max(maxDemand, ctx.getClient(c).getDemand());
                }
                tight = (capacity - used) <= fitFactor * maxDemand;
            }

            if (tight) {
                int bestDemand = -1;
                double bestDist = std::numeric_limits<double>::max();
                for (int c = 1; c < n; c++) {
                    int d = ctx.getClient(c).getDemand();
                    if (visited[c] || used + d > capacity) continue;
                    double dist = ctx.getDistance(current, c);
                    if (d > bestDemand || (d == bestDemand && dist < bestDist)) {
                        next = c;
                        bestDemand = d;
                        bestDist = dist;
                    }
                }
            } 
            else {
                for (int neighbor : ctx.getClient(current).getNearest()) {
                    if (!visited[neighbor] && used + ctx.getClient(neighbor).getDemand() <= capacity) {
                        next = neighbor;
                        break;
                    }
                }
                if (next == -1) {
                    double bestDist = std::numeric_limits<double>::max();
                    for (int c = 1; c < n; c++) {
                        if (!visited[c] &&
                            used + ctx.getClient(c).getDemand() <= capacity &&
                            ctx.getDistance(current, c) < bestDist) {
                            next = c;
                            bestDist = ctx.getDistance(current, c);
                        }
                    }
                }
            }

            if (next == -1) break;   // nada cabe: fecha a rota e passa ao proximo veiculo

            out.addClient(v, next, -1, ctx);
            visited[next] = true;
            used += ctx.getClient(next).getDemand();
            current = next;
            remaining--;
        }
    }
    return remaining == 0;
}

// Best Fit Decreasing: empacota os clientes por demanda decrescente no veiculo
// em que a sobra de capacidade fica menor; depois ordena cada rota por NN.
// Perde parte da coerencia geografica, por isso so e usado como ultimo recurso.
bool buildBestFitDecreasing(Context& ctx, int numVehicles, Route& out) {
    int n = ctx.getDimension();

    std::vector<int> order(n - 1);
    std::iota(order.begin(), order.end(), 1);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return ctx.getClient(a).getDemand() > ctx.getClient(b).getDemand();
    });

    std::vector<std::vector<int>> bins(numVehicles);
    std::vector<int> load(numVehicles, 0);

    for (int c : order) {
        int d = ctx.getClient(c).getDemand();
        int bestVehicle = -1;
        int bestSlack = std::numeric_limits<int>::max();
        for (int v = 0; v < numVehicles; v++) {
            int slack = ctx.getVehicle(v).getCapacity() - load[v] - d;
            if (slack >= 0 && slack < bestSlack) {
                bestSlack = slack;
                bestVehicle = v;
            }
        }
        if (bestVehicle == -1) return false;   // nao coube em nenhum veiculo
        bins[bestVehicle].push_back(c);
        load[bestVehicle] += d;
    }

    for (int v = 0; v < numVehicles; v++) {
        std::vector<int>& bin = bins[v];
        int current = 0;
        while (!bin.empty()) {
            size_t bestIdx = 0;
            for (size_t i = 1; i < bin.size(); i++) {
                if (ctx.getDistance(current, bin[i]) < ctx.getDistance(current, bin[bestIdx]))
                    bestIdx = i;
            }
            int c = bin[bestIdx];
            out.addClient(v, c, -1, ctx);
            bin.erase(bin.begin() + bestIdx);
            current = c;
        }
    }
    return true;
}

} 


Route SA::initialSolution(){
    static double fitFactors[] = {0.0, 2.0};
    for (double f : fitFactors) {
        Route candidate(numVehicles);
        if (buildNearestNeighbor(ctx, numVehicles, f, candidate)) return candidate;
    }

    Route candidate(numVehicles);
    if (buildBestFitDecreasing(ctx, numVehicles, candidate)) return candidate;

    throw std::runtime_error(
        "Instancia sem solucao inicial viavel com " + std::to_string(numVehicles) +
        " veiculos (as heuristicas de empacotamento falharam).");
}

bool SA::applyRandomMove(Route& route, Context& ctx, int numVehicles, RNG& rng){
    std::vector<int> active;
    active.reserve(numVehicles);
    for (int v = 0; v < numVehicles; v++) {
        if (!route.routeEmpty(v)) active.push_back(v);
    }
    if (active.size() < 2) return false;

    uni_int_dist pickFirst(0, (int)active.size() - 1);
    uni_int_dist pickOther(0, (int)active.size() - 2);
    int i1 = pickFirst(rng);
    int i2 = pickOther(rng);
    if (i2 >= i1) {
        i2++;
    }
    int v1 = active[i1];
    int v2 = active[i2];

    b_distr optChoice(0.5);
    bool OneOpt = optChoice(rng);

    if(OneOpt){ // relocate: move cliente de v1 para v2 (moveClient confere a capacidade de v2)
        uni_int_dist pos1(0, (int)route.routeSize(v1) - 1);
        int fromPos = pos1(rng);

        uni_int_dist pos2(0, (int)route.routeSize(v2));
        int toPos = pos2(rng);

        return route.moveClient(v1, fromPos, v2, toPos, ctx);
    }
    else { // swap: troca clientes entre v1 e v2
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

pair<double, SA> simAnneal(Context &ctx, 
                           int numVehicles,
                           double initialTemp, 
                           double decayRate, 
                           double finalTemp, 
                           int maxTempTrails){

    int nivelAtual = 0;
    int movimentosValidos = 0;
    int tentativasInvalidas = 0;

    double T = initialTemp, deltaC, currentCost, bestCost;

    int MAX_MOVIMENTOS_POR_NIVEL = 100; //cadeia de markov
    int MAX_TENTATIVAS_INVALIDAS = MAX_MOVIMENTOS_POR_NIVEL * 2;

    bool result;
    RNG rng(chrono::now().time_since_epoch().count());

    SA instCurrent(ctx, numVehicles);
    Route instBest(numVehicles), instCandidate(numVehicles);
    instBest  = instCurrent.getRoute();
    bestCost  = instBest.getTotalCost();                        

    do{
        movimentosValidos = 0;
        tentativasInvalidas = 0;
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
                movimentosValidos++;
            }
            else {
                tentativasInvalidas++;
            }
        } while(movimentosValidos < MAX_MOVIMENTOS_POR_NIVEL && tentativasInvalidas < MAX_TENTATIVAS_INVALIDAS);
        nivelAtual++;
        T = decayRate * T;
    } while(nivelAtual < maxTempTrails && T > finalTemp);
    return {bestCost, SA(ctx, numVehicles, instBest)};
}