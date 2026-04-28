"""
CMP720 — Week 11: Genetic Algorithm for the time-indexed makespan
                 problem with precedence and PE compatibility.

Same problem as the Week 11 ILP — but instead of branch-and-bound on
n·k·T binaries, we evolve a population of candidate schedules.

Encoding (two chromosomes per individual)
-----------------------------------------
    y   : List[int]    — y[i] = PE assigned to task i, with compat[i][y[i]] = 1
    pri : List[float]  — pri[i] ∈ [0,1], lower means schedule earlier

The (y, pri) pair is decoded into an actual schedule by a *serial
schedule generation scheme* (SSGS):

    1.  Mark all tasks unscheduled.  pe_avail[j] = 0 for every PE.
    2.  Repeat until every task is scheduled:
          a. ready = {i : every predecessor of i is scheduled}
          b. pick i* ∈ ready with the smallest pri[i*]   (random-key tiebreaker)
          c. j = y[i*]
             start = max(max end-time of predecessors, pe_avail[j])
             end   = start + exec_times[i*][j]
             record (start, end), update pe_avail[j] = end.

This decoder *always* respects precedence and PE non-overlap, so every
chromosome maps to a feasible schedule with respect to (C4) and (C5) of
the Week 11 model. Memory capacity (C2) is enforced softly via a
penalty term in the fitness; standard GA practice for problems where a
clean repair operator is awkward.

Operators
---------
    selection : tournament-of-k (default k = 3, minimisation)
    crossover : uniform on y, BLX-α blend on pri
    mutation  : per-gene PE re-roll on y; Gaussian perturbation on pri
    elitism   : top-E individuals copied unchanged each generation
"""

from __future__ import annotations

import random
import time
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

from problem import Problem


# ---------------------------------------------------------------------------
# Data classes
# ---------------------------------------------------------------------------

@dataclass
class Chromosome:
    y: List[int]            # PE per task, length n
    pri: List[float]        # priority per task, length n


@dataclass
class GAConfig:
    # population & loop
    pop_size: int = 60
    n_generations: int = 200
    elitism: int = 2
    tournament_k: int = 3

    # operators
    p_crossover: float = 0.9
    blx_alpha: float = 0.5          # BLX-α range expansion on the pri vector
    p_mut_y: float = 0.1            # per-gene PE re-roll probability
    p_mut_pri: float = 0.2          # per-gene priority perturbation probability
    pri_sigma: float = 0.15         # std-dev of Gaussian perturbation on pri

    # problem extension (passes through to decoder/feasibility)
    edges: List[Tuple[int, int]] = field(default_factory=list)
    compat: Optional[List[List[int]]] = None

    # constraint handling
    penalty_weight: float = 1000.0  # added to fitness per unit of memory overuse

    seed: Optional[int] = None


@dataclass
class GARun:
    best: Chromosome
    schedule: List[Tuple[int, int]]   # (PE, start) per task — same format as ILP
    makespan: float
    feasible: bool                    # memory feasible?
    history: List[float]              # best feasible makespan per generation (inf if none)
    runtime: float
    n_evaluations: int


# ---------------------------------------------------------------------------
# Decoder — Serial Schedule Generation Scheme (SSGS)
# ---------------------------------------------------------------------------

def decode(
    chromo: Chromosome,
    problem: Problem,
    edges: List[Tuple[int, int]],
) -> Tuple[List[Tuple[int, int]], int]:
    """
    Convert (y, pri) into a concrete schedule that respects precedence
    and per-PE non-overlap. Returns (schedule, makespan) where
    schedule[i] = (PE, start) for task i.
    """
    n = problem.n
    y, pri = chromo.y, chromo.pri

    # Build predecessor lists once per call. Cheap for small n.
    preds: List[List[int]] = [[] for _ in range(n)]
    for u, v in edges:
        preds[v].append(u)

    start = [0] * n
    end = [0] * n
    pe_avail = [0] * problem.m
    scheduled = [False] * n

    for _ in range(n):
        # ready = tasks whose predecessors are all scheduled, themselves unscheduled
        ready = [
            i for i in range(n)
            if not scheduled[i] and all(scheduled[p] for p in preds[i])
        ]
        if not ready:
            # Should never happen for a DAG with full coverage; defensive.
            raise ValueError("decoder stalled — is the precedence graph acyclic?")

        i = min(ready, key=lambda k: pri[k])
        j = y[i]

        pred_end = max((end[p] for p in preds[i]), default=0)
        start[i] = max(pred_end, pe_avail[j])
        end[i] = start[i] + problem.exec_times[i][j]
        pe_avail[j] = end[i]
        scheduled[i] = True

    schedule = list(zip(y, start))
    return schedule, max(end)


# ---------------------------------------------------------------------------
# Fitness
# ---------------------------------------------------------------------------

