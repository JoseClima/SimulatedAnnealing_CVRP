import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy import stats
import glob
import os

def testar_linearidade():
    arquivos = glob.glob("results/csv/B_d*_summary.csv")
    if not arquivos:
        print("[!] Nenhum arquivo encontrado.")
        return

    dfs = [pd.read_csv(f) for f in arquivos]
    df = pd.concat(dfs, ignore_index=True)

    x = df['num_clients'].values
    y = df['time_per_level_ms'].values

    slope, intercept, r_value, p_value, std_err = stats.linregress(x, y)
    r_squared = r_value ** 2

    plt.figure(figsize=(9, 6))
    
    plt.scatter(x, y, color='steelblue', alpha=0.7, edgecolors='k', label='Dados Reais')

    x_linha = np.array([min(x), max(x)])
    y_linha = slope * x_linha + intercept
    plt.plot(x_linha, y_linha, color='red', linewidth=2, linestyle='--', 
             label=f'Tendência Linear (R² = {r_squared:.3f})')

    eq_texto = f'y = {slope:.5f} * x + {intercept:.5f}'
    plt.text(min(x) + 5, max(y) * 0.9, eq_texto, color='red', fontsize=11, 
             bbox=dict(facecolor='white', alpha=0.8))

    plt.xlabel('Número de Clientes (N)', fontsize=12)
    plt.ylabel('Tempo por Nível (ms)', fontsize=12)
    plt.title('Teste de Linearidade: Tempo por Nível vs Tamanho da Instância', fontsize=14)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    plt.tight_layout()

    os.makedirs("results/solucoes", exist_ok=True)
    caminho_pdf = "results/solucoes/teste_linearidade_tempo.pdf"
    plt.savefig(caminho_pdf, bbox_inches="tight")
    print(f"[OK] Gráfico de linearidade salvo em: {caminho_pdf}")
    print(f"[INFO] Coeficiente R²: {r_squared:.4f}")
    print(f"[INFO] Equação: {eq_texto}")
    plt.show()

if __name__ == "__main__":
    testar_linearidade()