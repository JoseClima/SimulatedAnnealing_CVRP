//https://galgos.inf.puc-rio.br/cvrplib/en/instances

#include "../include/Context.h"
#include "../include/SA.h"
#include <cstdio>

int main() {
        Context ctx;
        ctx.load("tests/A-n32-k5.vrp", 5);
        ctx.distances();
        ctx.buildNearestClients(10);

        // ---- Teste na main
        printf("capacidade=%d, clientes=%d\n",
        ctx.vehicles[0].capacity, ctx.dimension);

        // ---- Solução inicial
        SA initial(ctx, 5);
        double custoInicial = initial.route.getTotalCost();
        printf("=== Solucao inicial ===\n");
        printf("Custo inicial: %.2f\n", custoInicial);
        initial.route.printRoute();

        Route r = initial.route;
        RNG rng(42);
        bool ok = initial.applyRandomMove(r, ctx, 5, rng);
        printf("primeiro movimento: %s, custo depois: %.2f\n",
       ok ? "true" : "false", r.getTotalCost());

        // ---- SA
        pair<double, SA> result = simAnneal(ctx, 5, 1000.0, 0.995, 1.0);
        double custoFinal = result.first;
        printf("\n=== Solucao final (SA) ===\n");
        printf("Custo SA: %.2f\n", custoFinal);
        result.second.route.printRoute();

        // ---- Comparação
        printf("\nMelhoria: %.2f (%.2f%%)\n",
            custoInicial - custoFinal,
            100.0 * (custoInicial - custoFinal) / custoInicial);

        return 0;
}