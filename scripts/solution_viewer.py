import sys
import os
import matplotlib.pyplot as plt
from leitor_instancia import ler_instancia

"""
Uso: python scripts/solution_viewer.py <instancia.vrp> <arquivo_solucao.txt>
Exemplo: python scripts/solution_viewer.py instances/A/A-n36-k5.vrp results/solucoes/A-n36-k5_TF1000.txt
"""

if len(sys.argv) < 3:
    sys.exit("Uso: python solution_viewer.py <instancia.vrp> <arquivo_solucao.txt>")

caminho_instancia = sys.argv[1]
caminho_solucao = sys.argv[2]

nome_base = os.path.splitext(os.path.basename(caminho_solucao))[0]

pasta_saida = os.path.join("results", "solucoes")
os.makedirs(pasta_saida, exist_ok=True)
caminho_pdf = os.path.join(pasta_saida, f"{nome_base}.pdf")

instancia = ler_instancia(caminho_instancia, True)
coords = {i: (x, y) for i, (x, y) in enumerate(instancia["COORDS"])}

cores = ['#3366cc', '#dc3912', '#ff9900', '#109618', '#990099',
         '#0099c6', '#dd4477', '#66aa00', '#b82e2e', '#316395',
         '#994499', '#22aa99', '#aaaa11', '#6633cc', '#e67300']

fig, ax = plt.subplots(figsize=(10, 7))

# Depósito (Nó 0)
dep_x, dep_y = coords[0]
ax.scatter(dep_x, dep_y, c='red', s=150, zorder=5, label='Depósito')

# Clientes
for i in range(1, instancia["DIMENSION"]):
    x, y = coords[i]
    ax.scatter(x, y, c='black', s=30, zorder=4)

with open(caminho_solucao, 'r', encoding='utf-8') as f:
    linhas = f.readlines()

k = 0
for line in linhas:
    if "Route" not in line:
        continue

    texto_rota = line.split(':')[1].strip()
    clients = [int(c) for c in texto_rota.split(" ") if c]
    if not clients:
        continue

    clients = [0] + clients + [0]
    cor = cores[k % len(cores)]

    for i in range(len(clients) - 1):
        u = clients[i]
        v = clients[i + 1]
        x_vals = [coords[u][0], coords[v][0]]
        y_vals = [coords[u][1], coords[v][1]]
        ax.plot(x_vals, y_vals, c=cor, linewidth=1.5, zorder=3)
    
    k += 1

# Título do gráfico com o nome do arquivo
plt.title(f"Solução: {nome_base}", fontsize=14, fontweight='bold')
plt.axis("off")
plt.tight_layout()

# Salva no local e com o nome correto
plt.savefig(caminho_pdf, bbox_inches="tight")
print(f"[OK] Grafico salvo em: {caminho_pdf}")
plt.show()