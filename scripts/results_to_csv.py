#!/usr/bin/env python3
"""
Converte results .txt em CSV.
"""

import re
import csv
import math
import argparse
from pathlib import Path
from fractions import Fraction

# ---------------------- parsing (secoes do .txt) ----------------------
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


# ---------------------- utilitarios de extracao ----------------------
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


def _safe_div(a, b):
    return (a / b) if (b not in (None, 0)) else None


def _build_config_id(t0, decay, tf, it, n):
    if t0 is None or decay is None or tf is None or it is None or n is None:
        return None
    def fmt(x):
        return f"{x:g}"
    return f"T{fmt(t0)}_d{fmt(decay)}_Tf{fmt(tf)}_it{int(it)}_n{int(n)}"


def _compute_levels(t0, decay, tf, max_iter):
    if None in (t0, decay, tf, max_iter):
        return None
    if t0 <= 0 or tf <= 0 or decay <= 0 or decay >= 1 or tf >= t0:
        return None
    L = math.log(t0 / tf) / math.log(1.0 / decay)
    return min(math.ceil(L), int(max_iter))


# ---------------------- flatten: 1 linha de summary ----------------------
def flatten(sections, filename: str, experiment: str = "") -> dict:
    b, t, c, cmp_ = sections["basic"], sections["time"], sections["cost"], sections["comparisons"]

    instance = Path(filename).name
    if instance.endswith("_results.txt"):
        instance = instance[: -len("_results.txt")]
    
    # Remove a tag do nome da instância
    if experiment and instance.endswith(f"_{experiment}"):
        instance = instance[: -len(f"_{experiment}")]

    best_opt_txt = _find_comp(cmp_, "Melhor (SA) - otimo")
    mean_opt_txt = _find_comp(cmp_, "Media (SA) - otimo")
    succ_txt     = _find_comp(cmp_, "Taxa de sucesso")
    imp_txt      = _find_comp(cmp_, "Melhor (SA) - Inicial")

    row = {
        "instance":              instance,
        "experiment":            experiment,
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

    inj = row["initial_solution_cost"]
    row["improvement_mean_pct"]   = _safe_div(100 * (inj - row["mean_cost"]), inj)   if inj else None
    row["improvement_median_pct"] = _safe_div(100 * (inj - row["median_cost"]), inj) if inj else None
    row["improvement_worst_pct"]  = _safe_div(100 * (inj - row["worst_cost"]), inj)  if inj else None

    opt = row["optimal_cost"]
    row["median_vs_optimal_pct"] = _safe_div(100 * (row["median_cost"] - opt), opt) if opt else None
    row["worst_vs_optimal_pct"]  = _safe_div(100 * (row["worst_cost"]  - opt), opt) if opt else None

    if row["best_cost"] is not None and row["worst_cost"] is not None:
        row["range_cost"] = row["worst_cost"] - row["best_cost"]
        row["spread_pct"] = _safe_div(100 * row["range_cost"], row["mean_cost"])
    else:
        row["range_cost"] = None
        row["spread_pct"] = None

    levels = _compute_levels(row["initial_temp"], row["decay_rate"],
                             row["final_temp"], row["max_iterations"])
    row["levels_effective"] = levels
    row["time_per_level_ms"] = (
        _safe_div(row["mean_time_s"] * 1000.0, levels) if levels else None
    )
    row["improvement_per_second"] = _safe_div(row["improvement_best_abs"], row["mean_time_s"])

    row["config_id"] = _build_config_id(
        row["initial_temp"], row["decay_rate"], row["final_temp"],
        row["max_iterations"], row["nearest_neighbors"],
    )
    
    n  = row["nearest_neighbors"]
    nc = row["num_clients"]
    if n is not None and nc is not None:
        nd = int(nc) + 1
        row["nearest_ratio_nodes"]   = n / nd
        frac = Fraction(int(n), nd)
        if frac.denominator <= 16:
            row["nearest_fraction"] = f"{frac.numerator}/{frac.denominator}"
        else:
            row["nearest_fraction"] = f"{int(n)}/{nd}"
    else:
        row["nearest_ratio_nodes"] = None
        row["nearest_fraction"] = None

    return row


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


# ---------------------- modos ----------------------
def process_file(filepath: Path, out_dir: Path, per_exec: bool, experiment: str):
    sec, per = parse_results_file(filepath)
    row = flatten(sec, filepath.name, experiment=experiment)
    instance = row["instance"]

    nome_arquivo = f"{experiment}_summary.csv" if experiment else f"{instance}_summary.csv"
    _write_csv([row], out_dir / nome_arquivo)

    if per_exec and per:
        exec_rows = [{
            "instance": instance, "config_id": row["config_id"],
            "experiment": experiment, "execution": p["execution"],
            "cost": p["cost"], "time_s": p["time_s"],
        } for p in per]
        nome_exec = f"{experiment}_execution.csv" if experiment else f"{instance}_execution.csv"
        _write_csv(exec_rows, out_dir / nome_exec,
                   fieldnames=["instance", "config_id", "experiment", "execution", "cost", "time_s"])


def process_folder(input_dir: Path, out_dir: Path, per_exec: bool, experiment: str, filter_str: str = None):
    files = sorted(input_dir.glob("*_results.txt"))
    
    if filter_str:
        files = [f for f in files if filter_str in f.name]
        
    if not files:
        print(f"[!] Nenhum *_results.txt em '{input_dir}'")
        return

    print(f"[i] {len(files)} arquivo(s) encontrado(s) em {input_dir}")

    summary_rows, exec_rows = [], []
    for f in files:
        try:
            sec, per = parse_results_file(f)
            row = flatten(sec, f.name, experiment=experiment)
            summary_rows.append(row)
            for pe in per:
                exec_rows.append({
                    "instance": row["instance"], "config_id": row["config_id"],
                    "experiment": experiment, "execution": pe["execution"],
                    "cost": pe["cost"], "time_s": pe["time_s"],
                })
        except Exception as e:
            print(f"[!] Erro em {f}: {e}")

    nome_arquivo = f"{experiment}_summary.csv" if experiment else "results_summary.csv"
    _write_csv(summary_rows, out_dir / nome_arquivo)
    
    if per_exec and exec_rows:
        nome_exec = f"{experiment}_execution.csv" if experiment else "results_per_execution.csv"
        _write_csv(exec_rows, out_dir / nome_exec,
                   fieldnames=["instance", "config_id", "experiment", "execution", "cost", "time_s"])


# ---------------------- CLI ----------------------
def _resolve_inputs(args):
    if args.input:
        inp = Path(args.input)
    elif Path("results/txt").is_dir():
        inp = Path("results/txt")
    else:
        inp = Path("results")

    if args.out_dir:
        out = Path(args.out_dir)
    elif inp.is_dir() and inp.name == "txt":
        out = inp.parent / "csv"
    else:
        out = Path("results/csv")

    return inp, out


def main():
    p = argparse.ArgumentParser(
        description="Converte results .txt em CSV (pasta inteira ou arquivo unico).")
    p.add_argument("input", nargs="?", default=None,
                   help="Arquivo .txt individual OU pasta. Padrao: results/txt (ou results)")
    p.add_argument("--out-dir", default=None,
                   help="Pasta de saida. Padrao: results/csv")
    p.add_argument("--per-exec", action="store_true",
                   help="Tambem gera CSV por execucao individual")
    p.add_argument("--experiment", default="",
                   help="Rotulo do experimento (ex: T500). Vai numa coluna extra e nomeia o arquivo.")
    p.add_argument("--filter", default=None,
                   help="Filtra arquivos pelo nome (ex: _T500_)")
    args = p.parse_args()

    inp, out = _resolve_inputs(args)

    if inp.is_file():
        print(f"[i] Processando arquivo: {inp}")
        process_file(inp, out, args.per_exec, args.experiment)
    elif inp.is_dir():
        process_folder(inp, out, args.per_exec, args.experiment, args.filter)
    else:
        print(f"[!] Caminho invalido: {inp}")


if __name__ == "__main__":
    main()