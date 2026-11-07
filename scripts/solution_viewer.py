import sys
import networkx as nx
import matplotlib.pyplot as plt
from leitor_instancia import ler_instancia

"""
Uso:
    python scripts/solution_viewer.py <instancia.vrp> <saida_sa.txt>
"""

if len(sys.argv) < 3:
    sys.exit("Uso: python solution_viewer.py <instancia.vrp> <saida_sa.txt>")

instancia = ler_instancia(sys.argv[1], True)

G = nx.Graph()
coordenadas = {}

# Agora o C++ usa 0-indexado no vetor de clientes (0=depósito, 1..n=clientes)
# E o COORDS do Python também é 0-indexado (COORDS[0]=depósito, COORDS[1]=cliente 1).
# Portanto, o ID que vem do C++ bate exatamente com o ID do grafo.
for i in range(instancia["DIMENSION"]):
    x, y = instancia["COORDS"][i]
    G.add_node(i) 
    coordenadas[i] = (x, y)

cores = ['#3366cc', '#dc3912', '#ff9900', '#109618', '#990099',
         '#0099c6', '#dd4477', '#66aa00', '#b82e2e', '#316395',
         '#994499', '#22aa99', '#aaaa11', '#6633cc', '#e67300',
         'olive', 'fuchsia', 'darkseagreen']

linhas = open(sys.argv[2]).readlines()

k = 0
for line in linhas:
    if "Route" not in line:
        break

    # Pega a parte após os dois pontos (ex: " 25 28 \n")
    texto_rota = line.split(':')[1].strip()
    
    # Separa os clientes em uma lista
    clients = texto_rota.split(" ") if texto_rota else []
    
    # Remove strings vazias (caso a rota esteja vazia)
    clients = [c for c in clients if c]
    if not clients:
        continue

    # O C++ não imprime o depósito na rota, então nós adicionamos o 0 no início e no fim.
    clients = ["0"] + clients + ["0"]

    for i in range(len(clients) - 1):
        # Como o C++ agora imprime o ID puro, não precisamos mais subtrair 1
        u = int(clients[i])
        v = int(clients[i + 1])
        
        G.add_edge(u, v, color=cores[k % len(cores)])
        
        # Se voltou ao depósito (0), muda a cor para a próxima rota
        if v == 0:
            k += 1

edge_colors = [G.edges[e]['color'] for e in G.edges()]
node_colors = ["red" if n == 0 else "black" for n in G.nodes()]
node_sizes  = [120 if n == 0 else 25         for n in G.nodes()]

plt.figure(figsize=(10, 7))
nx.draw(G, coordenadas,
        edge_color=edge_colors,
        node_color=node_colors,
        node_size=node_sizes,
        width=1.5)
plt.title(instancia["NAME"])
plt.axis("off")
plt.tight_layout()
plt.savefig("solucao.pdf", bbox_inches="tight")
plt.show()