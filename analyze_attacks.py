#!/usr/bin/env python3
"""
Analyze attack logs (global and relay-level) and generate plots + metrics.

Supports many CSV-like files at once. It is resilient to your current logging format:
- Global attack lines: "epoch,size"
- Relay attack lines:  "epoch,step,size"  OR "epoch,size" with occasional "successness,1/0" markers.
- Lines starting with "successness" or "Average Response time" are ignored (except successness bumps a step counter).
- Other non-numeric lines are ignored.

Outputs for each input file:
- <stem>_observed.png                      (epoch vs observed-set size)
- <stem>_observed_effectiveness.png        (delta drop per point)
- For relay logs, each deanonymization step is a separate line in the observed.png figure.

Usage examples:
  python analyze_attacks.py --global path/to/global1.csv path/to/global2.csv --relay path/to/relay1.csv --out outdir
  python analyze_attacks.py --relay logs/*relay*.csv
  python analyze_attacks.py --global logs/global_attack.csv --show
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Tuple

import pandas as pd
import matplotlib.pyplot as plt


# ------------------------------
# Parsing
# ------------------------------

@dataclass
class ParsedLog:
    kind: str                 # "global" or "relay"
    df: pd.DataFrame          # columns: epoch, size [, step], effectiveness
    stem: str                 # file stem (for naming plots)


def _try_parse_ints(parts: List[str]) -> Optional[List[int]]:
    try:
        return [int(p.strip()) for p in parts]
    except Exception:
        return None


def parse_global_file(path: Path) -> ParsedLog:
    data = []
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            s = line.strip()
            if not s:
                continue
            low = s.lower()
            if low.startswith("successness") or low.startswith("average"):
                continue
            parts = [p for p in s.split(",") if p != ""]
            ints = _try_parse_ints(parts)
            if ints is None:
                continue
            if len(ints) == 2:
                epoch, size = ints
                data.append({"epoch": epoch, "size": size})

    df = pd.DataFrame(data).sort_values("epoch").reset_index(drop=True)
    df["effectiveness"] = df["size"].shift(1) - df["size"]
    return ParsedLog(kind="global", df=df, stem=path.stem)


def parse_relay_file(path: Path) -> ParsedLog:
    """
    PRE-STEP is mapped to Relay 1.
    Any explicit or implicit step S is stored as (S + 1).
    """
    data = []
    # This counts how many successness markers we've seen so far
    success_count = 0
    # Current (implicit) step index BEFORE shifting — starts at 0
    current_step_zero_based = 0

    with path.open("r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            s = line.strip()
            if not s:
                continue

            low = s.lower()
            if low.startswith("successness"):
                # bump the zero-based step index
                success_count += 1
                current_step_zero_based = success_count
                continue
            if low.startswith("average"):
                continue

            parts = [p for p in s.split(",") if p != ""]
            ints = _try_parse_ints(parts)
            if ints is None:
                continue

            if len(ints) == 3:
                epoch, step_in_file, size = ints
                # Shift +1 so that 0->1, 1->2, ...
                step_shifted = step_in_file + 1
                data.append({"epoch": epoch, "step": step_shifted, "size": size})
            elif len(ints) == 2:
                epoch, size = ints
                # Use the implicit step (zero-based) and then shift +1
                step_shifted = current_step_zero_based + 1
                data.append({"epoch": epoch, "step": step_shifted, "size": size})
            # else ignore

    df = pd.DataFrame(data)
    if df.empty:
        return ParsedLog(kind="relay", df=df, stem=path.stem)

    df = df.sort_values(["step", "epoch"]).reset_index(drop=True)
    df["effectiveness"] = df.groupby("step")["size"].shift(1) - df["size"]
    return ParsedLog(kind="relay", df=df, stem=path.stem)


# ------------------------------
# Plotting helpers
# ------------------------------

def plot_global(parsed: ParsedLog, out_dir: Path, show: bool) -> Tuple[Path, Path]:
    df = parsed.df
    if df.empty:
        print(f"[WARN] {parsed.stem}: no global data recognized.")
        return (out_dir / f"{parsed.stem}_observed.png",
                out_dir / f"{parsed.stem}_observed_effectiveness.png")

    fig = plt.figure(figsize=(10, 6))
    plt.plot(df["epoch"], df["size"], label="Observed Set Size")
    plt.xlabel("Epoch")
    plt.ylabel("Observed Set Size")
    plt.title(f"Global Adversary: Observed Set Size — {parsed.stem}")
    plt.legend()
    plt.grid(True)
    out1 = out_dir / f"{parsed.stem}_observed.png"
    fig.savefig(out1, bbox_inches="tight")
    plt.close(fig)

    fig = plt.figure(figsize=(10, 6))
    plt.plot(df["epoch"], df["effectiveness"], label="Effectiveness")
    plt.xlabel("Epoch")
    plt.ylabel("Effectiveness (prev size - curr size)")
    plt.title(f"Global Adversary: Effectiveness — {parsed.stem}")
    plt.legend()
    plt.grid(True)
    out2 = out_dir / f"{parsed.stem}_observed_effectiveness.png"
    fig.savefig(out2, bbox_inches="tight")
    plt.close(fig)

    if show:
        plt.show()

    return out1, out2


def plot_relay(parsed: ParsedLog, out_dir: Path, show: bool) -> Tuple[Path, Path]:
    df = parsed.df
    if df.empty:
        print(f"[WARN] {parsed.stem}: no relay data recognized.")
        return (out_dir / f"{parsed.stem}_observed.png",
                out_dir / f"{parsed.stem}_observed_effectiveness.png")

    # One line per deanonymization step (Relay 1 == former prestep)
    fig = plt.figure(figsize=(10, 6))
    for step, group in df.groupby("step"):
        label = f"Relay {int(step)} Deanonymization Step"
        plt.plot(group["epoch"], group["size"], label=label)
    plt.xlabel("Epoch")
    plt.ylabel("Observed Set Size")
    plt.title(f"Relay-Level: Observed Set Size per Step — {parsed.stem}")
    plt.legend()
    plt.grid(True)
    out1 = out_dir / f"{parsed.stem}_observed.png"
    fig.savefig(out1, bbox_inches="tight")
    plt.close(fig)

    # Effectiveness per step
    fig = plt.figure(figsize=(10, 6))
    for step, group in df.groupby("step"):
        label = f"Relay {int(step)} Effectiveness"
        plt.plot(group["epoch"], group["effectiveness"], label=label)
    plt.xlabel("Epoch")
    plt.ylabel("Effectiveness (prev size - curr size)")
    plt.title(f"Relay-Level: Effectiveness per Step — {parsed.stem}")
    plt.legend()
    plt.grid(True)
    out2 = out_dir / f"{parsed.stem}_observed_effectiveness.png"
    fig.savefig(out2, bbox_inches="tight")
    plt.close(fig)

    if show:
        plt.show()

    return out1, out2


# ------------------------------
# CLI
# ------------------------------

def main():
    ap = argparse.ArgumentParser(description="Analyze global/relay attack logs and plot metrics.")
    ap.add_argument("--global", dest="global_files", nargs="*", default=[],
                    help="Paths to global attack CSV logs.")
    ap.add_argument("--relay", dest="relay_files", nargs="*", default=[],
                    help="Paths to relay attack CSV logs.")
    ap.add_argument("--out", dest="out_dir", default="plots",
                    help="Output directory for generated images (default: plots).")
    ap.add_argument("--show", action="store_true",
                    help="Display plots interactively in addition to saving.")
    args = ap.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    for f in args.global_files:
        p = Path(f)
        parsed = parse_global_file(p)
        g1, g2 = plot_global(parsed, out_dir, show=args.show)
        print(f"[OK] Global plots written: {g1}, {g2}")

    for f in args.relay_files:
        p = Path(f)
        parsed = parse_relay_file(p)
        r1, r2 = plot_relay(parsed, out_dir, show=args.show)
        print(f"[OK] Relay plots written: {r1}, {r2}")


if __name__ == "__main__":
    main()