def evaluate(
    chromo: Chromosome,
    problem: Problem,
    edges: List[Tuple[int, int]],
    penalty_weight: float,
) -> Tuple[float, int, bool]:
    """
    Returns (penalised_fitness, makespan, memory_feasible).
    Lower fitness is better.
    """
    _, makespan = decode(chromo, problem, edges)
    mem_use = problem.memory_use(chromo.y)
    overflow = sum(max(0, mem_use[j] - problem.capacity[j]) for j in range(problem.m))
    feasible = overflow == 0
    fitness = makespan + penalty_weight * overflow
    return fitness, makespan, feasible


# ---------------------------------------------------------------------------
# Operators
# ---------------------------------------------------------------------------

def _compatible_pes(i: int, compat: List[List[int]], m: int) -> List[int]:
    return [j for j in range(m) if compat[i][j]]


def random_chromosome(
    problem: Problem,
    compat: List[List[int]],
    rng: random.Random,
) -> Chromosome:
    y = [rng.choice(_compatible_pes(i, compat, problem.m)) for i in range(problem.n)]
    pri = [rng.random() for _ in range(problem.n)]
    return Chromosome(y, pri)


def crossover(
    p1: Chromosome,
    p2: Chromosome,
    blx_alpha: float,
    rng: random.Random,
) -> Chromosome:
    """Uniform crossover on y, BLX-α blend on pri."""
    n = len(p1.y)
    y = [p1.y[i] if rng.random() < 0.5 else p2.y[i] for i in range(n)]

    pri = []
    for i in range(n):
        a, b = p1.pri[i], p2.pri[i]
        lo, hi = (a, b) if a <= b else (b, a)
        d = hi - lo
        # Sample uniformly from [lo - α·d, hi + α·d], clipped to [0,1].
        x = rng.uniform(lo - blx_alpha * d, hi + blx_alpha * d)
        pri.append(min(1.0, max(0.0, x)))

    return Chromosome(y, pri)


def mutate(
    c: Chromosome,
    problem: Problem,
    compat: List[List[int]],
    cfg: GAConfig,
    rng: random.Random,
) -> Chromosome:
    y = list(c.y)
    pri = list(c.pri)
    for i in range(problem.n):
        if rng.random() < cfg.p_mut_y:
            options = _compatible_pes(i, compat, problem.m)
            # Avoid no-op mutation when there are alternatives.
            if len(options) > 1:
                y[i] = rng.choice([j for j in options if j != y[i]])
            else:
                y[i] = options[0]
        if rng.random() < cfg.p_mut_pri:
            pri[i] = min(1.0, max(0.0, pri[i] + rng.gauss(0.0, cfg.pri_sigma)))
    return Chromosome(y, pri)


def tournament_select(
    pop: List[Chromosome],
    fitnesses: List[float],
    k: int,
    rng: random.Random,
) -> Chromosome:
    contestants = rng.sample(range(len(pop)), k)
    winner = min(contestants, key=lambda idx: fitnesses[idx])
    return pop[winner]


# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def solve_ga(problem: Problem, config: Optional[GAConfig] = None) -> GARun:
    cfg = config or GAConfig()
    rng = random.Random(cfg.seed)

    edges = list(cfg.edges)
    compat = cfg.compat or [[1] * problem.m for _ in range(problem.n)]

    # Sanity: every task must have at least one compatible PE.
    for i in range(problem.n):
        if not any(compat[i][j] for j in range(problem.m)):
            raise ValueError(f"task {i} has no compatible PE")

    # ---- initial population ----
    pop = [random_chromosome(problem, compat, rng) for _ in range(cfg.pop_size)]
    evals = [evaluate(c, problem, edges, cfg.penalty_weight) for c in pop]
    fitnesses = [e[0] for e in evals]
    n_evaluations = len(pop)

    best_idx = min(range(len(pop)), key=lambda i: fitnesses[i])
    best = pop[best_idx]
    best_fit, best_makespan, best_feasible = evals[best_idx]

    history: List[float] = []
    t0 = time.perf_counter()

    for _ in range(cfg.n_generations):
        # ---- new population, elitism first ----
        order = sorted(range(len(pop)), key=lambda i: fitnesses[i])
        new_pop: List[Chromosome] = [pop[order[i]] for i in range(cfg.elitism)]

        while len(new_pop) < cfg.pop_size:
            p1 = tournament_select(pop, fitnesses, cfg.tournament_k, rng)
            p2 = tournament_select(pop, fitnesses, cfg.tournament_k, rng)
            if rng.random() < cfg.p_crossover:
                child = crossover(p1, p2, cfg.blx_alpha, rng)
            else:
                child = Chromosome(list(p1.y), list(p1.pri))
            child = mutate(child, problem, compat, cfg, rng)
            new_pop.append(child)

        pop = new_pop
        evals = [evaluate(c, problem, edges, cfg.penalty_weight) for c in pop]
        fitnesses = [e[0] for e in evals]
        n_evaluations += len(pop)

        # ---- track best-so-far (over the run, not just this generation) ----
        gen_best = min(range(len(pop)), key=lambda i: fitnesses[i])
        if fitnesses[gen_best] < best_fit:
            best = pop[gen_best]
            best_fit, best_makespan, best_feasible = evals[gen_best]

        history.append(best_makespan if best_feasible else float("inf"))

    runtime = time.perf_counter() - t0

    schedule, makespan = decode(best, problem, edges)
    feasible = problem.is_feasible(best.y)

    return GARun(
        best=best,
        schedule=schedule,
        makespan=float(makespan),
        feasible=feasible,
        history=history,
        runtime=runtime,
        n_evaluations=n_evaluations,
    )


