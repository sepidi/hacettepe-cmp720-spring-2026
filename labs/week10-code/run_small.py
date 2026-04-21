"""
Small benchmark — the 6×3 instance from the Weeks 10 slides.

Expected result:
    ILP  → makespan = 7 (optimal), sub-second.
    SA   → makespan = 7 most of the time, a handful of ms per run.
"""

from __future__ import annotations

from problem import small_benchmark, describe, format_assignment
from ilp_solver import solve_ilp
from sa_solver import SAConfig, solve_sa_multirestart


def main() -> None:
    problem = small_benchmark()
    print("=" * 72)
    print(describe(problem))
    print("=" * 72)

    # ---- ILP (Model A) ----
    print("\n[ILP — Model A (Gurobi)]")
    y_ilp, obj_ilp, stats_ilp = solve_ilp(problem)
    print(f"  status    : {stats_ilp['status']}")
    print(f"  makespan  : {obj_ilp:.0f}")
    print(f"  runtime   : {stats_ilp['runtime'] * 1000:.2f} ms")
    print(f"  vars      : {stats_ilp['num_vars']}")
    print(f"  constrs   : {stats_ilp['num_constrs']}")
    print(format_assignment(problem, y_ilp))

    # ---- SA (multi-restart) ----
    print("\n[SA — 30 restarts, reject-infeasible]")
    cfg = SAConfig(
        T0=10.0,
        alpha=0.95,
        inner_iter=50,
        n_temperatures=200,
        strategy="reject",
        seed=1,
    )
    best, runs = solve_sa_multirestart(problem, n_restarts=30, base_config=cfg)

    makespans = [r.makespan for r in runs if r.feasible]
    total_time = sum(r.runtime for r in runs)
    print(f"  best makespan (over 30 runs): {best.makespan:.0f}")
    print(f"  mean makespan               : {sum(makespans) / len(makespans):.2f}")
    print(f"  worst makespan              : {max(makespans):.0f}")
    print(f"  hit-optimum rate            : "
          f"{sum(1 for m in makespans if m == obj_ilp)}/{len(makespans)}")
    print(f"  total wall time (all runs)  : {total_time * 1000:.2f} ms")
    print(f"  per-run wall time (mean)    : {total_time / len(runs) * 1000:.2f} ms")
    print(format_assignment(problem, best.assignment))

    # ---- verdict ----
    print("\n" + "-" * 72)
    if best.makespan == obj_ilp:
        print(f"  SA matched the ILP optimum ({obj_ilp:.0f}). As expected on a tiny instance.")
    else:
        print(
            f"  SA best = {best.makespan:.0f}, ILP optimum = {obj_ilp:.0f} "
            f"(gap = {best.makespan - obj_ilp:+.0f})."
        )
    print("-" * 72)


if __name__ == "__main__":
    main()
