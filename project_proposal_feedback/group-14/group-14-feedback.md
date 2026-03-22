Dear Sezil and Mehmet Yusuf,

Thank you for your proposal. This is a very interesting and well-motivated project, especially given its safety-critical nature and strong societal impact. Below you can find my detailed feedback.

---

## 1. Topic Suitability for the Course

✅ **Very suitable for the Embedded Systems Design course.**

Your project proposes a **closed-loop Galvanic Vestibular Stimulation (GVS) system for real-time balance assistance**.

The project involves several important embedded systems aspects:

- real-time sensing and actuation (IMU → stimulation loop)  
- embedded control systems (PID-based closed-loop control)  
- safety-critical embedded design  
- RTOS-based task scheduling (FreeRTOS)  
- hardware/software co-design (MCU + analog stimulation circuit)  

This fits very well within the **Implementation Track**, as it requires full system design from sensing to actuation under strict timing and safety constraints.

---

## 2. Problem Definition

👍 **Very strong and well-articulated problem definition.**

The proposal clearly defines:

- the clinical motivation (vestibular disorders and fall risk)  
- the technical gap (lack of real-time, wearable, safe GVS systems)  
- the system objective (closed-loop balance assistance using real-time feedback) 

The formulation is already quite strong.

For the **Week 7 updated proposal**, you can further strengthen it by:

- explicitly defining **control objectives** (e.g., minimizing postural sway, stabilizing orientation)
- specifying **performance metrics** (e.g., response time, control latency, stability measures)
- clarifying what constitutes **successful system behavior**

---

## 3. System Design Quality

👍 **Excellent system-level architecture.**

The block diagram clearly shows a complete closed-loop system:

- IMU sensing (MPU6050)  
- MCU with RTOS handling sensor fusion and control  
- PID controller and safety monitor  
- GVS driver with constant current source  
- feedback loop through human response

This is a **textbook embedded control system architecture**, and it is very well structured.

Strong aspects:

- clear separation of sensing, processing, and actuation  
- inclusion of **sensor fusion (complementary/Kalman filter)**  
- explicit **closed-loop control design (PID)**  
- strong emphasis on **safety mechanisms**

For the next version, please clarify:

- the **task structure under FreeRTOS** (sampling, control, safety monitoring tasks)
- the **timing model** (sampling frequency, control loop period)
- the **data flow and buffering strategy**

---

## 4. Feasibility for the Semester

✅ **Feasible for the semester with the current scope.**

The project is well scoped:

- a single sensing modality (IMU)  
- a defined control strategy (PID)  
- a clearly defined actuation mechanism  

The strong emphasis on **safety and closed-loop operation** is appropriate and achievable within the timeframe.

A reasonable development approach would be:

- first validating the **sensor fusion and control loop in simulation**
- then integrating the **hardware stimulation subsystem**

---

## 5. Important Design Considerations (Required to Discuss)

Since this is a **safety-critical embedded system**, I expect a very explicit and structured discussion of the following aspects:

- **hard real-time constraints** of the control loop  
- **maximum allowable latency** between sensing and stimulation  
- **safety guarantees** (hardware vs software protection layers)  
- **fail-safe mechanisms** (e.g., watchdogs, emergency shutdown conditions)  
- **signal noise and filtering robustness**  
- **human safety constraints and ethical considerations**

You already address safety aspects well, which is very good. For the Week 7 proposal, please formalize these as **design requirements and constraints**, and explicitly discuss those that cannot be fully implemented, including justification and possible future improvements.

---

## 6. Suggested Semester Plan

### Requirements and system specification

- define functional and non-functional requirements  
- define safety constraints and limits (current, latency)  
- define control objectives and evaluation metrics  

### System architecture design

- define hardware architecture (MCU + GVS driver)  
- define RTOS task structure and timing  
- define data flow and control loop  

### Sensor processing

- implement IMU data acquisition  
- implement sensor fusion (complementary/Kalman filter)  

### Control system implementation

- implement PID controller  
- tune control parameters  
- validate closed-loop behavior  

### Safety subsystem

- implement current limiting and watchdog mechanisms  
- test fault scenarios  

### Evaluation

- measure control loop latency  
- evaluate system response under disturbances  
- verify safety constraints  

---

## 7. Overall Assessment

**Very strong proposal with excellent system-level thinking and high real-world impact.**

### Strengths

- clear and meaningful application domain  
- strong closed-loop embedded system design  
- excellent emphasis on safety-critical aspects  
- well-structured architecture and methodology  

### Suggested improvements for Week 7 submission

- explicitly define **control objectives and performance metrics**
- clearly describe the **RTOS task structure and timing model**
- formalize **safety requirements and constraints**
- define **evaluation methodology (latency, stability, safety tests)**
- include a **control loop timing diagram or pseudocode**

If these aspects are clearly defined, this project can become an excellent example of a **safety-critical embedded control system with real-world impact**.

---

Best regards,  
Selma