def solve_ga_multirestart(
    problem: Problem,
    n_restarts: int = 10,
    base_config: Optional[GAConfig] = None,
) -> Tuple[GARun, List[GARun]]:
    """Multi-restart wrapper, mirroring the SA solver's interface."""
    cfg = base_config or GAConfig()
    runs: List[GARun] = []
    for r in range(n_restarts):
        seed = (cfg.seed if cfg.seed is not None else 0) + r
        run_cfg = GAConfig(**{**cfg.__dict__, "seed": seed})
        runs.append(solve_ga(problem, run_cfg))
    best = min(
        (r for r in runs if r.feasible),
        key=lambda r: r.makespan,
        default=min(runs, key=lambda r: r.makespan),
    )
    return best, runs


# ---------------------------------------------------------------------------
# Pretty-printing
# ---------------------------------------------------------------------------

def format_schedule(
    problem: Problem,
    schedule: List[Tuple[int, int]],
) -> str:
    """Compact (task, PE, start, end) table — fine for n ≤ 20."""
    if problem.n > 20:
        return f"  (schedule of length {problem.n}; omitted)"
    lines = [" task | PE  | start | end ", "------+-----+-------+-----"]
    for i, (j, t) in enumerate(schedule):
        end_t = t + problem.exec_times[i][j]
        lines.append(f"  T{i+1:<2} |  P{j+1} |  {t:>3}  | {end_t:>3} ")
    makespan = max(t + problem.exec_times[i][j] for i, (j, t) in enumerate(schedule))
    lines.append(f"  makespan = {makespan}")
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Demos
# ---------------------------------------------------------------------------

def _demo_small() -> None:
    """Week-10 reduction: no edges, no compat → match the ILP optimum of 7."""
    from problem import small_benchmark

    problem = small_benchmark()
    cfg = GAConfig(
        pop_size=60, n_generations=150,
        p_crossover=0.9, p_mut_y=0.15, p_mut_pri=0.2,
        seed=1,
    )
    best, runs = solve_ga_multirestart(problem, n_restarts=20, base_config=cfg)

    feasible = [r for r in runs if r.feasible]
    makespans = [r.makespan for r in feasible]
    total_time = sum(r.runtime for r in runs)

    print("=" * 64)
    print("Demo 1: Small benchmark (n=6, m=3, no precedence)")
    print("=" * 64)
    print(f"  best makespan (over {len(runs)} runs) : {best.makespan:.0f}")
    print(f"  mean / worst                          : "
          f"{sum(makespans)/len(makespans):.2f} / {max(makespans):.0f}")
    print(f"  hit-optimum (=7) rate                 : "
          f"{sum(1 for m in makespans if m == 7)}/{len(makespans)}")
    print(f"  total wall time (all runs)            : {total_time*1000:.1f} ms")
    print(format_schedule(problem, best.schedule))


def _demo_dag() -> None:
    """Week-11 worked example: 6-task DAG with FFT compat. Expect C_max* = 11."""
    n, m = 6, 3
    durations = [2, 4, 2, 3, 2, 2]
    exec_times = [[d] * m for d in durations]
    memory = [1, 1, 1, 1, 1, 1]
    capacity = [9, 9, 9]

    edges = [(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 5)]
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

    cfg = GAConfig(
        pop_size=60, n_generations=150,
        edges=edges, compat=compat,
        p_crossover=0.9, p_mut_y=0.15, p_mut_pri=0.2,
        seed=1,
    )
    best, runs = solve_ga_multirestart(problem, n_restarts=20, base_config=cfg)

    feasible = [r for r in runs if r.feasible]
    makespans = [r.makespan for r in feasible]
    total_time = sum(r.runtime for r in runs)

    print()
    print("=" * 64)
    print("Demo 2: 6-task DAG with FFT compatibility (T3, T5 → P3 only)")
    print("=" * 64)
    print(f"  best makespan (over {len(runs)} runs) : {best.makespan:.0f}")
    print(f"  mean / worst                          : "
          f"{sum(makespans)/len(makespans):.2f} / {max(makespans):.0f}")
    print(f"  hit-optimum (=11) rate                : "
          f"{sum(1 for m in makespans if m == 11)}/{len(makespans)}")
    print(f"  total wall time (all runs)            : {total_time*1000:.1f} ms")
    print(format_schedule(problem, best.schedule))


if __name__ == "__main__":
    _demo_small()
    _demo_dag()
