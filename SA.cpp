#include <SA.h>

bool probability(double currentCost, double nextCost, double temperature, RNG &rng){
    double prob = exp(-(nextCost-currentCost)/temperature);
    if(prob > 1){
        return true;
    }
    else{
        b_distr d(prob);
        return d(rng);
    }
}

//primeiro construtor
SA::SA(Context &ctx, int numVehicles) : ctx(ctx), numVehicles(numVehicles){
    route = initialSolution();
}

//construtor para as outras vezes
SA::SA(Context &ctx, int numVehicles, Route &route) : ctx(ctx), numVehicles(numVehicles){
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