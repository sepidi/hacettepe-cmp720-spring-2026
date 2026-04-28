# CMP720 — Scheduling Lab: ILP vs GA

This package contains two solvers for the **time-indexed makespan
scheduling problem with precedence and PE compatibility** (Weeks 11–12),
plus a small test suite for comparing them.

You will run the same problem instances through both solvers and reason
about their behavior.


## 1. What's in the box

| File              | What it is                                                     |
|-------------------|----------------------------------------------------------------|
| `problem.py`      | The `Problem` dataclass and the small 6×3 benchmark.           |
| `ilp_solver_ti.py`| Time-indexed ILP solver (Gurobi). Extends Week 10's Model A.   |
| `ga_solver.py`    | Genetic Algorithm solver. Two-chromosome encoding + SSGS decoder. |
| `test_suite.py`   | Runs both solvers on three benchmarks and prints a summary.    |

Read the docstrings at the top of each file before running anything —
they're short and tell you exactly which constraints map to which lines
of code.


## 2. Setup

You need Python 3.9+ and Gurobi:

```bash
pip install gurobipy
```

A free academic Gurobi license works fine for everything in this lab. If
you don't have one yet, get it from
<https://www.gurobi.com/academia/academic-program-and-licenses/>. The
restricted/trial license that ships with `gurobipy` is also enough for
all instances here (largest is n=10).


## 3. The problem inputs at a glance

A scheduling instance is fully described by **four pieces**:

