Dear Ayberk and Berkay,

Thank you for your proposal. As you know, we had already discussed this topic earlier and I had shared a detailed example plan to help you structure the project. It is good to see that you continued developing the idea and adapting it to an embedded CPU+GPU platform. Below you can find my feedback on the current proposal.

---

## 1. Topic Suitability for the Course

✅ **Suitable for the Embedded Systems Design course.**

Your project proposes implementing a **quadrotor simulation and controller evaluation framework on an embedded CPU+GPU platform (NVIDIA Jetson)**. 

The project involves several important embedded systems aspects:

- real-time simulation of nonlinear system dynamics  
- embedded heterogeneous computing (CPU + GPU)  
- performance-aware task partitioning  
- embedded experimentation with robotics/control workloads  

The idea of keeping the **control loop simulation on the CPU** while using the **GPU for Monte Carlo controller evaluation** is a reasonable architecture for an embedded heterogeneous system.

This fits well with the **Implementation Track**.

---

## 2. Problem Definition

👍 **The general motivation is good, but the core problem statement should be clarified.**

Your proposal focuses on:

- evaluating controller robustness via Monte Carlo simulations  
- accelerating the evaluation using embedded GPU resources.

However, it is important to clearly distinguish between:

- **the quadrotor dynamic simulation**
- **controller evaluation / Monte Carlo experiments**
- **GPU acceleration methodology**

For the **Week 7 proposal**, please clearly define:

- which **quadrotor dynamic model** you will implement (e.g., the equations from the referenced paper)
- what **controller type** will be used (e.g., PID, MPC, or another approach)
- what exactly will be **parallelized on the GPU** (a diagram showing data flow and task mapping between CPU and GPU would be very helpful)
- what **performance or design objective** you are evaluating (e.g., execution time, scalability with increasing Monte Carlo samples, controller evaluation speed, etc.)

Right now, the description mixes **simulation, control, and evaluation aspects** without clearly separating their roles in the system.

---

## 3. System Design Quality

👍 **The high-level architecture is reasonable.**

The architecture shown in your proposal separates the system into:

- CPU: quadrotor simulation and controller execution  
- GPU: Monte Carlo evaluation and parameter exploration  
- PC: visualization and monitoring. 

This division of labor between CPU and GPU is logical and matches the strengths of each component.

However, for the next version please describe (and illustrate) the **software architecture more clearly**, including:

- the **simulation loop structure**
- the **data flow between CPU and GPU**
- how simulation states are transferred to GPU kernels
- how updated controller parameters are returned to the CPU.

---

## 4. Feasibility for the Semester

⚠️ **Feasible if the scope is carefully controlled.**

Your current proposal includes several complex elements:

- nonlinear quadrotor dynamics
- real-time simulation
- controller implementation
- Monte Carlo evaluation
- GPU acceleration

To keep the project manageable, I recommend focusing on:

- implementing **one quadrotor dynamic model**
- implementing **one controller**
- running **Monte Carlo evaluation in parallel on GPU**
- comparing **CPU-only vs CPU+GPU performance**.

This would already demonstrate the embedded heterogeneous computing concept.

---

## 5. Important Design Considerations (Required to Discuss)

Since this is an embedded systems project, please explicitly address important system-level design considerations, even if some are not fully implemented.

For example:

- **real-time execution constraints** for the simulation loop
- **latency of CPU–GPU data transfers**
- **deterministic timing behavior of the control loop**
- **energy consumption implications of GPU usage**
- **system scalability with increasing Monte Carlo samples**
- **reliability and numerical stability of the simulation**

These aspects are important in modern embedded system design and should at least be **acknowledged and discussed in the design requirements or limitations**.

---

## 6. Suggested Semester Plan

### Requirements and system specification
- define functional and non-functional requirements
- define timing constraints for the simulation loop
- define evaluation metrics (runtime, throughput, etc.)

### System architecture design
- define CPU vs GPU responsibilities
- define communication and memory transfer mechanisms

### Mathematical model implementation
- implement the quadrotor dynamic model
- implement numerical integration method

### Controller implementation
- implement the chosen control algorithm
- verify basic simulation behavior

### GPU acceleration
- implement Monte Carlo simulation kernels
- parallelize scenario evaluation

### Evaluation
- compare CPU-only vs CPU+GPU performance
- analyze scalability with increasing number of simulations

---

## 7. Overall Assessment

**Interesting project idea that fits well with the embedded heterogeneous computing theme.**

### Strengths

- relevant real-world robotics application
- interesting use of embedded GPU acceleration
- clear motivation related to controller evaluation

### Suggested improvements for Week 7 submission

- clearly define the **quadrotor dynamic model and controller type** (e.g., equations implemented and control algorithm used)
- describe the **CPU–GPU interaction and data flow**, including what computations run on each processor
- clearly explain **what parts of the evaluation are parallelized on the GPU**
- specify the **performance or design objectives** (e.g., runtime improvement, scalability with increasing Monte Carlo samples, controller evaluation speed)
- include a clearer **software architecture and data-flow diagram**
- define the **timing assumptions and performance metrics** used for evaluation

If the scope is controlled and the system architecture is clearly defined, this project can become a very good demonstration of **embedded real-time simulation and heterogeneous computing**.

---

Best regards,  
Selma