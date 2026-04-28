"""
CMP720 — Week 11: Time-indexed ILP for makespan minimization
              on heterogeneous PEs with precedence and compatibility.

This is a direct extension of Week 10's Model A. We replace the 2D
binary variable x[i,j] with a 3D time-indexed variable

    x[i,j,t] ∈ {0,1},   1 iff task i *starts* at time t on processor j.

That single change lets us add precedence edges and per-task PE
compatibility without introducing big-M ordering variables.

Variables
---------
    x[i,j,t] ∈ {0,1}    1 iff task i starts at time t on PE j
    C_max    ≥ 0        makespan (continuous)

Helper expressions (linear in x):
    start(i) = Σ_{j,t} t · x[i,j,t]
    end(i)   = Σ_{j,t} (t + exec[i][j]) · x[i,j,t]

Constraints
-----------
    C1  Σ_{j,t} x[i,j,t] = 1                              ∀ i
    C2  Σ_i mem[i] · (Σ_t x[i,j,t]) ≤ cap[j]              ∀ j
    C3  x[i,j,t] is created only when compat[i][j] = 1
    C4  end(u) ≤ start(v)                                 ∀ (u,v) ∈ E
    C5  Σ_i Σ_{τ : τ ≤ t < τ+exec[i][j]} x[i,j,τ] ≤ 1     ∀ j, ∀ t
    C6  C_max ≥ end(i)                                    ∀ i

Variables for which t + exec[i][j] > T are not created (a task cannot
start so late that it would run past the horizon).
"""

from __future__ import annotations

import time
from typing import Iterable, List, Optional, Tuple

import gurobipy as gp
from gurobipy import GRB

from problem import Problem


def default_horizon(problem: Problem) -> int:
    """A safe upper bound on the makespan: the worst-case sequential schedule
    where every task picks its slowest PE."""
    return sum(max(row) for row in problem.exec_times)


def solve_ilp_ti(
    problem: Problem,
    edges: Iterable[Tuple[int, int]] = (),
    compat: Optional[List[List[int]]] = None,
    horizon: Optional[int] = None,
    time_limit: Optional[float] = None,
    mip_gap: Optional[float] = None,
    verbose: bool = False,
) -> Tuple[Optional[List[Tuple[int, int]]], Optional[float], dict]:
    """
    Solve the time-indexed makespan model with Gurobi.

    Args:
        problem    : Problem instance (n, m, exec_times, memory, capacity).
        edges      : iterable of (u, v) precedence edges, 0-indexed task ids.
                     If empty, the model reduces to Week 10's Model A
                     (assignment-only, no scheduling).
        compat     : compatibility matrix, compat[i][j] ∈ {0,1}.
                     None means every (task, PE) pair is allowed.
        horizon    : time horizon T. Defaults to sum of per-task max exec_times.
        time_limit : Gurobi TimeLimit in seconds, optional.
        mip_gap    : Gurobi MIPGap, optional.
        verbose    : if True, let Gurobi print to stdout.

    Returns:
        (schedule, makespan, stats)
            schedule  = list of (proc, start) pairs, one per task; or None.
            makespan  = optimal makespan, or None.
            stats     = dict with status, runtime, mip_gap, num_vars,
                        num_constrs, horizon.
    """
    n, m = problem.n, problem.m
    exec_times = problem.exec_times

    if compat is None:
        compat = [[1] * m for _ in range(n)]
    if horizon is None:
        horizon = default_horizon(problem)
    T = horizon

    # Sanity: every task must have at least one compatible PE.
    for i in range(n):
        if not any(compat[i][j] for j in range(m)):
            raise ValueError(f"task {i} has no compatible PE")

    model = gp.Model("model_a_time_indexed")
    if not verbose:
        model.setParam("OutputFlag", 0)
    if time_limit is not None:
        model.setParam("TimeLimit", time_limit)
    if mip_gap is not None:
        model.setParam("MIPGap", mip_gap)

    # ---- variables ----
    # Skip incompatible (i,j); skip start times that would run past T.
    x = {}
    for i in range(n):
        for j in range(m):
            if not compat[i][j]:
                continue
            latest_start = T - exec_times[i][j]
            for t in range(latest_start + 1):
                x[i, j, t] = model.addVar(
                    vtype=GRB.BINARY, name=f"x[{i},{j},{t}]"
                )

    C_max = model.addVar(lb=0.0, vtype=GRB.CONTINUOUS, name="C_max")

    # ---- helper expressions (used by C4, C6) ----
    def start_expr(i: int) -> gp.LinExpr:
        return gp.quicksum(
            t * x[i, j, t]
            for j in range(m) if compat[i][j]
            for t in range(T - exec_times[i][j] + 1)
        )

    def end_expr(i: int) -> gp.LinExpr:
        return gp.quicksum(
            (t + exec_times[i][j]) * x[i, j, t]
            for j in range(m) if compat[i][j]
            for t in range(T - exec_times[i][j] + 1)
        )

    # ---- objective ----
    model.setObjective(C_max, GRB.MINIMIZE)

    # ---- C1: each task starts exactly once ----
    for i in range(n):
        model.addConstr(
            gp.quicksum(
                x[i, j, t]
                for j in range(m) if compat[i][j]
                for t in range(T - exec_times[i][j] + 1)
            ) == 1,
            name=f"assign[{i}]",
        )

    # ---- C2: memory capacity (collapse the t axis) ----
    for j in range(m):
        model.addConstr(
            gp.quicksum(
                problem.memory[i] * x[i, j, t]
                for i in range(n) if compat[i][j]
                for t in range(T - exec_times[i][j] + 1)
            ) <= problem.capacity[j],
            name=f"memory[{j}]",
        )

    # ---- C4: precedence,  end(u) ≤ start(v) ----
    for (u, v) in edges:
        model.addConstr(end_expr(u) <= start_expr(v), name=f"prec[{u},{v}]")

    # ---- C5: at most one task active per (PE, time slot) ----
    # Task i started at τ on j is "active at time t" iff τ ≤ t < τ + exec[i][j].
    for j in range(m):
        for t in range(T):
            active = [
                x[i, j, tau]
                for i in range(n) if compat[i][j]
                for tau in range(
                    max(0, t - exec_times[i][j] + 1),
                    min(t, T - exec_times[i][j]) + 1,
                )
            ]
            if active:
                model.addConstr(
                    gp.quicksum(active) <= 1, name=f"noverlap[{j},{t}]"
                )

    # ---- C6: makespan dominates every task end ----
    for i in range(n):
        model.addConstr(C_max >= end_expr(i), name=f"makespan[{i}]")

    model.update()

    t0 = time.perf_counter()
    model.optimize()
    runtime = time.perf_counter() - t0

    status_name = {
        GRB.OPTIMAL: "OPTIMAL",
        GRB.TIME_LIMIT: "TIME_LIMIT",
        GRB.INFEASIBLE: "INFEASIBLE",
        GRB.SUBOPTIMAL: "SUBOPTIMAL",
        GRB.INTERRUPTED: "INTERRUPTED",
    }.get(model.Status, f"STATUS_{model.Status}")

    stats = {
        "status": status_name,
        "runtime": runtime,
        "mip_gap": model.MIPGap if model.SolCount > 0 else float("inf"),
        "num_vars": model.NumVars,
        "num_constrs": model.NumConstrs,
        "horizon": T,
    }

    if model.SolCount == 0:
        return None, None, stats

    # ---- extract schedule: each task has exactly one x[i,j,t] = 1 ----
    schedule: List[Tuple[int, int]] = [(-1, -1)] * n
    for (i, j, t), var in x.items():
        if var.X > 0.5:
            schedule[i] = (j, t)

    assert all(p[0] >= 0 for p in schedule), "every task must be scheduled"
    return schedule, float(model.ObjVal), stats


