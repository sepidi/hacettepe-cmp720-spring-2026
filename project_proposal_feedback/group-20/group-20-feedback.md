Dear İlter Doğaç,

Thank you for your proposal. The topic is relevant and suitable for the course, and it is good to see that you are building upon an existing study. Below you can find my detailed feedback.

---

## 1. Topic Suitability for the Course

✅ **Suitable for the Embedded Systems Design course.**

Your project proposes implementing an **autonomous obstacle avoidance system using LiDAR on an embedded platform (Raspberry Pi), simulated in Renode**.

The project involves several important embedded systems aspects:

- sensor integration (LiDAR)  
- embedded control of actuators (motors)  
- real-time perception and decision-making  
- system-level simulation (Renode)  
- hardware/software integration  

This fits well within the **Implementation Track**.

---

## 2. Problem Definition

⚠️ **The general topic is clear, but the problem formulation needs improvement.**

The proposal describes an autonomous obstacle avoidance system using LiDAR, which is a well-known and relevant problem.

However, the current wording is problematic. In particular, the abstract states:

> “we present our autonomous vehicle implementation…” 

This is misleading, as it suggests that the system is already developed, whereas the goal of this project is to **replicate and study an existing work**.

For the revised proposal, you should:

- clearly state that you are **reproducing/replicating the method from the referenced study**
- explain **what exactly you will implement, evaluate, and analyze**
- define the **technical problem in embedded system terms**, for example:
  - real-time obstacle detection using LiDAR  
  - navigation under resource constraints  
  - system behavior under different environmental conditions  

---

## 3. System Design Quality

⚠️ **The current design description is too high-level.**

You mention that the system includes:

- main controller  
- motor controller  
- LiDAR sensor  
- simulation environment (Renode) 

However, the proposal lacks:

- a **clear system architecture diagram**
- a **software pipeline description**
- a **data flow explanation (sensor → processing → control → actuation)**

For the next version, please clearly describe:

- how LiDAR data is processed  
- how obstacle detection decisions are made  
- how control commands are generated  
- how the system operates over time (loop structure, timing)

A **block diagram and dataflow diagram** are strongly recommended for full credit.

---

## 4. Feasibility for the Semester

✅ **Feasible for the semester.**

The project is manageable because:

- it is based on an existing study  
- the scope can be controlled through simulation  
- hardware complexity is limited  

However, feasibility depends on **clearly limiting the scope** to:

- reproducing a specific method  
- implementing a working pipeline  
- evaluating system performance  

---

## 5. Important Design Considerations (Required to Discuss)

Since this is an embedded autonomous system, please explicitly address:

- **real-time constraints** (sensor processing and control loop timing)
- **latency between sensing and actuation**
- **robustness to sensor noise and environmental variation**
- **control stability and failure cases (e.g., missed obstacles)**
- **resource constraints** (CPU, memory in embedded platform)
- **simulation vs real-world limitations**

These should be clearly defined and discussed as **system requirements and constraints**, even if not all are fully implemented due to the project scope.

---

## 6. Suggested Semester Plan

### Literature review and problem definition

- study the referenced paper in detail  
- clearly define what will be replicated  

### System architecture design

- define hardware and software architecture  
- define data flow and control pipeline  

### Simulation setup

- configure Renode environment  
- model system components  

### Implementation

- implement LiDAR processing  
- implement obstacle detection logic  
- implement control and navigation logic  

### Testing and evaluation

- test in different simulated environments  
- evaluate obstacle avoidance performance  
- measure latency and reliability  

---

## 7. Overall Assessment

**Good topic, but the proposal needs clearer formulation and more technical detail.**

### Strengths

- relevant and practical application  
- appropriate use of embedded systems concepts  
- feasible scope  

### Suggested improvements for Week 7 submission

- clearly state that this is a **replication/implementation of an existing study**
- significantly improve the **literature review** (include more relevant works and clearly define baselines)
- provide a clear **system architecture and dataflow description**
- define **evaluation methodology and performance metrics**
- describe and illustrate the **control and decision-making pipeline in detail**

If these aspects are improved, this project can become a solid example of an **embedded autonomous navigation system**.

---

Best regards,  
Selma