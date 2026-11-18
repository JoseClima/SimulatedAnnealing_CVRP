import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import glob

def gerar_graficos():
    pasta_csv = "results/csv"
    
    arquivos = glob.glob(os.path.join(pasta_csv, "B_d*_summary.csv"))
    
    if not arquivos:
        print("[!] Nenhum arquivo B_d*_summary.csv encontrado em results/csv/")
        return

    dfs = [pd.read_csv(f) for f in arquivos]
    df = pd.concat(dfs, ignore_index=True)

    os.makedirs("results/solucoes", exist_ok=True)

    # =========================================================================
    # GRÁFICO 1: Barras - Níveis Efetivos vs Taxa de Resfriamento
    # =========================================================================
    plt.figure(figsize=(8, 5))
    
    df_levels = df.groupby('decay_rate')['levels_effective'].mean().reset_index()
    df_levels = df_levels.sort_values('decay_rate')

    bars = plt.bar(df_levels['decay_rate'].astype(str), df_levels['levels_effective'], 
                   color=['#4C72B0', '#55A868', '#C44E52', '#8172B2', '#CCB974'])

    for bar in bars:
        yval = bar.get_height()
        
        texto_valor = f'{int(yval):,}'.replace(',', '.') 
        
        plt.text(bar.get_x() + bar.get_width()/2, yval + (yval * 0.03), 
                 texto_valor, ha='center', va='bottom', fontsize=10, fontweight='bold')

    max_val = df_levels['levels_effective'].max()
    plt.ylim(0, max_val * 1.15)

    plt.xlabel('Taxa de Resfriamento (α)', fontsize=12)
    plt.ylabel('Níveis Efetivos Médios (L)', fontsize=12)
    plt.title('Crescimento Exponencial dos Níveis Térmicos por α', fontsize=14)
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig("results/solucoes/grafico_decay_barras.pdf", bbox_inches="tight")
    print("[OK] Gráfico de barras salvo em: results/solucoes/grafico_decay_barras.pdf")
    plt.close()


    # =========================================================================
    # GRÁFICO 2: Linhas - Tempo por Nível vs Número de Clientes
    # =========================================================================
    plt.figure(figsize=(10, 6))

    df_time = df.sort_values('num_clients')

    alphas = sorted(df_time['decay_rate'].unique())
    cores = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']

    for i, alpha in enumerate(alphas):
        subset = df_time[df_time['decay_rate'] == alpha]
        plt.plot(subset['num_clients'], subset['time_per_level_ms'], 
                 marker='o', markersize=5, linewidth=2, 
                 color=cores[i % len(cores)], label=f'α = {alpha}', alpha=0.8)

    destaques = {
        15: 'P-n16-k8',
        54: 'P-n55-k7',
        100: 'P-n101-k4'
    }
    for n_cli, nome in destaques.items():
        val = df_time[df_time['num_clients'] == n_cli]['time_per_level_ms'].mean()
        if pd.notna(val):
            plt.axvline(x=n_cli, color='gray', linestyle='--', alpha=0.5)
            plt.text(n_cli, max(df_time['time_per_level_ms']) * 0.9, nome, 
                     rotation=90, va='top', ha='right', fontsize=9, color='gray')

    plt.xlabel('Número de Clientes (N)', fontsize=12)
    plt.ylabel('Tempo por Nível (ms)', fontsize=12)
    plt.title('Complexidade Computacional por Nível vs Tamanho da Instância', fontsize=14)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(title='Taxa de Resfriamento')
    plt.tight_layout()
    plt.savefig("results/solucoes/grafico_decay_tempo.pdf", bbox_inches="tight")
    print("[OK] Gráfico de linhas salvo em: results/solucoes/grafico_decay_tempo.pdf")
    plt.close()

if __name__ == "__main__":
    gerar_graficos()