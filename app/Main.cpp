#include "Context.h"
#include "SA.h"
#include <cstdio>
//https://galgos.inf.puc-rio.br/cvrplib/en/instances
int main() {
    Context ctx;
    ctx.load("A-n32-k5.vrp", 5);
    /* 
    for (int i = 0; i < ctx.dimension; ++i) {
        printf("Cliente %d | (%d,%d) | demanda %d\n",
               ctx.clients[i].id, ctx.clients[i].x, ctx.clients[i].y, ctx.clients[i].demand);
    }
    printf("Veículos: %d, cap: %d\n", (int)ctx.vehicles.size(), ctx.vehicles[0].capacity);
    ctx.distances();
    printf("Distancia do deposito para o cliente 1: %.2f\n", ctx.matrixDistances[0][1]);
     */

    ctx.distances();
    ctx.buildNearestClients(10);

    SA sa(ctx, 5);
    sa.route.printRoute();

    printf("Custo total: %.2f", sa.route.getTotalCost());


    return 0;
}