"""
CMP720 — ILP Model A (binary blueprint) for makespan minimization.

This is the formulation from slides 14–18 of the Weeks 8+ deck:

    variables:
        x[i,j] ∈ {0,1}   — 1 if task i runs on processor j
        T_max  ≥ 0       — auxiliary continuous, the makespan

    minimize   T_max

    subject to:
        C1  Σ_j x[i,j] = 1                    ∀i     (each task assigned once)
        C2  T_max ≥ Σ_i t[i,j] · x[i,j]       ∀j     (makespan linking)
        C3  Σ_i mem[i] · x[i,j] ≤ cap[j]      ∀j     (memory capacity)

Returns the optimal assignment as a list y where y[i] is the processor
index (0-indexed) of task i.
"""

from __future__ import annotations

import time
from typing import Optional, Tuple, List

import gurobipy as gp
from gurobipy import GRB

from problem import Problem


def solve_ilp(
    problem: Problem,
    time_limit: Optional[float] = None,
    mip_gap: Optional[float] = None,
    verbose: bool = False,
) -> Tuple[Optional[List[int]], Optional[float], dict]:
    """
    Solve Model A with Gurobi.

    Returns (assignment, objective_value, stats) where:
        assignment     = list of processor indices, one per task, or None if no
                         feasible solution found within time_limit.
        objective_value = the makespan of `assignment`, or None.
        stats = {
            'status': str,         # e.g. 'OPTIMAL', 'TIME_LIMIT'
            'runtime': float,      # wall-clock seconds
            'mip_gap': float,      # final gap at termination
            'num_vars': int,       # model size
            'num_constrs': int,
        }
    """
    n, m = problem.n, problem.m

    model = gp.Model("model_a_makespan")
    if not verbose:
        model.setParam("OutputFlag", 0)
    if time_limit is not None:
        model.setParam("TimeLimit", time_limit)
    if mip_gap is not None:
        model.setParam("MIPGap", mip_gap)

    # ---- variables ----
    x = model.addVars(n, m, vtype=GRB.BINARY, name="x")
    T_max = model.addVar(lb=0.0, vtype=GRB.CONTINUOUS, name="T_max")

    # ---- objective ----
    model.setObjective(T_max, GRB.MINIMIZE)

    # ---- C1: assignment ----
    for i in range(n):
        model.addConstr(
            gp.quicksum(x[i, j] for j in range(m)) == 1,
            name=f"assign[{i}]",
        )

    # ---- C2: makespan linking ----
    for j in range(m):
        model.addConstr(
            T_max >= gp.quicksum(problem.exec_times[i][j] * x[i, j] for i in range(n)),
            name=f"makespan[{j}]",
        )

    # ---- C3: memory capacity ----
    for j in range(m):
        model.addConstr(
            gp.quicksum(problem.memory[i] * x[i, j] for i in range(n))
            <= problem.capacity[j],
            name=f"memory[{j}]",
        )

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
    }

    if model.SolCount == 0:
        return None, None, stats

    # Extract assignment. Each row has exactly one 1 by C1.
    y: List[int] = []
    for i in range(n):
        for j in range(m):
            if x[i, j].X > 0.5:
                y.append(j)
                break

    assert len(y) == n, "x variables should form a valid assignment"

    return y, float(model.ObjVal), stats
