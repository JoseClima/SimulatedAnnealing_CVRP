#!/usr/bin/env python3
"""
Converte results .txt em CSV.

Modos:
  # 1) Processar a pasta inteira (padrao)
  python scripts/results_to_csv.py
  python scripts/results_to_csv.py results/txt --per-exec

  # 2) Processar UM arquivo especifico
  python scripts/results_to_csv.py results/txt/A-n32-k5_results.txt
  python scripts/results_to_csv.py results/txt/A-n32-k5_results.txt --per-exec

Saida padrao:
  - batch  -> results/csv/results_summary.csv + results_per_execution.csv
  - single -> results/csv/<instance>_summary.csv + <instance>_execution.csv
"""

import re
import csv
import argparse
from pathlib import Path


# ---------------------- parsing (igual ao anterior) ----------------------
def parse_results_file(filepath: Path):
    sections = {"basic": {}, "time": {}, "cost": {}, "comparisons": {}}
    current = "basic"
    in_table = False
    per_execution = []

    with open(filepath, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line:
                continue
            if "--- Estatisticas de Tempo ---" in line:
                current = "time";  in_table = False; continue
            if "--- Estatisticas de Custo ---" in line:
                current = "cost";  in_table = False; continue
            if "--- Comparacoes ---" in line:
                current = "comparisons"; in_table = False; continue
            if "MELHOR SOLUCAO" in line:
                current = "best";  in_table = False; continue
            if "TODAS AS EXECUCOES" in line:
                in_table = True;   current = "end";  continue

            if in_table:
                if line.startswith("#"):
                    continue
                parts = line.split()
                if len(parts) == 3:
                    try:
                        per_execution.append({
                            "execution": int(parts[0]),
                            "cost": float(parts[1]),
                            "time_s": float(parts[2]),
                        })
                    except ValueError:
                        pass
                continue

            if current in ("basic", "time", "cost", "comparisons") and ":" in line:
                key, _, val = line.partition(":")
                sections[current][key.strip()] = val.strip()

    return sections, per_execution


def _num(text, default=None):
    if text is None:
        return default
    m = re.search(r"-?\d+\.?\d*", text)
    return float(m.group(0)) if m else default


def _paren(text):
    if text is None:
        return None
    m = re.search(r"\(([^)]+)\)", text)
    return m.group(1).strip() if m else None


def _find_comp(comp: dict, prefix: str):
    for key, val in comp.items():
        if key.startswith(prefix):
            return val
    return None


def flatten(sections, filename: str) -> dict:
    b, t, c, cmp_ = sections["basic"], sections["time"], sections["cost"], sections["comparisons"]

    instance = Path(filename).name
    if instance.endswith("_results.txt"):
        instance = instance[: -len("_results.txt")]

    best_opt_txt = _find_comp(cmp_, "Melhor (SA) - otimo")
    mean_opt_txt = _find_comp(cmp_, "Media (SA) - otimo")
    succ_txt     = _find_comp(cmp_, "Taxa de sucesso")
    imp_txt      = _find_comp(cmp_, "Melhor (SA) - Inicial")

    return {
        "instance":              instance,
        "num_executions":        _num(b.get("Numero de execucoes")),
        "num_vehicles":          _num(b.get("Numero de veiculos")),
        "num_clients":           _num(b.get("Clientes (excl. dep)")),
        "nearest_neighbors":     _num(b.get("Vizinhos (nearest)")),
        "initial_temp":          _num(b.get("Temperatura inicial")),
        "decay_rate":            _num(b.get("Taxa de decaimento")),
        "final_temp":            _num(b.get("Temperatura final")),
        "max_iterations":        _num(b.get("Max iteracoes")),
        "mean_time_s":           _num(t.get("Tempo medio")),
        "std_time_s":            _num(t.get("Desvio padrao")),
        "optimal_cost":          _num(c.get("Custo otimo")),
        "initial_solution_cost": _num(c.get("Custo solu inicial")),
        "mean_cost":             _num(c.get("Custo medio")),
        "median_cost":           _num(c.get("Mediana")),
        "std_cost":              _num(c.get("Desvio padrao")),
        "cv_cost_pct":           _num(c.get("Coef. de variacao")),
        "best_cost":             _num(c.get("Melhor custo")),
        "worst_cost":            _num(c.get("Pior custo")),
        "best_vs_optimal_abs":   _num(best_opt_txt),
        "best_vs_optimal_pct":   _num(_paren(best_opt_txt)),
        "mean_vs_optimal_abs":   _num(mean_opt_txt),
        "mean_vs_optimal_pct":   _num(_paren(mean_opt_txt)),
        "success_rate_pct":      _num(succ_txt),
        "improvement_best_abs":  _num(imp_txt),
        "improvement_best_pct":  _num(_paren(imp_txt)),
    }


# ---------------------- escrita CSV ----------------------
def _write_csv(rows, path: Path, fieldnames=None):
    if not rows:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=fieldnames or list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    print(f"[OK] {path}  ({len(rows)} linhas)")


# ---------------------- modos de execucao ----------------------
def process_file(filepath: Path, out_dir: Path, per_exec: bool):
    sec, per = parse_results_file(filepath)
    row = flatten(sec, filepath.name)
    instance = row["instance"]

    _write_csv([row], out_dir / f"{instance}_summary.csv")

    if per_exec and per:
        exec_rows = [{"instance": instance, **p} for p in per]
        _write_csv(exec_rows, out_dir / f"{instance}_execution.csv",
                   fieldnames=["instance", "execution", "cost", "time_s"])


def process_folder(input_dir: Path, out_dir: Path, per_exec: bool):
    files = sorted(input_dir.glob("*_results.txt"))
    if not files:
        print(f"[!] Nenhum *_results.txt em '{input_dir}'")
        return

    print(f"[i] {len(files)} arquivo(s) encontrado(s) em {input_dir}")

    summary_rows, exec_rows = [], []
    for f in files:
        try:
            sec, per = parse_results_file(f)
            row = flatten(sec, f.name)
            summary_rows.append(row)
            for pe in per:
                exec_rows.append({"instance": row["instance"], **pe})
        except Exception as e:
            print(f"[!] Erro em {f}: {e}")

    _write_csv(summary_rows, out_dir / "results_summary.csv")
    if per_exec and exec_rows:
        _write_csv(exec_rows, out_dir / "results_per_execution.csv",
                   fieldnames=["instance", "execution", "cost", "time_s"])


# ---------------------- CLI ----------------------
def _resolve_inputs(args):
    # 1) input
    if args.input:
        inp = Path(args.input)
    elif Path("results/txt").is_dir():
        inp = Path("results/txt")
    else:
        inp = Path("results")

    # 2) output
    if args.out_dir:
        out = Path(args.out_dir)
    elif inp.is_dir() and inp.name == "txt":
        out = inp.parent / "csv"          # results/txt -> results/csv
    else:
        out = Path("results/csv")

    return inp, out


def main():
    p = argparse.ArgumentParser(
        description="Converte results .txt em CSV (pasta inteira ou arquivo unico).")
    p.add_argument("input", nargs="?",
                   help="Arquivo .txt individual OU pasta. Padrao: results/txt (ou results)")
    p.add_argument("--out-dir", default=None,
                   help="Pasta de saida. Padrao: results/csv")
    p.add_argument("--per-exec", action="store_true",
                   help="Tambem gera CSV por execucao individual")
    args = p.parse_args()

    inp, out = _resolve_inputs(args)

    if inp.is_file():
        print(f"[i] Processando arquivo: {inp}")
        process_file(inp, out, args.per_exec)
    elif inp.is_dir():
        process_folder(inp, out, args.per_exec)
    else:
        print(f"[!] Caminho invalido: {inp}")


if __name__ == "__main__":
    main()