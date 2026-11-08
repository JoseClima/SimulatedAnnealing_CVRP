//https://galgos.inf.puc-rio.br/cvrplib/en/instances

#include "../include/Context.h"
#include "../include/SA.h"
#include "../include/Statistics.h"
#include <filesystem>
#include <cstdio>
#include <cctype>
#include <string>
#include <stdexcept>
#include <cmath>
#include <algorithm>

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
        printf("Uso: %s <arquivo.vrp> [opcoes]\n", argv[0]);
        printf("  --statistics | -s        -> roda 30 vezes e salva .txt\n");
        printf("  --t0 <double>            -> temperatura inicial (padrao 5000)\n");
        printf("  --decay <double>         -> taxa de decaimento  (padrao 0.999)\n");
        printf("  --tf <double>            -> temperatura final   (padrao 0.5)\n");
        printf("  --iter <int>             -> max iteracoes       (padrao 20000)\n");
        printf("  --nearest-fraction <d>   -> fracao da dimension (padrao 0.25)\n");
        printf("  --tag <string>           -> sufixo no nome do arquivo de saida\n");
        return 1;
    }

    string filename = argv[1];

    bool   statisticsMode = false;
    double initialTemp    = 5000.0;
    double decayRate      = 0.999;
    double finalTemp      = 0.5;
    int    maxIter        = 20000;
    double nearestFrac    = 0.25;
    string tag            = "";

    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        auto needValue = [&]() -> bool {
            if (i + 1 >= argc) {
                printf("ERRO: flag '%s' requer valor\n", arg.c_str());
                return false;
            }
            return true;
        };

        if      (arg == "--statistics" || arg == "-s") statisticsMode = true;
        else if (arg == "--t0"    && needValue()) initialTemp = std::stod(argv[++i]);
        else if (arg == "--decay" && needValue()) decayRate   = std::stod(argv[++i]);
        else if (arg == "--tf"    && needValue()) finalTemp   = std::stod(argv[++i]);
        else if (arg == "--iter"  && needValue()) maxIter     = std::stoi(argv[++i]);
        else if (arg == "--nearest-fraction" && needValue())
                                                  nearestFrac = std::stod(argv[++i]);
        else if (arg == "--tag"   && needValue()) tag         = argv[++i];
    }

    int numVehicles = extractKFromFilename(filename);
    printf("Arquivo: %s\n", filename.c_str());

    // BLoco try-catch para evitar que o programa quebre caso a solucao inicial seja impossivel
    try {
        Context ctx;
        ctx.load(filename, numVehicles);
        ctx.distances();

        // fracao * dimension, arredondado, com piso em 1
        int nearestK = std::max(1, (int)std::round(nearestFrac * ctx.getDimension()));
        ctx.buildNearestClients(nearestK);

        printf("capacidade=%d, clientes=%d, nearest=%d (%.2f x dim)\n",
               ctx.getVehicle(0).getCapacity(), ctx.getDimension(),
               nearestK, nearestFrac);

        if (statisticsMode) {
            string baseName = filename;
            size_t slash = baseName.find_last_of("/\\");
            if (slash != string::npos) baseName = baseName.substr(slash + 1);
            size_t dot = baseName.find_last_of('.');
            if (dot != string::npos) baseName = baseName.substr(0, dot);

            string outFile = "results/txt/" + baseName
                           + (tag.empty() ? "" : "_" + tag)
                           + "_results.txt";

            std::filesystem::create_directories("results/txt");

            Statistics stats(ctx, numVehicles,
                             30,
                             initialTemp,
                             decayRate,
                             finalTemp,
                             maxIter);
            stats.run(outFile);
            return 0;
        }

        // modo normal 
        SA initial(ctx, numVehicles);
        printf("Custo inicial: %.2f\n", initial.getRoute().getTotalCost());

        pair<double, SA> result = simAnneal(ctx, numVehicles,
                                            initialTemp, decayRate,
                                            finalTemp, maxIter);
        printf("\nCusto SA: %.2f\n", result.first);
        result.second.getRoute().printRoute();
        
    } 
    catch (const std::exception& e) {
        printf("\n[ERRO FATAL] %s\n", e.what());
        printf("O programa foi encerrado para evitar gerar solucoes invalidas (estouro de capacidade).\n");
        return 1;
    }

    return 0;
}