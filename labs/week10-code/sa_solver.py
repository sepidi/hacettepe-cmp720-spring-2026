"""
CMP720 — Simulated Annealing for makespan minimization.

Encoding: y[i] ∈ {0,…,m-1}  — the integer that Model B failed to exploit.

Algorithm (matches slides 4–8):
    1. Start from a greedy feasible initial solution.
    2. For each temperature step, propose `inner_iter` neighbors.
       A neighbor is constructed by picking one task uniformly at random
       and reassigning it to a different processor (also uniformly random).
    3. Metropolis acceptance: always accept downhill; accept uphill with
       probability exp(-ΔE / T).
    4. Cool geometrically: T ← α · T.

Two feasibility strategies are supported:
    - 'reject' : discard infeasible neighbors (matches the hand-trace).
    - 'penalty': include memory violations in the objective, weighted by λ.
"""

from __future__ import annotations

import math
import random
import time
from dataclasses import dataclass, field
from typing import List, Literal, Optional, Tuple

from problem import Problem


# ---------------------------------------------------------------------------
# SA configuration and result
# ---------------------------------------------------------------------------

@dataclass
class SAConfig:
    T0: float = 10.0
    alpha: float = 0.95
    inner_iter: int = 50            # moves per temperature
    n_temperatures: int = 400       # total temperature steps
    strategy: Literal["reject", "penalty"] = "reject"
    penalty_lambda: float = 100.0   # only used when strategy == "penalty"
    use_swap_moves: bool = False    # if True, mix in swap moves
    p_swap: float = 0.3             # probability of using swap vs reassignment
    seed: Optional[int] = None

    @property
    def total_iterations(self) -> int:
        return self.inner_iter * self.n_temperatures


@dataclass
class SAResult:
    assignment: List[int]
    makespan: float
    feasible: bool
    runtime: float
    history: List[float] = field(default_factory=list)  # best-so-far after each inner sweep
    stats: dict = field(default_factory=dict)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def greedy_initial(problem: Problem, rng: random.Random) -> List[int]:
    """
    Greedy feasible initial assignment: iterate tasks in a randomized order,
    assign each to the processor that minimizes the resulting max load AND
    respects memory capacity. Falls back to any feasible processor if needed.
    """
    n, m = problem.n, problem.m
    order = list(range(n))
    rng.shuffle(order)

    y = [-1] * n
    load = [0] * m
    mem = [0] * m

    for i in order:
        best_j = -1
        best_score = math.inf
        for j in range(m):
            if mem[j] + problem.memory[i] > problem.capacity[j]:
                continue
            new_load = load[j] + problem.exec_times[i][j]
            new_max = max(max(load[:j] + load[j+1:], default=0), new_load)
            if new_max < best_score:
                best_score = new_max
                best_j = j
        if best_j == -1:
            raise RuntimeError(
                f"Greedy initial failed: task {i} does not fit in any processor. "
                "Instance may be infeasible."
            )
        y[i] = best_j
        load[best_j] += problem.exec_times[i][best_j]
        mem[best_j] += problem.memory[i]

    return y


def neighbor(y: List[int], m: int, rng: random.Random) -> Tuple[List[int], int, int, int]:
    """Reassign one random task to a different random processor (the basic move)."""
    n = len(y)
    i = rng.randrange(n)
    j_old = y[i]
    # uniform over {0,…,m-1} \ {j_old}
    j_new = rng.randrange(m - 1)
    if j_new >= j_old:
        j_new += 1
    y_new = y.copy()
    y_new[i] = j_new
    return y_new, i, j_old, j_new


def neighbor_mixed(y: List[int], m: int, rng: random.Random, p_swap: float = 0.3) -> List[int]:
    """
    Mixed neighborhood: reassign (default) or swap two tasks on different processors.
    Swap moves help escape local optima where individual reassignments don't improve.
    """
    if rng.random() < p_swap:
        n = len(y)
        # Find two tasks on different processors. If all tasks on one processor,
        # fall back to reassignment.
        for _ in range(10):
            i1, i2 = rng.sample(range(n), 2)
            if y[i1] != y[i2]:
                y_new = y.copy()
                y_new[i1], y_new[i2] = y[i2], y[i1]
                return y_new
    # reassignment fallback
    y_new, _, _, _ = neighbor(y, m, rng)
    return y_new


