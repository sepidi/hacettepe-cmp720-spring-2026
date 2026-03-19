Dear Ahmet,

Thank you for your proposal. This is a well-thought-out and technically strong idea with clear relevance to real-world UAV systems. Below you can find my detailed feedback.

---

## 1. Topic Suitability for the Course

✅ **Suitable for the Embedded Systems Design course.**

Your project proposes a **real-time energy-aware optimization framework for UAV missions running on embedded hardware (STM32)**.

The project involves several important embedded systems aspects:

- real-time decision-making under resource constraints  
- embedded optimization algorithms  
- interaction between control systems and high-level decision layers  
- energy-aware system design  
- hardware-informed modeling (PWM → power estimation)  

This fits very well with the **Optimization Track**, as the core contribution is the design and evaluation of a **real-time optimization algorithm under embedded constraints**.

---

## 2. Problem Definition

👍 **Very clear and well-motivated problem definition.**

The proposal clearly identifies:

- limitations of **static threshold-based RTH mechanisms**
- challenges of **real-time optimization under disturbances (e.g., wind)**
- need for **lightweight, embedded-compatible optimization methods**

The formulation is also strong:

- decision variable: **velocity (V)**
- objective: **energy vs mission time trade-off**
- constraint: **E_remaining ≥ E_safe**

This is a **well-defined optimization problem**.

For the **Week 7 proposal**, I recommend making the formulation even more explicit by:

- writing the **full mathematical formulation** (objective + constraints clearly defined)
- defining all variables and parameters precisely  
- specifying whether the problem is solved **discretely (candidate sampling) or continuously**

---

## 3. System Design Quality

👍 **Strong system-level architecture.**

The proposal clearly separates:

- **Inner Loop (flight controller)** → stabilization and trajectory tracking  
- **Outer Loop (STM32 companion)** → energy estimation and optimization  

The architecture diagram (page 2) clearly shows:

- telemetry inputs (PWM, GPS, energy, distance)
- energy estimation module
- predictive optimizer
- feasibility check and failsafe trigger

This separation of concerns is very appropriate for embedded UAV systems.

For the next version, please further clarify:

- how often the optimizer runs (**sampling period / control frequency**) - this can be updated in the final report if it changes.
- how telemetry is obtained (**simulation vs real system assumptions**)
- how the optimizer interacts with the flight controller (interface definition)

---

## 4. Feasibility for the Semester

✅ **Feasible for the semester with the current scoped approach.**

The proposal already includes a good strategy to maintain feasibility:

- decoupling trajectory planning from velocity optimization  
- reducing the problem to **1D velocity optimization**
- using **heuristics / metaheuristics instead of full optimal solvers**

The plan of using **Gurobi as a reference baseline** and then designing a lightweight solver is particularly strong and appropriate.

As long as the focus remains on:

- a **well-defined optimization problem**
- a **working embedded-compatible solver**
- **clear evaluation against baseline methods**

the scope is realistic.

---

## 5. Important Design Considerations (Required to Discuss if Cannot Be Fully Implemented)

Since this is an embedded optimization system, please explicitly address:

- **real-time constraints** (maximum computation time per decision step)
- **sampling/update frequency of the optimizer**
- **accuracy of the energy estimation model (PWM → power)**
- **robustness under noisy or delayed telemetry**
- **trade-off between solution quality and computation time**
- **fail-safe behavior guarantees**

These are critical aspects for embedded control and optimization systems and should be clearly defined as **design constraints**.

---

## 6. Suggested Semester Plan

### Problem formulation

- define full mathematical formulation  
- define objective function and constraints  
- define system parameters and assumptions  

### Baseline implementation

- implement threshold-based RTH baseline  
- define simulation environment and scenarios  

### Optimization modeling

- implement reference solver (e.g., Gurobi)  
- obtain lower-bound solutions  

### Embedded solver design

- design lightweight heuristic / search-based method  
- ensure real-time feasibility  

### Integration

- integrate optimizer with simulated flight loop  
- implement decision update mechanism  

### Evaluation

- compare against baseline methods  
- evaluate energy efficiency, mission success rate, safety  
- analyze computation time vs solution quality  

---

## 7. Overall Assessment

**Very strong proposal with clear problem definition and well-structured methodology.**

### Strengths

- well-defined optimization problem  
- strong system-level architecture  
- clear separation between control and optimization layers  
- realistic and meaningful application (UAV safety and energy management)  

### Suggested improvements for Week 7 submission

- include full **mathematical formulation** (objective + constraints + variables)
- clarify **solver design and algorithmic approach**
- define **real-time constraints and update frequency**
- explain **simulation or experimental setup in more detail**
- include **pseudocode of the proposed optimization method**

If the mathematical formulation and solver design are clearly presented, this project can become a very strong example of **embedded real-time optimization in cyber-physical systems**.

---

Best regards,  
Selma