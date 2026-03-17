Dear Metin and Anıl,

Thank you for your proposal. The project addresses an interesting and relevant problem in embedded edge AI systems, and it is good to see that you are thinking about system architecture, resource constraints, and simulation-based development. Below you can find my feedback on the current proposal.

---

## 1. Topic Suitability for the Course

✅ **Suitable for the Embedded Systems Design course.**

Your project proposes implementing an **embedded edge-AI based access control system using face recognition on an STM32H7 microcontroller**.

The proposal involves several important embedded system aspects:

- embedded AI inference on resource-constrained hardware  
- RTOS-based system design (FreeRTOS)  
- DMA-based data transfer and frame buffering  
- embedded computer vision pipeline  
- simulation-driven development using Renode  

These elements align well with the **Implementation Track**, since the project requires system architecture design, hardware/software integration, real-time task management, and performance-aware embedded implementation.

---

## 2. Problem Definition

👍 **The motivation and system goal are clear.**

The proposal describes a system that performs **face detection and recognition directly on a microcontroller**, avoiding cloud-based biometric processing. The goal is to reduce latency and improve privacy by performing inference locally on the device.

However, the **core embedded systems challenge could be articulated more explicitly**. For example, the project could emphasize technical questions such as:

- how to execute **real-time vision inference on a constrained MCU**
- how to manage **memory and frame buffers efficiently**
- how to design a **deterministic RTOS task pipeline for image processing**
- how to balance **inference accuracy vs computational constraints**

Clarifying these challenges will help define **measurable technical objectives** for the system.

---

## 3. System Design Quality

👍 **Good initial system architecture.**

The proposal already identifies several important architectural elements:

- STM32H743ZI microcontroller platform  
- FreeRTOS task scheduling  
- DMA-based frame buffer handling  
- TensorFlow Lite Micro inference pipeline  
- Renode-based system simulation and testing

The use of **Renode simulation to inject camera data into the MCU memory** is an interesting idea and can be useful for deterministic testing of embedded ML pipelines.

However, the proposal would benefit from a **clearer architectural diagram and processing pipeline**, including:

- camera frame acquisition flow  
- DMA buffer management  
- preprocessing steps  
- face detection stage  
- recognition inference stage  
- access control output (e.g., door unlock logic).

A visual **software pipeline diagram** would significantly improve clarity.

---

## 4. Feasibility for the Semester

⚠️ **Feasible if the scope is carefully controlled.**

The proposal currently includes several demanding elements:

- embedded vision pipeline  
- face detection algorithm implementation  
- CNN-based recognition model  
- TensorFlow Lite Micro deployment  
- FreeRTOS task architecture  
- Renode-based simulation infrastructure  

To keep the project manageable within the semester, I recommend focusing on a **minimal but functional system**:

- frame acquisition and preprocessing  
- **one lightweight face detection method**
- **deployment of a small quantized model using TFLite Micro**
- a simple access-control decision pipeline.

Advanced improvements such as optimizing recognition accuracy or supporting larger models can remain **optional extensions**.

---

## 5. Important Design Considerations (Required to Discuss)

Since this is an embedded edge-AI system, the proposal should explicitly address several system-level design considerations:

- **memory constraints** of the MCU (model size, frame buffers)
- **real-time inference latency**
- **task scheduling and pipeline timing under FreeRTOS**
- **DMA bandwidth and buffer management**
- **energy consumption of continuous inference**
- **reliability of recognition decisions**

These aspects should be **explicitly discussed as system requirements or design constraints**, even if they are not fully optimized during the project.

---

## 6. Suggested Semester Plan

### Requirements and system specification

- define functional and non-functional requirements  
- define latency targets for recognition pipeline  
- define system resource constraints

### System architecture design

- design software architecture (FreeRTOS tasks)  
- define data flow and buffer management strategy  
- define DMA-based frame acquisition pipeline

### Simulation environment

- configure Renode environment  
- implement camera input simulation  
- verify frame injection into MCU memory

### Embedded ML pipeline

- implement face detection stage  
- deploy quantized CNN model using TFLite Micro  
- integrate inference pipeline

### Access control logic

- implement recognition decision logic  
- control GPIO/UART output for access decisions

### Evaluation

- measure inference latency  
- measure memory utilization  
- evaluate recognition accuracy under test datasets

---

## 7. Overall Assessment

**Interesting embedded AI project that fits well within the course scope.**

### Strengths

- relevant real-world application (embedded biometric access control)  
- good consideration of embedded constraints (DMA, RTOS, memory limits)  
- thoughtful use of simulation tools (Renode)  
- well-structured milestone plan

### Suggested improvements for Week 7 submission

- clearly define the **software architecture and processing pipeline**
- specify **model size, memory requirements, and buffer strategy**
- clarify the **data flow from camera input to inference output**
- define **performance metrics** (e.g., latency, accuracy, memory usage)
- expand the **related work section** with more related embedded edge-AI examples

If the system architecture and embedded constraints are clearly defined, this project can become a strong example of **TinyML deployment on microcontrollers**.

---

Best regards,  
Selma