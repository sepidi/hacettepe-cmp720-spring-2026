"""
CMP720 — Test suite for the time-indexed scheduling problem.

Runs three benchmark instances of increasing difficulty through both
solvers and prints a side-by-side comparison.

  Instance 1 — Tiny, no precedence (Week 10 reduction).
               Sanity check: GA must match the ILP optimum.

  Instance 2 — n=6 DAG with FFT compatibility (Week 11 worked example).
               Both solvers should find the critical-path optimum of 11.

  Instance 3 — n=10 DAG with heterogeneous PE speeds and partial compat.
               First instance where the GA's runtime starts to matter.

Each row of the comparison table shows:
    ILP : makespan*, runtime,
    GA  : best & mean makespan over n_restarts, hit-optimum rate, runtime,
    gap = (GA_best - ILP_optimum) / ILP_optimum.

Run with:
    python test_suite.py

For more verbose Gurobi output:
    python test_suite.py --verbose
"""

from __future__ import annotations

import argparse
import time
from dataclasses import dataclass
from typing import List, Optional, Tuple

from problem import Problem, small_benchmark
from ilp_solver_ti import solve_ilp_ti
from ga_solver import GAConfig, solve_ga_multirestart


# ---------------------------------------------------------------------------
# Test instances
# ---------------------------------------------------------------------------

@dataclass
class Instance:
    name: str
    problem: Problem
    edges: List[Tuple[int, int]]
    compat: Optional[List[List[int]]]
    known_optimum: Optional[int] = None    # for sanity-checking, if known


def instance_1_tiny_no_dag() -> Instance:
    """Original small benchmark from Week 10 — no precedence, no compat.
    Optimal makespan = 7. Both solvers must find it."""
    return Instance(
        name="1. Tiny, no precedence (n=6, m=3)",
        problem=small_benchmark(),
        edges=[],
        compat=None,
        known_optimum=7,
    )


def instance_2_dag_with_compat() -> Instance:
    """Week 11 worked example — 6-task DAG, T3 and T5 (FFT) restricted to P3.
    Critical-path optimum = 11."""
    n, m = 6, 3
    durations = [2, 4, 2, 3, 2, 2]
    exec_times = [[d] * m for d in durations]
    memory = [1, 1, 1, 1, 1, 1]
    capacity = [9, 9, 9]

    edges = [(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 5)]
    compat = [[1, 1, 1] for _ in range(n)]
    compat[2] = [0, 0, 1]   # T3 → P3 only
    compat[4] = [0, 0, 1]   # T5 → P3 only

    return Instance(
        name="2. DAG + FFT compat (n=6, m=3)",
        problem=Problem(
            n=n, m=m, exec_times=exec_times,
            memory=memory, capacity=capacity,
            name="Running DAG (FFT compat)",
        ),
        edges=edges,
        compat=compat,
        known_optimum=11,
    )


def instance_3_medium_dag() -> Instance:
    """Hand-crafted 10-task DSP-style DAG with heterogeneous PE speeds.

    PE1 is a fast scalar core, PE2 is medium, PE3 is a DSP/FFT accelerator.
    T5 (FFT) and T7 (FIR convolution) are restricted to P3.
    """
    n, m = 10, 3
    exec_times = [
        [3, 5, 4],   # T1
        [4, 2, 6],   # T2
        [6, 3, 4],   # T3
        [2, 4, 3],   # T4
        [5, 8, 3],   # T5  — FFT
        [4, 5, 4],   # T6
        [3, 6, 2],   # T7  — FIR
        [5, 4, 7],   # T8
        [2, 3, 5],   # T9
        [4, 3, 4],   # T10
    ]
    memory = [2, 3, 2, 4, 1, 3, 2, 3, 2, 1]
    capacity = [12, 12, 10]

    edges = [
        (0, 1), (0, 2),     # T1 → T2, T3
        (1, 3), (1, 4),     # T2 → T4, T5
        (2, 5),             # T3 → T6
        (3, 6),             # T4 → T7
        (4, 6),             # T5 → T7
        (5, 7),             # T6 → T8
        (6, 8),             # T7 → T9
        (7, 9), (8, 9),     # T8 → T10, T9 → T10
    ]
    compat = [[1, 1, 1] for _ in range(n)]
    compat[4] = [0, 0, 1]   # T5 (FFT) → P3 only
    compat[6] = [0, 0, 1]   # T7 (FIR) → P3 only

    return Instance(
        name="3. Medium DAG (n=10, m=3)",
        problem=Problem(
            n=n, m=m, exec_times=exec_times,
            memory=memory, capacity=capacity,
            name="DSP-style 10-task DAG",
        ),
        edges=edges,
        compat=compat,
        known_optimum=None,    # let the ILP discover it
    )


# ---------------------------------------------------------------------------
# Per-instance solver runners
# ---------------------------------------------------------------------------

@dataclass
class ILPResult:
    makespan: Optional[float]
    runtime: float
    status: str
    num_vars: int
    num_constrs: int


@dataclass
class GAResult:
    best_makespan: float
    mean_makespan: float
    worst_makespan: float
    runtime: float                     # mean per-restart wall time, seconds
    total_runtime: float               # wall time across all restarts
    hit_optimum_rate: Tuple[int, int]  # (hits, total_runs)
    n_evaluations: int                 # mean across restarts


