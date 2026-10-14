#include "../include/Statistics.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <iomanip>

using namespace std::chrono;

// escreve uma rota em qualquer std::ostream
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

Statistics::Statistics(Context& ctx, int numVehicles, int numExecutions,
                       double initialTemp, double decayRate, double finalTemp)
    : ctx(ctx), numVehicles(numVehicles), numExecutions(numExecutions),
      initialTemp(initialTemp), decayRate(decayRate), finalTemp(finalTemp),
      results(),
      bestRoute(numVehicles),
      bestCost(numeric_limits<double>::max()),
      initialCost(0.0) {
    results.reserve(numExecutions);
}

void Statistics::run(string& outputFile) {
    results.clear();
    bestCost = numeric_limits<double>::max();

    SA initial(ctx, numVehicles);
    initialCost = initial.getRoute().getTotalCost();

    printf("=== Execucao ===\n");
    printf("Custo da solucao inicial: %.4f\n", initialCost);
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

        //printf("  Execucao %2d/%d: custo=%.2f, tempo=%.3fs\n", i + 1, numExecutions, res.first, elapsed);
    }

    saveStatistics(outputFile);

    double sumCost = 0.0;
    for (int i = 0; i < (int)results.size(); ++i) {
        sumCost += results[i].cost;
    }
    double meanCost = sumCost / results.size();
    int    optimal  = ctx.getOptimalCost();

        printf("\n=== Resumo ===\n");
    if (optimal > 0) {
        printf("Custo otimo              : %d\n", optimal);

        double diffMean = meanCost - optimal;
        double pctMean  = 100.0 * diffMean / optimal;
        printf("Custo medio (SA) - otimo : %.4f  (%.2f%%)\n",
               diffMean, pctMean);

        double diffBest = bestCost - optimal;
        double pctBest  = 100.0 * diffBest / optimal;
        printf("Melhor (SA) - otimo      : %.4f  (%.2f%%)\n",
               diffBest, pctBest);
    } else {
        printf("Custo otimo              : N/A\n");
        printf("Custo medio (SA)         : %.4f\n", meanCost);
        printf("Melhor (SA)              : %.4f\n", bestCost);
    }

    double melhoriaAbs = initialCost - bestCost;
    double melhoriaPct = (initialCost > 0)
                       ? 100.0 * melhoriaAbs / initialCost
                       : 0.0;
    printf("Melhoria (inicial->best SA)   : %.4f  (%.2f%%)\n",
           melhoriaAbs, melhoriaPct);
}

void Statistics::saveStatistics(string& outputFile) {
    double sumTime = 0.0, sumCost = 0.0;
    double worstCost = numeric_limits<double>::lowest();
    int worstIdx = 0;
    int bestIdx  = 0;

    bestCost = numeric_limits<double>::max();

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

    out << "Numero de execucoes : " << numExecutions             << "\n";
    out << "Numero de veiculos  : " << numVehicles               << "\n";
    out << "Clientes (excl. dep): " << ctx.getQtdClients()       << "\n";
    out << "Vizinhos (nearest)  : " << ctx.getNearestNeighbors() << " (qtdClient/4)\n";
    out << "Custo otimo         : " << ctx.getOptimalCost()      << "\n\n";

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

        int optimal = ctx.getOptimalCost();

    out << "--- Comparacoes ---\n";
    out << "Custo solucao inicial    : " << initialCost << "\n";
    out << "Melhor custo (SA)        : " << bestCost    << "\n";
    out << "Custo medio (SA)         : " << meanCost    << "\n";

    if (optimal > 0) {
        double diffBest = bestCost - optimal;
        double pctBest  = 100.0 * diffBest / optimal;
        out << "Melhor (SA) - otimo      : " << diffBest
            << "  (" << pctBest << "% pior que o otimo)\n";

        double diffMean = meanCost - optimal;
        double pctMean  = 100.0 * diffMean / optimal;
        out << "Media (SA) - otimo       : " << diffMean
            << "  (" << pctMean << "% pior que o otimo)\n";
    } else {
        out << "Melhor (SA) - otimo      : N/A (custo otimo nao disponivel)\n";
        out << "Media (SA) - otimo       : N/A (custo otimo nao disponivel)\n";
    }

    double melhoriaAbs = initialCost - bestCost;
    double melhoriaPct = (initialCost > 0)
                       ? 100.0 * melhoriaAbs / initialCost
                       : 0.0;
    out << "Melhoria (inicial->best SA)   : " << melhoriaAbs
        << "  (" << melhoriaPct << "%)\n\n";
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