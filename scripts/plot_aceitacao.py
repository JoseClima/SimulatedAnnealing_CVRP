#só funciona com algumas linhas do SA.cpp descomentadas

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

def plotar_aceitacao(caminho_csv="results/csv/aceitacao_real.csv"):
    try:
        df = pd.read_csv(caminho_csv)
    except FileNotFoundError:
        print(f"[!] Arquivo nao encontrado: {caminho_csv}")
        return

    for col in ['initialTemp', 'T', 'deltaC', 'accepted']:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')
    
    df = df.dropna()

    if df.empty:
        print("[!] O arquivo CSV esta vazio ou corrompido.")
        return

    delta_alvo = 50

    plt.figure(figsize=(10, 6))

    df['log_T'] = np.log10(df['T'])
    df['T_bin'] = pd.cut(df['log_T'], bins=50)

    for t0 in sorted(df['initialTemp'].unique()):
        mask = (df['initialTemp'] == t0) & (df['deltaC'] >= delta_alvo * 0.9) & (df['deltaC'] <= delta_alvo * 1.1)
        df_sub = df[mask]

        if df_sub.empty:
            continue

        grouped = df_sub.groupby('T_bin', observed=False).agg(
            T_medio=('T', 'mean'),
            Aceitacao=('accepted', 'mean')
        ).dropna().sort_values('T_medio', ascending=False)

        plt.plot(grouped['T_medio'], grouped['Aceitacao'] * 100, 
                 marker='.', markersize=6, linewidth=2, 
                 label=f'T0 = {t0}')

    plt.xscale('log')
    plt.gca().invert_xaxis()
    plt.xlabel('Temperatura (T) - Escala Logarítmica')
    plt.ylabel(f'Probabilidade de Aceitação (%) para ΔC ≈ {delta_alvo}')
    plt.title('Curva de Aceitação Empírica: Impacto da Temperatura Inicial')
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend()
    plt.tight_layout()
 
    os.makedirs("results/solucoes", exist_ok=True)
    caminho_pdf = "results/solucoes/aceitacao_T0_1.pdf"
    plt.savefig(caminho_pdf, bbox_inches="tight")
    print(f"[OK] Gráfico salvo em: {caminho_pdf}")
    plt.show()

if __name__ == "__main__":
    plotar_aceitacao()