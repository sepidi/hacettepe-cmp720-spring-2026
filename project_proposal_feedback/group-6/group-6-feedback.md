Dear Oğulcan and Barış,

Thank you for your proposal. You can find my feedback below.

---

## 1. Topic Suitability for the Course

✅ **Very suitable for the Embedded Systems Design course.**

Your project focuses on **task scheduling in NoC-based multicore systems**, which is a well-established and important topic in embedded system architecture and system-level design.

The proposal includes several relevant aspects:

- Network-on-Chip (NoC) communication behavior  
- task scheduling in MPSoCs  
- communication/computation trade-offs  
- congestion-aware optimization  

These topics align well with the **Optimization Track** of the course.

---

## 2. Problem Definition

👍 **The problem is clearly stated and technically meaningful.**

You correctly highlight that **duplication-based scheduling methods often ignore network contention**, which can degrade performance as NoC sizes grow.

Your proposed idea — incorporating **contention penalties into duplication decisions** — is reasonable and logically motivated.

For the **Week 7 proposal**, please clarify the **formal problem definition**, including:

- the **system model** (NoC topology, number of cores, router assumptions)
- the **task model** (DAG properties, computation/communication cost assumptions)
- the **contention model** used in BookSim
- the **objective function** (e.g., minimize makespan, minimize congestion, etc.)

These elements will make the optimization pipeline clearer.

---

## 3. Methodology / Optimization Pipeline

👍 **Good initial methodology and tool selection.**

The proposed pipeline combines:

- DAG generation (TGFF or daggen)
- scheduling engine implementation
- network simulation with BookSim
- evaluation of contention-aware duplication decisions.

Using **BookSim 2.0** for network-level metrics is a good choice because it provides realistic statistics such as:

- link utilization  
- buffer occupancy  
- latency  

For the updated proposal version in Week 7, please:

- include a clear **mathematical formulation of the problem**, including the decision variables, constraints, and objective function
- clarify how the **contention penalty term** will be computed
- clarify how **scheduling decisions interact with the network simulation**
- specify whether the scheduling and network simulation will run **sequentially or iteratively**
- include **pseudocode of the proposed scheduling algorithm**, clearly showing how the contention-aware duplication decision and penalty calculation are incorporated into the scheduling process

---

## 4. Feasibility for the Semester

⚠️ **Feasible if the scope remains focused.**

Your current plan includes:

- multiple scheduling strategies
- DAG generation
- network-level simulation
- performance evaluation across several parameters.

This is reasonable, but I recommend keeping the evaluation manageable.

For example, you could:

- limit experiments to **a few NoC sizes (e.g., 4×4 and 8×8)**
- use **several CCR values**
- compare **three scheduling strategies** (baseline HEFT, classical duplication, your method)

This would already provide meaningful experimental results.

---

## 5. Important Design Considerations (Required to Discuss)

Even though this is a simulation-based optimization project, it is still important to discuss broader **embedded system design considerations**.

In your extended proposal and final report, please explicitly address aspects such as:

- **real-time execution constraints**
- **communication latency variability**
- **energy implications of duplication decisions**
- **scalability with increasing NoC size**
- **fairness and resource utilization balance**

Even if not all of these aspects are directly implemented, they should be **acknowledged as system design considerations or future research directions**.

---

## 6. Suggested Semester Plan

### Literature review and system modeling
- review duplication-based scheduling techniques
- define system and task models
- define evaluation metrics

### Baseline implementation
- implement HEFT scheduling baseline
- implement classical duplication-based scheduler

### Contention modeling
- integrate BookSim for network statistics
- define congestion metrics used in scheduling

### Proposed scheduling heuristic
- implement contention-aware duplication decision rule
- integrate penalty model into scheduling logic

### Experimental evaluation
- generate workloads with different CCR values
- simulate different NoC sizes
- collect latency and utilization statistics

### Analysis
- compare makespan and duplication overhead
- analyze congestion effects on scheduling decisions

---

## 7. Overall Assessment

**Strong proposal with clear technical motivation and appropriate methodology.**

### Strengths

- clear problem motivation
- strong relation to NoC-based embedded systems
- good use of existing simulation tools
- well-structured evaluation plan

### Suggested improvements for Week 7 submission

- clearly define the **system and task models** (NoC topology, processor model, DAG assumptions)
- include a **mathematical formulation of the problem**, specifying the decision variables, constraints, and objective function
- define the **contention penalty formulation** used in the duplication decision
- describe the **integration between the scheduler and BookSim**, including how network statistics are obtained
- specify whether scheduling and network simulation run **sequentially or iteratively**
- include **pseudocode of the proposed scheduling algorithm**, clearly showing how the contention-aware duplication decision is applied

Overall, this is a well-structured proposal and a good fit for the course.

---

Best regards,  
Selma