1. **A `Problem`** — tasks, PEs, exec_times, memory, capacity (Week 10's input).
2. **`edges`** — list of `(u, v)` precedence pairs, **0-indexed**.
   `(u, v)` means "task u must finish before task v starts".
3. **`compat`** — `compat[i][j] ∈ {0, 1}`. `1` if task i is allowed on PE j.
   Pass `None` for "every PE allowed for every task".
4. **`horizon`** *(ILP only, optional)* — upper bound T on the makespan.
   Defaults to `sum_i max_j exec_times[i][j]`, which is always safe.

Pay attention to the indexing convention. The slides number tasks T1…T6
(1-indexed); the code uses 0…n-1. So the slide edge `T1 → T2` becomes
`(0, 1)` in `edges`.


## 4. Running each solver standalone

### 4.1 ILP

```python
from problem import Problem, small_benchmark
from ilp_solver_ti import solve_ilp_ti

problem = small_benchmark()           # n=6, m=3, no precedence
schedule, makespan, stats = solve_ilp_ti(problem)
print(f"makespan = {makespan}  ({stats['runtime']*1000:.1f} ms)")
```

To turn this into the **Week 11 worked example**, just pass `edges` and
`compat`:

```python
edges = [(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 5)]
compat = [[1,1,1]] * 6
compat[2] = [0, 0, 1]   # T3 → P3 only (FFT)
compat[4] = [0, 0, 1]   # T5 → P3 only (FFT)

schedule, makespan, stats = solve_ilp_ti(problem, edges=edges, compat=compat)
```

`schedule[i] = (PE, start_time)` for task i. `stats['num_vars']` and
`stats['num_constrs']` tell you how big the model got — useful when you
push n higher.

You can also just run the built-in demo:

```bash
python ilp_solver_ti.py
```

### 4.2 GA

The GA takes the same inputs through a `GAConfig`:

```python
from ga_solver import GAConfig, solve_ga

cfg = GAConfig(
    pop_size=60, n_generations=150,
    edges=edges, compat=compat,
    p_crossover=0.9, p_mut_y=0.15, p_mut_pri=0.2,
    seed=1,
)
result = solve_ga(problem, cfg)
print(f"makespan = {result.makespan}, feasible = {result.feasible}")
```

For multi-restart (recommended — GA is stochastic, single runs lie):

```python
from ga_solver import solve_ga_multirestart
best, all_runs = solve_ga_multirestart(problem, n_restarts=10, base_config=cfg)
```

Built-in demos:

```bash
python ga_solver.py
```


## 5. Running the test suite

```bash
python test_suite.py
```

This runs **three benchmarks** of increasing difficulty:

| # | Instance                         | What it tests                          |
|---|----------------------------------|----------------------------------------|
| 1 | Tiny, no precedence (n=6)        | Sanity — must reduce to Week 10's optimum (7). |
| 2 | DAG + FFT compat (n=6)           | Week 11 worked example. Optimum = 11.  |
| 3 | Medium DAG (n=10)                | First instance where GA's runtime starts to matter. |

The runner prints per-instance details and a summary table:

```
Instance                         |  ILP*   ILP time | GA best GA mean    GA time |    gap    hits
-------------------------------------------------------------------------------------------------
1. Tiny, no precedence (n=6, m=3) |     7    30.9 ms |       7    7.00   296.3 ms |   0.0% 10/10
2. DAG + FFT compat (n=6, m=3)   |    11     3.1 ms |      11   11.00   291.0 ms |   0.0% 10/10
3. Medium DAG (n=10, m=3)        |    18    49.3 ms |      18   18.00   491.0 ms |   0.0% 10/10
```

What each column means:

- **ILP\*** — Gurobi-proven optimum (or best-found if it hit the time limit).
- **ILP time** — wall time for one ILP solve.
- **GA best / mean** — best and mean makespan over the GA restarts.
- **GA time** — *per-restart* wall time. Multiply by `--restarts` for total.
- **gap** — `(GA_best − ILP*) / ILP*`. Zero means the GA reached the optimum.
- **hits** — how many restarts hit the ILP optimum.

Useful flags:

```bash
python test_suite.py --restarts 30          # tighter GA statistics
python test_suite.py --ilp-time-limit 10    # cap the ILP at 10s per instance
python test_suite.py --verbose              # see Gurobi's solver log
```


## 6. Suggested experiments

These are the questions you should be able to answer after playing with
the code:

1. **Encoding sanity.** Solve Instance 1 (no precedence) with the GA.
   Why does it reach makespan 7 even though the GA "knows nothing" about
   load balancing? Trace through the SSGS decoder for one chromosome.

2. **Where compatibility helps the ILP.** Compare the ILP's `num_vars`
   on Instance 2 with FFT compat versus without (set `compat=None`).
   How much smaller is the model? Does the runtime change in the same
   ratio?

3. **Where the GA pulls ahead.** Build an instance with n=20 or n=30
   tasks (you can copy and extend Instance 3's pattern). Set
   `--ilp-time-limit 30`. At what size does the ILP hit the time limit
   before proving optimality? Does the GA still produce a feasible
   schedule of the same quality?

4. **GA stability.** Drop `pop_size` to 10 or `n_generations` to 20 in
   `test_suite.py`. How does `hits` change on Instance 3? What does the
   `mean` column tell you that `best` doesn't?

5. **Memory as a real constraint.** Reduce `capacity` on Instance 3 to,
   say, `[6, 6, 6]`. Does the GA still find feasible solutions? Does
   the penalty term in `evaluate()` ever fail to prevent infeasibility?
   When would a repair operator be a better choice than a penalty?


## 7. Adding your own benchmark

Add a function in `test_suite.py`:

```python
def instance_4_my_dag() -> Instance:
    n, m = 8, 4
    exec_times = [...]                # n × m
    memory     = [...]                # length n
    capacity   = [...]                # length m
    edges      = [(0, 1), (1, 2), ...]
    compat     = [[1] * m for _ in range(n)]
    # ...optional restrictions...
    return Instance(
        name="4. My DAG (n=8, m=4)",
        problem=Problem(n=n, m=m, exec_times=exec_times,
                        memory=memory, capacity=capacity, name="custom"),
        edges=edges, compat=compat, known_optimum=None,
    )
```

then append it to the `instances` list in `main()`. Both solvers will
pick it up with no further changes.
