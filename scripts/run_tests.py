#!/usr/bin/env python3
"""
Roda o SA em lote com varias configuracoes sobre uma ou mais pastas de instancias.
Aceita multiplos arquivos JSON e curingas (*.json).
"""

import argparse
import json
import subprocess
import sys
import glob
from pathlib import Path


def load_configs(paths):
    configs = []
    for path in paths:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
            # Adiciona o nome do arquivo de origem para exibir no terminal
            for cfg in data:
                cfg["_source_file"] = Path(path).name
                configs.append(cfg)
    return configs


def find_instances(dirs):
    files = []
    for d in dirs:
        p = Path(d)
        if p.is_file() and p.suffix == ".vrp":
            files.append(p)
        elif p.is_dir():
            files.extend(sorted(p.rglob("*.vrp")))
        else:
            print(f"[!] Nao encontrado: {d}")
    return files


def run_one(exe: Path, vrp: Path, cfg: dict):
    cmd = [
        str(exe), str(vrp), "--statistics",
        "--t0",    str(cfg["t0"]),
        "--decay", str(cfg["decay"]),
        "--tf",    str(cfg["tf"]),
        "--iter",  str(cfg["iter"]),
        "--nearest-fraction", str(cfg["nearest_fraction"]),
        "--tag",   str(cfg["tag"]),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"\n[!] FALHOU ao rodar {vrp.name} com tag {cfg['tag']}:")
        print(result.stdout)
        print(result.stderr)
        return False
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", required=True, help="Caminho do executavel")
    ap.add_argument("--instances", nargs="+", required=True, help="Pastas ou arquivos .vrp")
    ap.add_argument("--configs", nargs="+", required=True, help="Arquivos JSON (aceita curinga *.json)")
    ap.add_argument("--skip-csv", action="store_true", help="Nao roda results_to_csv.py no final")
    args = ap.parse_args()

    exe = Path(args.bin).resolve()
    if not exe.exists():
        sys.exit(f"[!] Executavel nao encontrado: {exe}")

    # Expande o curinga (*.json) no Windows e Linux
    expanded_config_paths = []
    for pattern in args.configs:
        matches = glob.glob(pattern)
        if not matches:
            print(f"[!] Aviso: Nenhum arquivo encontrado para o padrao '{pattern}'")
        expanded_config_paths.extend(matches)

    if not expanded_config_paths:
        sys.exit("[!] Nenhuma configuracao valida encontrada.")

    configs = load_configs(expanded_config_paths)
    instances = find_instances(args.instances)
    
    if not instances:
        sys.exit("[!] Nenhum .vrp encontrado")
    if not configs:
        sys.exit("[!] Nenhuma configuracao encontrada nos JSONs")

    print(f"[i] {len(instances)} instancia(s) x {len(configs)} config(s) = {len(instances) * len(configs)} execucoes\n")

    Path("results/txt").mkdir(parents=True, exist_ok=True)

    fails = 0
    count = 0
    total = len(instances) * len(configs)

    for vrp in instances:
        print(f"\n>>> Processando: {vrp.name}")
        for cfg in configs:
            count += 1
            # Exibe o nome do JSON e a tag!
            print(f"  [{count}/{total}] {cfg['_source_file']} -> tag: {cfg['tag']} (T0={cfg['t0']}, d={cfg['decay']})")
            if not run_one(exe, vrp, cfg):
                fails += 1

    print(f"\n[OK] {total - fails} execucoes OK, {fails} falha(s)")

    if not args.skip_csv:
        print("\n[i] Consolidando em CSV...")
        Path("results/csv").mkdir(parents=True, exist_ok=True)
        
        tags = list(set(cfg["tag"] for cfg in configs))
        
        for tag in tags:
            print(f"  Gerando CSV para a configuracao: {tag}...")
            subprocess.run([
                sys.executable, "scripts/results_to_csv.py",
                "results/txt", 
                "--filter", f"_{tag}_",
                "--experiment", tag,
                "--out-dir", "results/csv"
            ], check=False)


if __name__ == "__main__":
    main()