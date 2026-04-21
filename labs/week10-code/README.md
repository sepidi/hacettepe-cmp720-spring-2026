# CMP720 — ILP vs Simulated Annealing for Makespan Scheduling

Reference implementations accompanying the Week 10 lecture material on the
heterogeneous embedded task scheduler. Two solvers over the same `Problem`
class:

- `ilp_solver.py` — Model A from the slides, implemented in Gurobi.
- `sa_solver.py`  — Simulated Annealing over the integer-vector encoding
  `y(i) ∈ {0,…,m-1}`, with reject-infeasible and penalty strategies.

Both solvers consume the same `problem.Problem` object and return the same
shape of answer: an assignment vector and a makespan. Students can substitute
one for the other in an outer experiment loop.

## Files

| File | Purpose |
|---|---|
| `problem.py` | `Problem` class and the `small_benchmark()` instance from the slides. |
| `ilp_solver.py` | `solve_ilp(problem, time_limit, mip_gap)` — Model A in Gurobi. |
| `sa_solver.py` | `solve_sa(problem, config)` and `solve_sa_multirestart(...)` — the SA loop matching slides 4–8. |
| `run_small.py` | Demonstrates both solvers on the 6×3 slide instance. |

## Dependencies

```bash
pip install gurobipy
```

`gurobipy` installed via pip ships with a restricted license (2,000 vars /
constraints), which is more than enough for the slide instance.

## Reproducing the small benchmark

```bash
python3 run_small.py
```

Expected output (abridged):

```
[ILP — Model A (Gurobi)]
  makespan  : 7
  runtime   : 2.36 ms
  y = [1, 2, 3, 1, 3, 2]   (1-indexed)
    P1: tasks [1, 4]  exec=7  mem=6/8
    P2: tasks [2, 6]  exec=6  mem=6/7
    P3: tasks [3, 5]  exec=7  mem=3/6
  makespan = 7

[SA — 30 restarts, reject-infeasible]
  best makespan (over 30 runs): 7
  hit-optimum rate            : 29/30
  per-run wall time (mean)    : 27.33 ms
  y = [1, 2, 3, 1, 3, 2]   (1-indexed)
```

The best assignment SA reaches is exactly the state the hand-trace ends in
the lecture notes: `y = [1, 2, 3, 1, 3, 2]`, makespan = 7.

## Why we're still teaching SA

On **pure Model A** instances (independent tasks, heterogeneous execution
times, memory constraints), Gurobi's ILP is very hard to beat. Every
constraint in Model A is linear, the LP relaxation of this problem class
is tight, and modern branch-and-bound closes the gap quickly — often in
milliseconds on instances with thousands of binary variables.

Students should not walk away from this demonstration thinking SA is a
free win. On this problem class, it isn't. Metaheuristics earn their keep
when one or more of the following becomes true:

1. **The instance scales beyond what branch-and-bound can finish** — either
   because the LP relaxation weakens, because memory blows up, or because
   the license limit kicks in.
2. **Constraints stop being linear.** The moment a constraint is a check
   rather than an inequality (e.g. "these three tasks collectively should
   not co-locate" or "this task's deadline depends on another's finish
   time"), the ILP requires indicator variables, big-M constants, or
   auxiliary linking — and its LP relaxation weakens. SA just evaluates
   the check.
3. **Precedence and timing enter the model.** Once we move from
   "independent tasks with makespan" to "scheduling with precedence,
   communication delays, or deadlines," the gap narrows dramatically and
   metaheuristics become competitive — sometimes dominant — at realistic
   problem sizes.

Exercise "If tasks had precedence constraints…" is the setup for 
revisiting this comparison under conditions where SA has room to win.

## Design notes

**Shared `Problem` class.** Both solvers read the same instance. This makes
it trivial for students to swap solvers or add a third (e.g., a greedy
list scheduler, a genetic algorithm).

**SA strategies.** `SAConfig.strategy` is either `"reject"` (discard
infeasible neighbors — matches the hand-trace on slide 11) or `"penalty"`
(include memory violations in the energy with weight `penalty_lambda`).

**Neighborhoods.** The default neighborhood reassigns one random task to a
different processor. Setting `use_swap_moves=True` mixes in swap moves —
useful when the feasible region has narrow corridors, which happens on
tight memory budgets.

**Reproducibility.** Every benchmark and SA run takes a seed. Students
sharing results should report seeds.
