#include "../include/Statistics.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <vector>

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

// Mediana: precisa receber por valor porque vamos ordenar
static double computeMedian(std::vector<double> values) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const size_t n = values.size();
    if (n % 2 == 1) return values[n / 2];
    return (values[n / 2 - 1] + values[n / 2]) / 2.0;
}

Statistics::Statistics(Context& ctx, int numVehicles, 
                        int numExecutions, double initialTemp, 
                        double decayRate, double finalTemp,
                        int maxTempTrails)
    : ctx(ctx), 
      numVehicles(numVehicles), 
      numExecutions(numExecutions),
      initialTemp(initialTemp), 
      decayRate(decayRate), 
      finalTemp(finalTemp),
      maxTempTrails(maxTempTrails),
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

    for (int i = 0; i < numExecutions; ++i) {
        auto start = steady_clock::now();

        pair<double, SA> res = simAnneal(ctx, numVehicles,
                                 initialTemp, decayRate, finalTemp,
                                 maxTempTrails);

        auto end = steady_clock::now();
        double elapsed = duration<double>(end - start).count();

        results.emplace_back(elapsed, res.first);

        if (res.first < bestCost) {
            bestCost  = res.first;
            bestRoute = res.second.getRoute();
        }
    }

    saveStatistics(outputFile);
}

void Statistics::saveStatistics(string& outputFile) {
    // guarda contra execucao sem resultados (evita divisao por zero)
    if (results.empty()) {
        printf("Erro: nenhuma execucao registrada em saveStatistics().\n");
        return;
    }

    double sumTime = 0.0, sumCost = 0.0;
    double worstCost = numeric_limits<double>::lowest();
    int worstIdx = 0;
    int bestIdx  = 0;

    bestCost = numeric_limits<double>::max();

    std::vector<double> costs;
    costs.reserve(results.size());

    for (int i = 0; i < (int)results.size(); ++i) {
        sumTime += results[i].timeSeconds;
        sumCost += results[i].cost;
        costs.push_back(results[i].cost);

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

    // ---- NOVAS METRICAS ----
    const double medianCost = computeMedian(costs);

    // Coeficiente de variacao = desvio padrao / media (em %)
    const double cvCost = (meanCost > 0.0)
                        ? (stdCost / meanCost) * 100.0
                        : 0.0;

    // Taxa de sucesso: % de execucoes com custo <= 110% do otimo
    const int optimal = ctx.getOptimalCost();
    int successCount = 0;
    double successThreshold = 0.0;
    double successRate = 0.0;
    if (optimal > 0) {
        successThreshold = optimal * 1.10;
        for (int i = 0; i < (int)results.size(); ++i) {
            if (results[i].cost <= successThreshold) successCount++;
        }
        successRate = 100.0 * successCount / results.size();
    }
    // -------------------------

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

    out << "\n--- Parametros do SA ---\n";
    out << "Vizinhos (nearest)  : " << ctx.getNearestNeighbors() << "\n";
    out << "Temperatura inicial : " << initialTemp << "\n";
    out << "Taxa de decaimento  : " << decayRate   << "\n";
    out << "Temperatura final   : " << finalTemp   << "\n";
    out << "Max iteracoes       : " << maxTempTrails << "\n\n";

    out << "--- Estatisticas de Tempo ---\n";
    out << "Tempo medio         : " << meanTime << " s\n";
    out << "Desvio padrao       : " << stdTime  << " s\n\n";

    out << "--- Estatisticas de Custo ---\n";
    out << "Custo otimo         : " << ctx.getOptimalCost()      << "\n";
    out << "Custo solu inicial  : " << initialCost << "\n";
    out << "Custo medio         : " << meanCost << "\n";
    out << "Mediana             : " << medianCost << "\n";
    out << "Desvio padrao       : " << stdCost << "\n";
    out << "Coef. de variacao   : " << cvCost << " %\n";
    out << "Melhor custo        : " << bestCost
        << "  (execucao #" << (bestIdx + 1) << ")\n";
    out << "Pior custo          : " << worstCost
        << "  (execucao #" << (worstIdx + 1) << ")\n\n";

    out << "--- Comparacoes ---\n";
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

        out << "Taxa de sucesso (<=10% otimo): "
            << successRate << "%  ("
            << successCount << "/" << results.size()
            << " execucoes dentro de 10% do otimo)\n";
    } else {
        out << "Melhor (SA) - otimo      : N/A (custo otimo nao disponivel)\n";
        out << "Media (SA) - otimo       : N/A (custo otimo nao disponivel)\n";
        out << "Taxa de sucesso (<=10% otimo): N/A (custo otimo nao disponivel)\n";
    }

    double melhoriaAbs = initialCost - bestCost;
    double melhoriaPct = (initialCost > 0)
                       ? 100.0 * melhoriaAbs / initialCost
                       : 0.0;
    out << "Melhor (SA) - Inicial        : " << melhoriaAbs
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