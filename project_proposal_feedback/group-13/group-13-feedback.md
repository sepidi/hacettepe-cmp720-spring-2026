Dear Emre and Yavuz Selim,

Thank you for your proposal. This is a technically strong and well-structured idea that aligns well with modern embedded AI systems and heterogeneous computing. Below you can find my detailed feedback.

---

## 1. Topic Suitability for the Course

✅ **Very suitable for the Embedded Systems Design course.**

Your project proposes optimizing a **heterogeneous edge AI inference pipeline on the TI TDA4VM SoC using the C7x DSP and MMA accelerator**.

The project involves several important embedded systems aspects:

- heterogeneous computing (ARM + DSP + accelerator)  
- embedded AI inference under resource constraints  
- memory hierarchy optimization (DDR vs on-chip SRAM)  
- performance and energy-aware system design  
- system-level pipeline optimization  

This fits very well within the **Optimization Track**, as the core contribution is improving system performance (latency, throughput, efficiency) through architectural and software-level optimization.

---

## 2. Problem Definition

👍 **Clear and well-motivated problem.**

The proposal clearly identifies key challenges:

- balancing accuracy, latency, and energy efficiency  
- memory bandwidth bottlenecks (DDR vs MSMC SRAM)  
- cross-core workload distribution  
- quantization-related accuracy degradation

The problem is well grounded in real-world edge AI deployment.

- explicitly defining and formulating the **optimization objective(s)** (e.g., minimizing latency, maximizing FPS, maximizing TOPS/W)
- clarifying whether the problem is **single-objective or multi-objective**, and how trade-offs are handled
- clearly defining the **decision variables** (e.g., task partitioning, memory placement, pipeline scheduling)
- specifying the **system constraints** (e.g., memory capacity, bandwidth limits, real-time requirements, hardware resource limits)

---

## 3. System Design / Methodology

👍 **Very good high-level system architecture.**

The proposed pipeline clearly separates:

- ARM Cortex-A72 → data ingestion and preprocessing  
- C7x DSP + MMA → neural network inference  
- C66x DSP / ARM → post-processing  
- MSMC SRAM → intermediate data buffering

This is a well-structured **heterogeneous pipeline** and reflects good understanding of the SoC architecture.

For the updated proposal version, please clarify:

- whether the pipeline is **fully asynchronous or partially synchronized**
- how **data transfers between DDR and MSMC SRAM are scheduled**
- how **task partitioning decisions are made** (static vs adaptive)
- how **pipeline stages are coordinated** (buffers, queues, synchronization mechanisms)

A clearer **software/dataflow diagram** would significantly strengthen the proposal.

---

## 4. Feasibility for the Semester

✅ **Feasible for the semester.**

The project is well scoped:

- one model (lightweight CNN)  
- use of TIDL framework for deployment  
- clear staged milestones (quantization → deployment → profiling)

The use of existing tools (TIDL, TI SDK) significantly reduces implementation complexity.

To ensure smooth progress, I recommend keeping the focus on:

- correct deployment of the model on the heterogeneous platform  
- systematic profiling and bottleneck analysis  
- targeted optimization (memory + scheduling)

---

## 5. Important Design Considerations (Required to Discuss)

Since this is a heterogeneous embedded AI system, please explicitly address:

- **memory bandwidth limitations** and data movement costs  
- **latency vs throughput trade-offs** in pipeline design  
- **synchronization overhead between cores**  
- **impact of quantization on accuracy vs performance**  
- **energy efficiency considerations (e.g., TOPS/W)**  
- **scalability with different model sizes or input resolutions**

These should be clearly discussed as **design constraints or evaluation dimensions**, even if not all are fully optimized.

---

## 6. Suggested Semester Plan

### Problem formulation and system modeling

- define optimization objectives and constraints  
- define evaluation metrics (latency, FPS, energy efficiency)  

### Baseline implementation

- deploy FP32 / INT8 model using TIDL  
- establish baseline performance on the platform  

### System integration

- implement heterogeneous pipeline (ARM + DSP + accelerator)  
- configure memory hierarchy (DDR vs MSMC usage)  

### Profiling and bottleneck analysis

- measure latency and throughput per stage  
- identify memory and synchronization bottlenecks  

### Optimization

- optimize memory placement (MSMC usage)  
- optimize task partitioning across cores  
- reduce data movement overhead  

### Evaluation

- compare baseline vs optimized system  
- analyze trade-offs between performance, accuracy, and energy  

---

## 7. Overall Assessment

**Strong proposal with clear system-level thinking and high relevance to modern embedded AI systems.**

### Strengths

- well-defined and relevant problem  
- strong heterogeneous system architecture  
- realistic platform (TI TDA4VM)  
- clear pipeline design and milestones  
- good consideration of performance and energy trade-offs  

### Suggested improvements for Week 7 submission

- explicitly define the **optimization problem**, including the objective function, decision variables, and constraints
- clearly describe the **task partitioning and scheduling strategy** across heterogeneous processing units
- provide a more detailed explanation of the **memory management and data movement strategy** (e.g., DDR vs on-chip SRAM usage)
- include a clearer **software/dataflow architecture diagram** illustrating pipeline stages and interactions
- define the **evaluation methodology and performance metrics** more formally (e.g., latency, FPS, energy efficiency)
- optionally include **pseudocode or an algorithmic description** of the proposed optimization strategy

If these aspects are clarified, this project can become a very strong example of **heterogeneous embedded AI pipeline optimization on edge devices**.

---

Best regards,  
Selma