import sys
import os
import matplotlib.pyplot as plt
from leitor_instancia import ler_instancia

"""
Uso: python scripts/comparar_solucao.py <instancia.vrp> <solucao1.txt> <solucao2.txt>
Exemplo: python scripts/comparar_solucao.py instances/A/A-n36-k5.vrp results/solucoes/inicial.txt results/solucoes/final.txt
"""

if len(sys.argv) < 4:
    sys.exit("Uso: python comparar_solucao.py <instancia.vrp> <solucao1.txt> <solucao2.txt>")

caminho_instancia = sys.argv[1]
caminho_sol1 = sys.argv[2]
caminho_sol2 = sys.argv[3]

# Extrai os nomes para o título e para o arquivo PDF
nome_sol1 = os.path.splitext(os.path.basename(caminho_sol1))[0]
nome_sol2 = os.path.splitext(os.path.basename(caminho_sol2))[0]
nome_base = os.path.splitext(os.path.basename(caminho_instancia))[0]

pasta_saida = os.path.join("results", "solucoes")
os.makedirs(pasta_saida, exist_ok=True)
caminho_pdf = os.path.join(pasta_saida, f"comp_{nome_sol1}_vs_{nome_sol2}.pdf")

# Carrega instância
instancia = ler_instancia(caminho_instancia, True)
coords = {i: (x, y) for i, (x, y) in enumerate(instancia["COORDS"])}

cores = ['#3366cc', '#dc3912', '#ff9900', '#109618', '#990099',
         '#0099c6', '#dd4477', '#66aa00', '#b82e2e', '#316395']

def desenhar_rotas(ax, caminho_arquivo, titulo):
    """Função auxiliar para desenhar uma solução em um eixo específico"""
    # Desenha o depósito e clientes
    ax.scatter(coords[0][0], coords[0][1], c='red', s=100, zorder=5)
    for i in range(1, instancia["DIMENSION"]):
        ax.scatter(coords[i][0], coords[i][1], c='black', s=20, zorder=4)

    with open(caminho_arquivo, 'r', encoding='utf-8') as f:
        linhas = f.readlines()

    k = 0
    for line in linhas:
        if "Route" not in line: continue
        clients = [int(c) for c in line.split(':')[1].strip().split(" ") if c]
        if not clients: continue
        
        clients = [0] + clients + [0]
        cor = cores[k % len(cores)]
        
        for i in range(len(clients) - 1):
            u, v = clients[i], clients[i+1]
            ax.plot([coords[u][0], coords[v][0]], [coords[u][1], coords[v][1]], 
                    c=cor, linewidth=1.5, zorder=3)
        k += 1
        
    ax.set_title(titulo, fontsize=12, fontweight='bold')
    ax.axis("off")

# Cria a figura com 1 linha e 2 colunas
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 7))

# Desenha as duas soluções lado a lado
desenhar_rotas(ax1, caminho_sol1, f"A) {nome_sol1}")
desenhar_rotas(ax2, caminho_sol2, f"B) {nome_sol2}")

plt.suptitle(f"Comparação de Soluções - Instância: {nome_base}", fontsize=14)
plt.tight_layout()

plt.savefig(caminho_pdf, bbox_inches="tight")
print(f"[OK] Gráfico comparativo salvo em: {caminho_pdf}")
plt.show()