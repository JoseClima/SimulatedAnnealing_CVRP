//https://galgos.inf.puc-rio.br/cvrplib/en/instances

#include "../include/Context.h"
#include "../include/SA.h"
#include <cstdio>
#include <cctype>


int extractKFromFilename(const std::string& path) {
    size_t pos = path.rfind("-k");

    if (pos == string::npos)
        return 1;

    pos += 2;

    int k = 0;
    bool found = false;

    while (pos < path.size() && std::isdigit((unsigned char)path[pos])) {
        k = k * 10 + (path[pos] - '0');
        found = true;
        ++pos;
    }

    return found ? k : 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.vrp>\n", argv[0]);
        printf("Exemplo: %s tests/A-n45-k6.vrp\n", argv[0]);
        return 1;
    }

    string filename = argv[1];
    int numVehicles = extractKFromFilename(filename);

    printf("Arquivo: %s\n", filename.c_str());
    
    Context ctx;
    ctx.load(filename, numVehicles);
    ctx.distances();
    ctx.buildNearestClients(10);

    printf("capacidade=%d, clientes=%d\n",
           ctx.vehicles[0].capacity, ctx.dimension);

    // ---- Solução inicial
    SA initial(ctx, numVehicles);
    double custoInicial = initial.route.getTotalCost();
    printf("=== Solucao inicial ===\n");
    printf("Custo inicial: %.2f\n", custoInicial);
    initial.route.printRoute();

    Route r = initial.route;
    RNG rng(42);
    bool ok = initial.applyRandomMove(r, ctx, numVehicles, rng);
    printf("\nprimeiro movimento: %s, custo depois: %.2f\n",
           ok ? "true" : "false", r.getTotalCost());

    // ---- SA
    pair<double, SA> result = simAnneal(ctx, numVehicles, 1000.0, 0.995, 1.0);
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