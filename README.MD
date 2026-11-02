# VRP com SA

Este projeto implementa o **Simulated Annealing (SA)** para o **CVRP** (Problema de Roteamento de Veículos com Capacidade).


## execução

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

# Limpar artefatos do build (mantem build/ configurado)
cmake --build build --target clean

# Listar alvos disponiveis e pasta de testes atual
cmake --build build --target targets

#exporta para csv os resultados
cmake --build build --target export-csv

#exporta para csv um unico arquivo de resultados
python scripts/results_to_csv.py results/txt/nome_do_arquivo_results --per-exec
#caso nao informe o --per-exec, ele roda o SA uma vez

# Trocar de pasta de testes (sem apagar build/)
cmake -S . -B build -DTESTS_DIR=B

# Apagar tudo do build e reconfigurar do zero
Remove-Item -Recurse -Force build