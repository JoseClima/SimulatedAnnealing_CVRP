#include "../include/Statistics.h"

#include <iomanip>
#include <fstream>

using namespace std::chrono;
using std::ostream;

// ------------------------------------------------------------
// helper: escreve uma rota em qualquer std::ostream
// ------------------------------------------------------------
static void writeRouteTo(std::ostream& out, Route& route) {
    for (int v = 0; v < route.getNumVehicles(); ++v) {
        out << "Route #" << (v + 1) << ": ";
        for (int clientIdx : route.getRoute(v)) {
            out << (clientIdx + 1) << " ";
        }
        out << "\n";
    }
    out << "Cost " << route.getTotalCost() << "\n";
}

// ------------------------------------------------------------
Statistics::Statistics(Context& ctx, int numVehicles, int numExecutions,
                       double initialTemp, double decayRate, double finalTemp)
    : ctx(ctx), numVehicles(numVehicles), numExecutions(numExecutions),
      initialTemp(initialTemp), decayRate(decayRate), finalTemp(finalTemp),
      results(),
      bestRoute(numVehicles),
      bestCost(numeric_limits<double>::max()) {
    results.reserve(numExecutions);
}

// ------------------------------------------------------------
void Statistics::run(string& outputFile) {
    results.clear();
    bestCost = numeric_limits<double>::max();

    printf("Executando %d vezes o Simulated Annealing...\n", numExecutions);

    for (int i = 0; i < numExecutions; ++i) {
        auto start = steady_clock::now();

        pair<double, SA> res = simAnneal(ctx, numVehicles,
                                         initialTemp, decayRate, finalTemp);

        auto end = steady_clock::now();
        double elapsed = duration<double>(end - start).count();

        results.emplace_back(elapsed, res.first);

        if (res.first < bestCost) {
            bestCost  = res.first;
            bestRoute = res.second.getRoute();
        }

        printf("  Execucao %2d/%d: custo=%.2f, tempo=%.3fs\n",
               i + 1, numExecutions, res.first, elapsed);
    }

    saveStatistics(outputFile);
    printf("\nResultados salvos em: %s\n", outputFile.c_str());
}

// ------------------------------------------------------------
void Statistics::saveStatistics(string& outputFile) {
    double sumTime = 0.0, sumCost = 0.0;
    double worstCost = -numeric_limits<double>::max();
    bestCost = numeric_limits<double>::max();
    int worstIdx = 0;
    int bestIdx  = 0;

    for (int i = 0; i < (int)results.size(); ++i) {
        sumTime += results[i].timeSeconds;
        sumCost += results[i].cost;

        if (results[i].cost < bestCost) {
            bestCost = results[i].cost;
            bestIdx  = i;
        }
        if (results[i].cost > worstCost) {
            worstCost = results[i].cost;
            worstIdx  = i;
        }
    }

    const double meanTime = sumTime / results.size();
    const double meanCost = sumCost / results.size();

    double varTime = 0.0, varCost = 0.0;
    for (int i = 0; i < (int)results.size(); ++i) {
        varTime += (results[i].timeSeconds - meanTime) * (results[i].timeSeconds - meanTime);
        varCost += (results[i].cost        - meanCost) * (results[i].cost        - meanCost);
    }
    const double stdTime = std::sqrt(varTime / results.size());
    const double stdCost = std::sqrt(varCost / results.size());

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        printf("Erro: nao foi possivel criar o arquivo '%s'.\n", outputFile.c_str());
        return;
    }

    out << std::fixed << std::setprecision(4);

    out << "====================================================\n";
    out << "   ESTATISTICAS DE EXECUCOES - SIMULATED ANNEALING\n";
    out << "====================================================\n\n";

    out << "Numero de execucoes : " << numExecutions       << "\n";
    out << "Numero de veiculos  : " << numVehicles         << "\n";
    out << "Clientes (excl. dep): " << ctx.getQtdClients() << "\n\n";

    out << "--- Parametros do SA ---\n";
    out << "Temperatura inicial : " << initialTemp << "\n";
    out << "Taxa de decaimento  : " << decayRate   << "\n";
    out << "Temperatura final   : " << finalTemp   << "\n\n";

    out << "--- Estatisticas de Tempo ---\n";
    out << "Tempo medio         : " << meanTime << " s\n";
    out << "Desvio padrao       : " << stdTime  << " s\n\n";

    out << "--- Estatisticas de Custo ---\n";
    out << "Custo medio         : " << meanCost << "\n";
    out << "Desvio padrao       : " << stdCost << "\n";
    out << "Melhor custo        : " << bestCost
        << "  (execucao #" << (bestIdx + 1) << ")\n";
    out << "Pior custo          : " << worstCost
        << "  (execucao #" << (worstIdx + 1) << ")\n\n";

    out << "====================================================\n";
    out << "  MELHOR SOLUCAO (execucao #" << (bestIdx + 1) << ")\n";
    out << "====================================================\n";
    out << "Custo: " << bestCost << "\n";
    writeRouteTo(out, bestRoute);

    out << "\n====================================================\n";
    out << "  TODAS AS EXECUCOES\n";
    out << "====================================================\n";
    out << "   #     Custo(s)      Tempo(s)\n";
    for (int i = 0; i < (int)results.size(); ++i) {
        out << std::setw(4)  << (i + 1) << "   "
            << std::setw(10) << results[i].cost << "   "
            << std::setw(10) << results[i].timeSeconds << "\n";
    }

    out.close();
}