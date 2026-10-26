# VRP com SA

Este projeto implementa o **Simulated Annealing (SA)** para o **CVRP** (Problema de Roteamento de Veículos com Capacidade).

## execução

# Configure (uma vez, ou ao trocar de pasta de testes)
cmake -S . -B build -G "MinGW Makefiles" -DTESTS_DIR=A

# Compilar
cmake --build build

# Rodar todos os .vrp da pasta escolhida (sem estatísticas)
cmake --build build --target run-all

# Rodar todos os .vrp da pasta escolhida com --statistics
cmake --build build --target run-stats

# Rodar todos os .vrp de tests/ recursivamente (sem estatísticas)
cmake --build build --target run-all-folders

# Rodar todos os .vrp de tests/ recursivamente com --statistics
cmake --build build --target run-stats-folders

# Limpar artefatos do build (mantém build/ configurado)
cmake --build build --target clean

# Listar alvos disponíveis e pasta de testes atual
cmake --build build --target targets

# Trocar de pasta de testes (sem apagar build/)
cmake -S . -B build -DTESTS_DIR=B

# Apagar tudo do build e reconfigurar do zero
Remove-Item -Recurse -Force build