#include "../include/SA.h"

//talvez deixar o initialSolution e o ApplyRandomMove como métodos do Route


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
bool probabilityV2(double delta, double T, RNG& rng) {
        double prob = exp(-delta / T); 
        b_distr d(prob);
        return d(rng);
}


SA::SA(Context &ctx, int numVehicles) : ctx(ctx), numVehicles(numVehicles), route(numVehicles){
    route = initialSolution();
}


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
    int antiloop = 0; 

    int v1 = vehJ(rng);
    int v2 = vehJ(rng);

    do{
        v1 = (v1+1)%(numVehicles);
        antiloop++;
    } while(antiloop < numVehicles && (route.routes[v1].empty()));

    antiloop = 0;

    do{
        v2 = (v2+1)%(numVehicles);
        antiloop++;
    } while(antiloop < numVehicles && (v1==v2 || route.routes[v2].empty()));

    if(v2 == v1) return false;

    if(OneOpt){ //1-opt move um cliente de v1 para v2
        uni_int_dist pos1(0, route.routes[v1].size() - 1);
        int fromPos = pos1(rng);

        uni_int_dist pos2(0, route.routes[v2].size());
        int toPos = pos2(rng);

        return route.moveClient(v1, fromPos, v2, toPos, ctx);
    }
    else if(route.routes[v2].empty() == false){ //2-opt troca clientes entre o v1 e v2
        uni_int_dist pos1(0, route.routes[v1].size()-1);
        uni_int_dist pos2(0, route.routes[v2].size()-1);

        int p1 = pos1(rng);
        int c1 = route.routes[v1][p1];
        int d1 = ctx.clients[c1].demand;

        
        int p2 = pos2(rng);
        int c2 = route.routes[v2][p2];
        int d2 = ctx.clients[c2].demand;


        if(route.usedCapacity[v1] - d1 + d2 > ctx.vehicles[v1].capacity
        || route.usedCapacity[v2] - d2 + d1 > ctx.vehicles[v2].capacity){
            return false;
        }

        route.removeClient(v1, p1, ctx);
        route.removeClient(v2, p2, ctx);

        route.addClient(v1, c2, p1, ctx);
        route.addClient(v2, c1, p2, ctx);
        
        return true;
    }
    return false;
}

pair <double, SA> simAnneal(Context &ctx, int numVehicles, double initialTemp, double decayRate, double finalTemp){

    int r = 0, n, attempts; //attemps existe p/ quando resulta em false o applyrandommove
    double T = initialTemp, deltaC, currentCost, bestCost;
    const int MaxTempTrails = 1000;
    const int SoluPerTemp = 100;
    const int maxAttemps = SoluPerTemp*2;
    bool result;
    RNG rng(chrono::now().time_since_epoch().count());

    SA instCurrent(ctx, numVehicles);
    Route instBest(numVehicles), instCandidate(numVehicles);
    instBest = instCurrent.route;
    bestCost = instBest.getTotalCost();

    //testadores
    long long totalTried = 0, totalValid = 0, totalAccepted = 0, totalImprovements = 0;

    do{
        n = 0;
        attempts = 0;
        do{
            instCandidate = instCurrent.route;
            result = instCurrent.applyRandomMove(instCandidate, ctx, numVehicles, rng);

            //primeiro teste
            totalTried++;

            if(result == true){
                deltaC = instCandidate.getTotalCost() - bestCost;
                
                //segundo teste
                totalValid++;

                if(deltaC < 0){

                    //terceiro teste
                    totalAccepted++;

                    instBest = instCandidate;
                    bestCost = instBest.getTotalCost();
                    instCurrent.route = instCandidate;
                }
                else{
                    if(probabilityV2(deltaC, T, rng)){
                        instCurrent.route = instCandidate;
                    }
                }
                n++;
            }
            else{
                attempts++;
            }
        }while(n<SoluPerTemp && attempts < maxAttemps);

        r++;

        if(r < 5){
            printf("r=%d T=%.2f bestCost=%.2f currentCost=%.2f\n",
           r, T, bestCost, instCurrent.route.getTotalCost());
        }

        T = decayRate*T;

    }while(r < MaxTempTrails && T > finalTemp);
    
    printf("tried=%lld valid=%lld accepted=%lld improvements=%lld\n",
       totalTried, totalValid, totalAccepted, totalImprovements);

    return {bestCost, SA(ctx, numVehicles, instBest)};
}