def run_ilp(inst: Instance, time_limit: float, verbose: bool) -> ILPResult:
    _, makespan, stats = solve_ilp_ti(
        inst.problem,
        edges=inst.edges,
        compat=inst.compat,
        time_limit=time_limit,
        verbose=verbose,
    )
    return ILPResult(
        makespan=makespan,
        runtime=stats["runtime"],
        status=stats["status"],
        num_vars=stats["num_vars"],
        num_constrs=stats["num_constrs"],
    )


def run_ga(
    inst: Instance,
    n_restarts: int,
    target: Optional[float],
    seed: int,
) -> GAResult:
    cfg = GAConfig(
        pop_size=60, n_generations=150,
        edges=inst.edges,
        compat=inst.compat or [[1] * inst.problem.m for _ in range(inst.problem.n)],
        p_crossover=0.9, p_mut_y=0.15, p_mut_pri=0.2,
        seed=seed,
    )
    best, runs = solve_ga_multirestart(inst.problem, n_restarts=n_restarts, base_config=cfg)
    feasible = [r for r in runs if r.feasible]
    makespans = [r.makespan for r in feasible] or [r.makespan for r in runs]
    total = sum(r.runtime for r in runs)

    if target is not None:
        hits = sum(1 for m in makespans if m == target)
    else:
        hits = sum(1 for m in makespans if m == min(makespans))

    return GAResult(
        best_makespan=best.makespan,
        mean_makespan=sum(makespans) / len(makespans),
        worst_makespan=max(makespans),
        runtime=total / len(runs),
        total_runtime=total,
        hit_optimum_rate=(hits, len(makespans)),
        n_evaluations=sum(r.n_evaluations for r in runs) // len(runs),
    )


# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

def fmt_ms(seconds: float) -> str:
    if seconds < 1.0:
        return f"{seconds * 1000:>6.1f} ms"
    return f"{seconds:>7.2f} s "


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true", help="Print Gurobi output")
    parser.add_argument("--restarts", type=int, default=10, help="GA restarts per instance")
    parser.add_argument("--ilp-time-limit", type=float, default=60.0)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()

    instances = [
        instance_1_tiny_no_dag(),
        instance_2_dag_with_compat(),
        instance_3_medium_dag(),
    ]

    rows: List[Tuple[Instance, ILPResult, GAResult]] = []

    for inst in instances:
        print(f"\n>>> Solving {inst.name}")
        print(f"    n={inst.problem.n}, m={inst.problem.m}, "
              f"edges={len(inst.edges)}, "
              f"compat-restricted tasks={sum(1 for row in (inst.compat or []) if sum(row) < inst.problem.m)}")

        ilp = run_ilp(inst, args.ilp_time_limit, args.verbose)
        print(f"    ILP : status={ilp.status}, makespan={ilp.makespan}, "
              f"vars={ilp.num_vars}, constrs={ilp.num_constrs}, runtime={fmt_ms(ilp.runtime)}")

        ga = run_ga(inst, args.restarts, ilp.makespan, seed=args.seed)
        hits, total = ga.hit_optimum_rate
        print(f"    GA  : best={ga.best_makespan:.0f}, mean={ga.mean_makespan:.2f}, "
              f"worst={ga.worst_makespan:.0f}, hit-opt={hits}/{total}, "
              f"per-run runtime={fmt_ms(ga.runtime)}")

        rows.append((inst, ilp, ga))

    # ---- summary table ----
    print("\n" + "=" * 92)
    print(" SUMMARY")
    print("=" * 92)
    header = (
        f"{'Instance':<32} | "
        f"{'ILP*':>5} {'ILP time':>10} | "
        f"{'GA best':>7} {'GA mean':>7} {'GA time':>10} | "
        f"{'gap':>6} {'hits':>7}"
    )
    print(header)
    print("-" * len(header))
    for inst, ilp, ga in rows:
        if ilp.makespan is not None and ilp.makespan > 0:
            gap = (ga.best_makespan - ilp.makespan) / ilp.makespan
            gap_str = f"{gap * 100:>5.1f}%"
        else:
            gap_str = "  n/a"
        opt_str = f"{ilp.makespan:.0f}" if ilp.makespan is not None else "—"
        hits, total = ga.hit_optimum_rate
        print(
            f"{inst.name:<32} | "
            f"{opt_str:>5} {fmt_ms(ilp.runtime):>10} | "
            f"{ga.best_makespan:>7.0f} {ga.mean_makespan:>7.2f} {fmt_ms(ga.runtime):>10} | "
            f"{gap_str:>6} {hits:>2}/{total:<3}"
        )
    print("=" * 92)
    print(
        "\nReading the table:\n"
        "  ILP*       optimal makespan reported by Gurobi (or best-found if TIME_LIMIT)\n"
        "  ILP time   wall time for one ILP solve\n"
        "  GA best    best makespan over all GA restarts\n"
        "  GA mean    mean makespan across feasible restarts\n"
        "  GA time    mean per-restart GA wall time\n"
        "  gap        (GA_best - ILP*) / ILP* — 0% means GA matched the optimum\n"
        "  hits       restarts that reached the ILP optimum / total restarts\n"
    )


if __name__ == "__main__":
    main()