# ---------------------------------------------------------------------------
# Demo on the Week 11 worked example.
#   n=6, m=3, edges as in the LaTeX, T3 and T5 are FFTs (P3 only).
#   Expected: C_max* = 11 (matches the critical-path lower bound).
# ---------------------------------------------------------------------------

def _demo() -> None:
    n, m = 6, 3
    durations = [2, 4, 2, 3, 2, 2]               # t_v from the LaTeX
    exec_times = [[d] * m for d in durations]    # homogeneous timing
    memory   = [1, 1, 1, 1, 1, 1]
    capacity = [9, 9, 9]                          # memory non-binding for demo

    # E = {(1,2),(1,3),(2,4),(3,5),(4,6),(5,6)} in the LaTeX, 0-indexed here.
    edges = [(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 5)]

    # T3 (idx 2) and T5 (idx 4) are FFT ops — only P3 (idx 2) can run them.
    compat = [[1, 1, 1] for _ in range(n)]
    compat[1] = [1, 1, 0]
    compat[2] = [0, 0, 1]
    compat[3] = [1, 1, 0]
    compat[4] = [0, 0, 1]

    problem = Problem(
        n=n, m=m, exec_times=exec_times,
        memory=memory, capacity=capacity,
        name="Running DAG (n=6, FFT compat)",
    )

    schedule, c_max, stats = solve_ilp_ti(problem, edges=edges, compat=compat)

    print(f"status   : {stats['status']}")
    print(f"horizon T: {stats['horizon']}")
    print(f"vars     : {stats['num_vars']}")
    print(f"constrs  : {stats['num_constrs']}")
    print(f"runtime  : {stats['runtime'] * 1000:.2f} ms")
    print(f"C_max*   : {c_max:.0f}")
    print()
    print(" task | PE  | start | end")
    print("------+-----+-------+----")
    for i, (j, t) in enumerate(schedule):
        end_t = t + exec_times[i][j]
        print(f"  T{i+1}  |  P{j+1} |  {t:>3}  |  {end_t:>2}")


if __name__ == "__main__":
    _demo()
