Dear Umut and Eren,

Thank you for your proposal. You can find my feedback below.

---

## 1. Topic Suitability for the Course

✅ **Very suitable for the Embedded Systems Design course.**

Your project addresses important embedded systems challenges such as:

- real-time task scheduling  
- shared resource contention (memory/cache/bus)  
- deterministic behavior under resource constraints  
- scheduler behavior in RTOS environments  

These topics are central to **modern embedded and real-time system design**, and therefore the proposal aligns very well with the **Optimization Track**.

---

## 2. Problem Definition

👍 **The problem is clearly motivated and technically meaningful.**

You correctly point out that classical scheduling algorithms such as **EDF and RM** typically assume simplified execution models and often ignore effects such as **memory contention and cache interference**, which can degrade real-time performance in practice.

However, in the **Week 7 submission**, please clarify the **formal system model** more explicitly. In particular:

- the **processor model** (single-core vs multi-core)  
- the **task model** (periodic/sporadic tasks, deadlines, WCET assumptions)  
- how **memory intensity or contention** will be estimated or measured  

Try to provide a mathematical formulation of the problem as much as possible, even if you are not going to implement such optimization approach. But it will help us understand the objective function and will make the optimization problem more precise.

---

## 3. System Design / Methodology

👍 **Good initial methodology proposal.**

Your idea combines:

- an **online heuristic scheduler extension** (Slack-Aware EDF)  
- an **offline optimization model (ACO)** used as a theoretical reference schedule  

This is a nice approach because it allows you to:

- evaluate how close the online heuristic gets to an offline optimal schedule  
- provide a meaningful baseline for experimental evaluation.

One important clarification for the updated proposal would be to explain:

- how **memory intensity of tasks will be estimated**  
- what **metrics define contention penalties**  
- what **features the heuristic scheduler uses to reorder tasks**

---

## 4. Feasibility for the Semester

⚠️ **Feasible, but the scope should remain controlled.**

Implementing a **full custom RTOS scheduler** could become complex.

A reasonable scope would be:

- implement a **scheduler wrapper or modified scheduling policy** in FreeRTOS  
- simulate or generate workloads with different **memory intensities**  
- compare **EDF vs your heuristic vs offline ACO reference**

You do **not need to build a full OS-level scheduling framework** from scratch.

---

## 5. Important Design Considerations (Required to Discuss)

Even if they cannot all be fully implemented, I strongly encourage you to **explicitly discuss important embedded system design considerations** in the extended proposal and final report.

For example:

- **real-time guarantees and schedulability analysis**
- **worst-case latency and deadline miss ratio**
- **system predictability under memory contention**
- **scalability (more tasks / higher contention)**
- **energy implications of scheduling policies**
- **reliability under overload conditions**

Even if some of these aspects are not fully implemented, they should at least be **acknowledged and discussed** as part of the system design requirements or future work.

---

## 6. Suggested Semester Plan

### Literature review and system modeling
- review EDF, RM, and contention-aware scheduling  
- define system architecture and task model  

### Problem formulation
- define optimization objective and constraints  
- define memory contention metrics  

### Baseline implementation
- implement standard EDF scheduling baseline  

### Proposed heuristic scheduler
- implement Slack-Aware EDF wrapper  
- integrate contention metrics into scheduling decisions  

### Offline optimization model
- implement ACO-based optimal schedule generator  

### Evaluation
- run workloads with different contention levels  
- compare EDF, heuristic scheduler, and ACO baseline  

### Analysis
- evaluate deadline misses, latency variability, and scheduling stability  

---

## 7. Overall Assessment

**Strong proposal with good alignment to the course objectives.**

### Strengths
- clear real-time systems motivation  
- good combination of varying optimization approaches  
- relevant embedded systems problem  

### Suggested improvements for Week 7 submission
- clearly define the **system and task model** and the **objective function**
- clarify how **memory contention metrics** will be obtained
- define **evaluation metrics** more explicitly

Good work so far.

---

Best regards,  
Selma
