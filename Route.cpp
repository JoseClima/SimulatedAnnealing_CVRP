#include "Route.h"

//adiciona o cliente em uma posição especifica, caso seja informado o -1 o cliente é add no final
void Route::addClient(int vehicle, int client, int position, Context& ctx){
    if(position == -1 || position == (int)routes[vehicle].size()){
        routes[vehicle].push_back(client);
    }
    else{
        routes[vehicle].insert(routes[vehicle].begin() + position, client); //adiciona na posicao informada
    }

    ctx.vehicles[vehicle].usedCapacity += ctx.clients[client].demand; //atualiza a capacity usada

    RouteCost(vehicle, ctx); //recalcula o preço daquele veiculo

}

void Route::removeClient(int vehicle, int position, Context& ctx){
    
    int client = routes[vehicle][position]; //descobre o cliente
    routes[vehicle].erase(routes[vehicle].begin() + position);

    ctx.vehicles[vehicle].usedCapacity -= ctx.clients[client].demand; //subtrai a capacidade antes usada pelo cliente agora removido 

    RouteCost(vehicle, ctx);

}


bool Route::moveClient(int fromVehicle, int fromPosition, int toVehicle, int toPosition, Context& ctx){
    int client = routes[fromVehicle][fromPosition];
    int demand = ctx.clients[client].demand;

    //verifica se cabe o novo veiculo
    if (fromVeh != toVeh) {
        if (ctx.vehicles[toVeh].usedCapacity + demand > ctx.vehicles[toVeh].capacity)
            return false;
    }

    //se ele for mudar de cliente em um mesmo veiculo a posição final precisa ser subtraida caso ela tenha um valor maior que o fromposition
    if (fromVehicle == toVehicle && toPosition > fromPosition) {
        toPosition--;  
    }

    removeClient(fromVehicle, fromPosition, ctx);
    addClient(toVehicle, client, toPosition, ctx);

    return true;

}


//custo da rota de um veiculo
void Route::RouteCost(int vehicle, Context& ctx){
    double cost = 0.0;
    vector<int>& r = routes[vehicle];

    if(r.empty()){
        costPerVehicles[vehicle] = 0.0;
        return;
    }

    //deposito(0) por causa do primeiro cliente
    cost += ctx.matrixDistances[0][r[0]];

    //proximos clientes
    for(size_t i = 1; i < r.size(); i++){
        cost += ctx.matrixDistances[r[i-1]][r[i]];
    }

    //ultimo cliente
    cost += ctx.matrixDistances[r.back()][0];

    costPerVehicles[vehicle] = cost;

} 

//custo de todos os veiculos
void Route::updateAllCosts(Context& ctx){
    for(size_t v = 0; v < routes.size(); v++){
        RouteCost(v, ctx);
    }
}

//valor total
double Route::getTotalCost(){
    double total = 0.0;
    for(double c : costPerVehicles){
        total +=c;
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
