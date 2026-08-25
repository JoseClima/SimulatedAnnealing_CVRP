#include "SA.h"

//feito com base no template
bool probabilityV1(double currentCost, double nextCost, double temperature, RNG &rng){
    double prob = exp(-(nextCost-currentCost)/temperature);
    if(prob > 1){
        return true;
    }
    else{
        b_distr d(prob);
        return d(rng);
    }
}

//feito com base no pseudocodigo do artigo
bool probabilityV2(double delta, double T, mt19937& rng) {
    if (delta >= 0){
        double prob = exp(-delta / T); 
        bernoulli_distribution d(prob);
        return d(rng);
    }
    else{ //delta sendo melhor que a global
        return true;
    }
}

//primeiro construtor
SA::SA(Context &ctx, int numVehicles) : ctx(ctx), numVehicles(numVehicles), route(numVehicles){
    route = initialSolution();
}

//construtor para as outras vezes
SA::SA(Context &ctx, int numVehicles, Route &route) : ctx(ctx), numVehicles(numVehicles), route(numVehicles){
    this->route = route;
}

//primeira solução que pega os clientes mais proximos dos veiculos
Route SA::initialSolution(){
    Route newRoute(numVehicles);
    vector<bool> visited(ctx.dimension, false);
    visited[0] = true;

    int current;
    int used;

    for (int v = 0; v < numVehicles; v++){
        current = 0;
        used = 0;

        while (true){
            int nextClient = -1; //para caso ele nao ache visinhos que nao foram visitados

            //procura um visinho nao visitado
            for(int neighbor : ctx.clients[current].nearest){
                if(visited[neighbor] == false && (used + ctx.clients[neighbor].demand <= ctx.vehicles[v].capacity)){
                    nextClient = neighbor;
                    break;
                }
            }

            //caso nao tenha encontrado nenhum visinho nao visitado
            if(nextClient == -1){
                double bestDist = numeric_limits<double>::max(); //valor maximo de um double
                for (int c = 1; c < ctx.dimension; c++){
                    if(visited[c] == false && (used + ctx.clients[c].demand <= ctx.vehicles[v].capacity)){
                        if(ctx.matrixDistances[current][c] < bestDist){
                            nextClient = c;
                            bestDist = ctx.matrixDistances[current][c];
                        }
                    }
                }
            }
            //nao achou nenhum cliente
            if(nextClient == -1){
                break;
            }

            newRoute.addClient(v, nextClient, -1, ctx);
            visited[nextClient] = true;

            used += ctx.clients[nextClient].demand;
            current = nextClient;
        }

    }
    
    newRoute.updateAllCosts(ctx);
    return newRoute;
}

bool SA::applyRandomMove(Route& route, Context& ctx, int numVehicles, RNG& rng){
    //distribuição bernoulli gera verdadeiro ou falso com base na prob informada, nesse caso metade para cada caso 
    b_distr optChoice(0.5);
    //o bool armazena o resultado da escolha aleatoria do b_distr
    bool OneOpt = optChoice(rng);


    uni_int_dist vehJ(0, numVehicles - 1);
    int v1 = vehJ(rng);
    int v2 = vehJ(rng);

    int antiloop = 0; 
    while ((v2 == v1 || route.routes[v1].empty() || route.routes[v2].empty()) && antiloop < 15){
        v2 = vehJ(rng);
        antiloop++;
    }

    if(OneOpt){ //1-opt move um cliente de v1 para v2
        uni_int_dist pos1(0, route.routes[v1].size() - 1);
        int fromPos = pos1(rng);

        uni_int_dist pos2(0, route.routes[v2].size());
        int toPos = pos2(rng);

        return route.moveClient(v1, fromPos, v2, toPos, ctx);
    }
    else{ //2-opt troca clientes entre o v1 e v2
        uni_int_dist pos1(0, route.routes[v1].size()-1);
        uni_int_dist pos2(0, route.routes[v2].size()-1);

        int p1 = pos1(rng);
        int c1 = route.routes[v1][p1];
        int d1 = ctx.clients[c1].demand;

        
        int p2 = pos2(rng);
        int c2 = route.routes[v2][p2];
        int d2 = ctx.clients[c2].demand;


        if(ctx.vehicles[v1].usedCapacity - d1 + d2 > ctx.vehicles[v1].capacity
        || ctx.vehicles[v2].usedCapacity - d2 + d1 > ctx.vehicles[v2].capacity){
            return false;
        }

        route.removeClient(v1, p1, ctx);
        route.removeClient(v2, p2, ctx);

        route.addClient(v1, c2, p1, ctx);
        route.addClient(v2, c1, p2, ctx);

    }

}