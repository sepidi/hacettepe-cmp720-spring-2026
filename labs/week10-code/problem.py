"""
CMP720 — Makespan scheduling problem definition and benchmark generators.

A Problem captures:
    n                 — number of tasks
    m                 — number of processors
    exec_times[i][j]  — execution time of task i on processor j
    memory[i]         — memory footprint of task i
    capacity[j]       — memory capacity of processor j
"""

from __future__ import annotations

import random
from dataclasses import dataclass
from typing import List


@dataclass
class Problem:
    n: int                              # number of tasks
    m: int                              # number of processors
    exec_times: List[List[int]]         # shape (n, m)
    memory: List[int]                   # shape (n,)
    capacity: List[int]                 # shape (m,)
    name: str = ""

    def __post_init__(self):
        assert len(self.exec_times) == self.n
        assert all(len(row) == self.m for row in self.exec_times)
        assert len(self.memory) == self.n
        assert len(self.capacity) == self.m

    # ---- convenience: compute loads and check feasibility for an assignment ----

    def compute_loads(self, y: List[int]) -> List[int]:
        """Given y with y[i] ∈ {0,…,m-1}, return per-processor execution load."""
        loads = [0] * self.m
        for i, j in enumerate(y):
            loads[j] += self.exec_times[i][j]
        return loads

    def makespan(self, y: List[int]) -> int:
        return max(self.compute_loads(y))

    def memory_use(self, y: List[int]) -> List[int]:
        use = [0] * self.m
        for i, j in enumerate(y):
            use[j] += self.memory[i]
        return use

    def is_feasible(self, y: List[int]) -> bool:
        use = self.memory_use(y)
        return all(use[j] <= self.capacity[j] for j in range(self.m))

    def lower_bound(self) -> int:
        """Cheap lower bound: per-task minimum / m, rounded up. Ignores memory."""
        total_min = sum(min(row) for row in self.exec_times)
        return -(-total_min // self.m)  # ceil division


# ---------------------------------------------------------------------------
# Small benchmark — the 6×3 instance used throughout the Weeks 8+ slides.
# ---------------------------------------------------------------------------

def small_benchmark() -> Problem:
    """The exact instance from slide 8 of Weeks 8+. Optimal makespan = 7."""
    exec_times = [
        [3, 7, 5],   # T1
        [8, 2, 6],   # T2
        [6, 3, 4],   # T3
        [4, 5, 9],   # T4
        [5, 8, 3],   # T5
        [7, 4, 8],   # T6
    ]
    memory   = [2, 3, 2, 4, 1, 3]
    capacity = [8, 7, 6]
    return Problem(
        n=6, m=3,
        exec_times=exec_times,
        memory=memory,
        capacity=capacity,
        name="Small (6×3, from slides)",
    )

# ---------------------------------------------------------------------------
# Pretty-printing (shared by ILP and SA runners)
# ---------------------------------------------------------------------------

def describe(problem: Problem) -> str:
    return (
        f"{problem.name}\n"
        f"  n={problem.n}, m={problem.m}\n"
        f"  total task memory = {sum(problem.memory)}\n"
        f"  total capacity    = {sum(problem.capacity)}\n"
        f"  slack             = {sum(problem.capacity) / sum(problem.memory):.2f}×\n"
        f"  trivial LB (ceil of per-task minima / m) = {problem.lower_bound()}"
    )


def format_assignment(problem: Problem, y: List[int]) -> str:
    """Small instances only — returns a compact table."""
    if problem.n > 20:
        return f"  (assignment vector of length {problem.n}; omitted)"
    loads = problem.compute_loads(y)
    mem   = problem.memory_use(y)
    lines = [f"  y = {[j + 1 for j in y]}   (1-indexed)"]
    for j in range(problem.m):
        tasks = [i + 1 for i, yi in enumerate(y) if yi == j]
        lines.append(
            f"    P{j + 1}: tasks {tasks}  "
            f"exec={loads[j]}  mem={mem[j]}/{problem.capacity[j]}"
        )
    lines.append(f"  makespan = {max(loads)}")
    return "\n".join(lines)
