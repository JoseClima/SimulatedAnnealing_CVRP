# VRP com SA

Este projeto implementa o **Simulated Annealing (SA)** para o **CVRP** (Problema de Roteamento de Veículos com Capacidade).

## Requisitos e execução

Tenha o `make` e um compilador C++ instalados e disponíveis no PATH. No terminal, entre na pasta `scripts` e execute:

```sh
cd scripts
mingw32-make                      # compila
mingw32-make run-stats            # roda tests/A com --statistics
mingw32-make run-stats TESTS_DIR=tests/qualquerPasta
mingw32-make run-stats-folders    # roda todos os .vrp dentro de tests/
mingw32-make clean                # apaga .o e o binário
mingw32-make help                 # lista todos os comandos
```

**Aviso 1:** usei o mingw32-make pois estou no windows
**Aviso 2:** para executar de qualquer pasta, use `make -C caminho/para/VRP/scripts`.
