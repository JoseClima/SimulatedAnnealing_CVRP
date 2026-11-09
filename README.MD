# VRP com SA

Este projeto implementa o **Simulated Annealing (SA)** para o **CVRP** (Problema de Roteamento de Veículos com Capacidade), com foco em experimentação automatizada e análise de dados.

## 📋 Pré-requisitos ##
Para rodar os scripts de análise e visualização, você precisará do Python 3 e das seguintes bibliotecas:
```bash
pip install pandas matplotlib networkx
```

## Execução ##
```bash
# Configure (uma vez, ou ao trocar de pasta de testes)
cmake -S . -B build -G "MinGW Makefiles" -DTESTS_DIR=A

# Compilar
cmake --build build

# Rodar todos os .vrp da pasta escolhida (sem estatisticas)
cmake --build build --target run-all

# Rodar todos os .vrp da pasta escolhida com --statistics
cmake --build build --target run-stats

# Rodar todos os .vrp de instances/ recursivamente (sem estatisticas)
cmake --build build --target run-all-folders

# Rodar todos os .vrp de instances/ recursivamente com --statistics
cmake --build build --target run-stats-folders

# Listar alvos disponiveis e pasta de testes atual
cmake --build build --target targets

# Limpar artefatos do build (mantem build/ configurado)
cmake --build build --target clean

# Trocar de pasta de testes (sem apagar build/)
cmake -S . -B build -DTESTS_DIR=B

# Apagar tudo do build e reconfigurar do zero
Remove-Item -Recurse -Force build
```

## Testes automatizados
```bash
# Rodar todas as configurações de uma pasta JSON em todas as instâncias
python scripts/run_tests.py --bin build/vrp_teste.exe --instances instances/A instances/B instances/E --configs configs/*.json

# Rodar apenas uma configuração específica em uma instância
python scripts/run_tests.py --bin build/vrp_teste.exe --instances instances/A/A-n32-k5.vrp --configs configs/tempFinal.json
```

## Extração de Dados para o CSV
```bash
# Exportar todas as pastas via CMake
cmake --build build --target export-csv

# Exportar em lote manualmente (todos os arquivos de results/txt)
python scripts/results_to_csv.py results/txt --per-exec

# Exportar um lote específico com uma tag de experimento (ex: TesteA)
python scripts/results_to_csv.py results/txt --per-exec --experiment TesteA --out-dir results/csv

# Exportar um único arquivo de resultado
python scripts/results_to_csv.py results/txt/A-n32-k5_results.txt
```

## Visualizar Soluções (feito por https://github.com/LuizFXavier)
```bash
#visualizar uma solução
python scripts/solution_viewer.py instances/A/A-n36-k5.vrp results/solucoes/A-n36-k5_TF1000.txt

#compara soluções
python scripts/comparar_solucao.py instances/A/A-n36-k5.vrp results/solucoes/inicial.txt results/solucoes/final.txt
```