def violation(problem: Problem, y: List[int]) -> int:
    """Total amount by which memory capacity is exceeded (0 if feasible)."""
    use = problem.memory_use(y)
    return sum(max(0, use[j] - problem.capacity[j]) for j in range(problem.m))


# ---------------------------------------------------------------------------
# Main solver
# ---------------------------------------------------------------------------

def solve_sa(problem: Problem, config: Optional[SAConfig] = None) -> SAResult:
    if config is None:
        config = SAConfig()
    rng = random.Random(config.seed)

    t0 = time.perf_counter()

    # ---- initial solution ----
    y = greedy_initial(problem, rng)
    cur_make = problem.makespan(y)
    cur_viol = violation(problem, y) if config.strategy == "penalty" else 0
    cur_energy = cur_make + config.penalty_lambda * cur_viol

    best_y = y.copy()
    best_make = cur_make
    best_feasible = problem.is_feasible(y)

    T = config.T0
    history = [best_make]
    moves_proposed = 0
    moves_accepted = 0
    moves_rejected_infeasible = 0

    # ---- outer loop over temperatures ----
    for step in range(config.n_temperatures):
        for _ in range(config.inner_iter):
            if config.use_swap_moves:
                y_new = neighbor_mixed(y, problem.m, rng, p_swap=config.p_swap)
            else:
                y_new, _, _, _ = neighbor(y, problem.m, rng)
            moves_proposed += 1

            if config.strategy == "reject":
                if not problem.is_feasible(y_new):
                    moves_rejected_infeasible += 1
                    continue
                new_make = problem.makespan(y_new)
                new_energy = new_make
                new_viol = 0
            else:  # penalty
                new_make = problem.makespan(y_new)
                new_viol = violation(problem, y_new)
                new_energy = new_make + config.penalty_lambda * new_viol

            delta = new_energy - cur_energy
            if delta <= 0 or rng.random() < math.exp(-delta / T):
                y = y_new
                cur_make = new_make
                cur_viol = new_viol
                cur_energy = new_energy
                moves_accepted += 1

                if cur_viol == 0 and cur_make < best_make:
                    best_y = y.copy()
                    best_make = cur_make
                    best_feasible = True

        history.append(best_make)
        T *= config.alpha

    runtime = time.perf_counter() - t0

    return SAResult(
        assignment=best_y,
        makespan=best_make,
        feasible=best_feasible,
        runtime=runtime,
        history=history,
        stats={
            "moves_proposed": moves_proposed,
            "moves_accepted": moves_accepted,
            "moves_rejected_infeasible": moves_rejected_infeasible,
            "acceptance_rate": moves_accepted / max(1, moves_proposed),
            "final_T": T,
            "strategy": config.strategy,
            "config": config,
        },
    )


# ---------------------------------------------------------------------------
# Multi-restart wrapper — gives a fairer picture of a stochastic algorithm.
# ---------------------------------------------------------------------------

def solve_sa_multirestart(
    problem: Problem,
    n_restarts: int = 10,
    base_config: Optional[SAConfig] = None,
) -> Tuple[SAResult, List[SAResult]]:
    """
    Run SA from `n_restarts` different seeds. Returns (best_result, all_results).
    """
    if base_config is None:
        base_config = SAConfig()
    runs: List[SAResult] = []
    best: Optional[SAResult] = None
    for k in range(n_restarts):
        cfg = SAConfig(**{**base_config.__dict__, "seed": (base_config.seed or 0) + k})
        r = solve_sa(problem, cfg)
        runs.append(r)
        if best is None or (r.feasible and r.makespan < best.makespan):
            best = r
    assert best is not None
    return best, runs
