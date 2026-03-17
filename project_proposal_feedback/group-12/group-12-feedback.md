Dear Gökçer,

Thank you for your proposal. The project idea is interesting and socially meaningful, and it addresses a real-world problem in disaster response. Below you can find my feedback on the current proposal.

---

## 1. Topic Suitability for the Course

✅ **Suitable for the Embedded Systems Design course.**

Your project proposes designing a **distributed acoustic localization system for search-and-rescue (SAR) scenarios** using embedded sensor nodes and edge computing.

The system involves several relevant embedded systems elements:

- distributed embedded sensor nodes (ESP32 + digital microphone)
- wireless communication between nodes (LoRa)
- edge processing (Jetson Nano)
- time synchronization across distributed systems
- real-time signal acquisition and processing

This clearly falls within the **Implementation Track**, as it involves system architecture design, hardware integration, communication protocols, and embedded signal processing.

---

## 2. Problem Definition

👍 **The motivation and societal relevance are very clear.**

The proposal explains that the system aims to detect and localize acoustic signals produced by trapped survivors using **Time Difference of Arrival (TDoA)** localization across multiple nodes.

However, the **technical problem statement could be defined more explicitly**. In particular, it would help to clarify the main embedded system challenges, for example:

- achieving **precise time synchronization between distributed sensor nodes**
- **accurate timestamping of acoustic events**
- reliable **wireless communication under low-power constraints**
- performing **real-time localization with noisy acoustic data**

Clearly identifying these challenges will help define measurable technical objectives.

---

## 3. System Design Quality

👍 **Good high-level system architecture.**

The proposed architecture consists of:

- multiple **sensor nodes** (ESP32 + digital microphone)
- **LoRa communication** between nodes
- a **Jetson Nano gateway** for localization computation
- a **web dashboard** for visualization.

This layered architecture is reasonable and reflects typical **wireless sensing system design**.

The choice of:

- **ESP32-S3** for sensor nodes
- **I2S digital microphones**
- **GNSS PPS synchronization**
- **LoRa communication**

is technically reasonable for a distributed embedded sensing system.

However, for the next proposal version it would be useful to include a more detailed **software architecture**, including:

- the **acoustic event detection pipeline**
- timestamp acquisition and synchronization workflow
- communication protocol between nodes and gateway
- localization computation pipeline.

---

## 4. Feasibility for the Semester

✅ **Feasible for the semester if the implementation scope is prioritized carefully.**

Your proposal includes several technically interesting components:

- distributed sensor nodes
- GNSS-based time synchronization
- LoRa communication
- acoustic signal processing (GCC-PHAT)
- machine learning for event classification
- localization algorithms
- dashboard visualization

While all of these are relevant to the system, it is important to **clearly prioritize the core embedded functionality**.

A reasonable minimal system could focus on:

- a **small number of sensor nodes** (e.g., 3–4 nodes)
- **TDoA-based localization using GCC-PHAT**
- reliable **timestamping and synchronization**
- localization computation on the **Jetson Nano gateway**

Additional features such as **machine learning–based event classification, advanced filtering, or more sophisticated visualization tools** can be treated as **optional extensions** if time allows.

---

## 5. Important Design Considerations (Required to Discuss)

Since this is an embedded distributed sensing system, please explicitly discuss several important system design considerations:

- **time synchronization accuracy** between nodes (critical for TDoA)
- **latency of wireless communication**
- **power consumption of sensor nodes**
- **real-time detection and processing delays**
- **robustness to environmental noise**
- **scalability with increasing number of nodes**

Even if some of these aspects are not fully implemented, they should be **clearly discussed as design constraints or limitations**.

---

## 6. Suggested Semester Plan

### Requirements and system specification

- define functional and non-functional requirements
- define localization accuracy targets
- define synchronization precision requirements

### System architecture design

- finalize node architecture
- define communication protocol
- define data flow from sensor nodes to gateway

### Hardware prototyping

- implement ESP32 + microphone node
- implement LoRa communication
- verify timestamp synchronization

### Signal processing implementation

- implement acoustic event detection
- implement GCC-PHAT time delay estimation

### Localization algorithm

- implement multilateration solver
- test localization accuracy with known sound sources

### Evaluation

- measure localization accuracy
- evaluate synchronization precision
- test robustness under noise conditions

---

## 7. Overall Assessment

**Interesting project with strong societal impact and clear embedded systems relevance.**

### Strengths

- meaningful real-world application (disaster search and rescue)
- distributed embedded architecture
- good choice of embedded hardware platforms
- well-written motivation and related work

### Suggested improvements for Week 7 submission

- clearly define the **system architecture and data flow**
- specify the **time synchronization method and required precision**
- clarify the **localization algorithm pipeline**
- define **performance evaluation metrics** (localization error, latency, node synchronization accuracy)
- explain the **role of machine learning in the system** and whether it is essential or optional

If the scope is kept manageable and the embedded system architecture is clearly defined, this project can become a very good example of a **distributed embedded sensing system for real-world applications**.

---

Best regards,  
Selma