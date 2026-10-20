//https://galgos.inf.puc-rio.br/cvrplib/en/instances

#include "../include/Context.h"
#include "../include/SA.h"
#include "../include/Statistics.h"
#include <filesystem>
#include <cstdio>
#include <cctype>
#include <string>

using std::string;

int extractKFromFilename(string& path) {
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
        printf("Uso: %s <arquivo.vrp> [--statistics|-s]\n", argv[0]);
        printf("  sem flag    -> execucao normal (uma vez)\n");
        printf("  --statistics-> roda 30 vezes e gera arquivo .txt com estatisticas\n");
        printf("Exemplo: %s tests/A-n45-k6.vrp --statistics\n", argv[0]);
        return 1;
    }

    string filename = argv[1];

    bool statisticsMode = false;
    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--statistics" || arg == "-s") statisticsMode = true;
    }

    int numVehicles = extractKFromFilename(filename);

    printf("Arquivo: %s\n", filename.c_str());

    Context ctx;
    ctx.load(filename, numVehicles);
    ctx.distances();
    //ctx.buildNearestClients(15);
    ctx.buildNearestClients(ctx.getDimension()/4);

    printf("capacidade=%d, clientes=%d\n",
           ctx.getVehicle(0).getCapacity(), ctx.getDimension());

    // modo estatisticas
    if (statisticsMode) {
        string baseName = filename;
        size_t slash = baseName.find_last_of("/\\");
        if (slash != string::npos) baseName = baseName.substr(slash + 1);
        size_t dot = baseName.find_last_of('.');
        if (dot != string::npos) baseName = baseName.substr(0, dot);

        string outFile = "results/" + baseName + "_results.txt";

        Statistics stats(ctx, numVehicles,
                         30,       // execucoes
                         1000.0,   // temperatura inicial
                         0.995,    // taxa de decaimento
                         1.0);     // temperatura final
        stats.run(outFile);
        return 0;
    }

    // modo normal
    SA initial(ctx, numVehicles);
    double custoInicial = initial.getRoute().getTotalCost();
    printf("=== Solucao inicial ===\n");
    printf("Custo inicial: %.2f\n", custoInicial);
    initial.getRoute().printRoute();


    pair<double, SA> result = simAnneal(ctx, numVehicles, 100000.0, 0.999, 1.0, 12000); //1000 e 0.995
    double custoFinal = result.first;
    printf("\n=== Solucao final (SA) ===\n");
    printf("Custo SA: %.2f\n", custoFinal);
    result.second.getRoute().printRoute();

    printf("\nCusto Inicial -> Custo Final: %.2f, melhoria de (%.2f%%)\n",
           custoInicial - custoFinal,
           100.0 * (custoInicial - custoFinal) / custoInicial);

    return 0;
}