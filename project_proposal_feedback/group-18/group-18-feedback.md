Dear Hüseyin Eren,

Thank you for your proposal. This is a very interesting and modern project that tackles an important challenge in embedded AI systems, particularly in offline and privacy-sensitive environments. Below you can find my detailed feedback.

---

## 1. Topic Suitability for the Course

✅ **Suitable for the Embedded Systems Design course.**

Your project proposes implementing an **offline Retrieval-Augmented Generation (RAG) pipeline on resource-constrained embedded systems (simulated ARM platform)**.

The project involves several important embedded systems aspects:

- deployment of AI models under **strict resource constraints (CPU, RAM)**  
- system-level pipeline design (ingestion → embedding → retrieval → generation)  
- performance-aware embedded computing  
- profiling and optimization under constrained environments  
- simulation of embedded platforms (QEMU ARM SBC)  

This fits well within the **Implementation Track**, as the project focuses on designing and evaluating a complete system pipeline under embedded constraints.

---

## 2. Problem Definition

👍 **Clear, relevant, and well-motivated problem.**

The proposal clearly identifies:

- the dependency of modern AI systems on cloud infrastructure  
- the limitations of deploying LLM-based systems on edge devices  
- the need for **offline, privacy-preserving AI pipelines**  

The formulation is strong.

For the **Week 7 updated proposal**, I recommend strengthening it further by:

- explicitly defining **system-level objectives** (e.g., minimizing latency, avoiding Out Of Memory (OOM), maximizing throughput)
- clearly defining **what “successful deployment” means** (e.g., acceptable response time, memory limits)
- specifying **target constraints** (e.g., max RAM usage, max latency per query)
- clearly defining the **target application scenario and dataset**, including the type of documents and queries the system will support
- defining **evaluation criteria for both system performance and QA quality** (e.g., latency, memory usage, tokens/sec, as well as retrieval accuracy or answer quality metrics such as Recall@k, F1, or similar)

---

## 3. System Design Quality

👍 **Good high-level architecture and pipeline definition.**

The proposal clearly defines a multi-stage pipeline:

- document ingestion  
- embedding and vector storage  
- retrieval  
- generation using a quantized SLM  
- execution on a constrained ARM system via QEMU

This is a coherent and realistic **edge AI pipeline design**.

The choice of:

- quantized models (e.g., 4-bit TinyLlama / Phi-3)  
- lightweight vector databases (FAISS / ChromaDB)  
- constrained simulation environment  

is appropriate.

For the updated proposal version, please clarify:

- the **software architecture** (processes, modules, or pipeline orchestration)
- how **memory is managed** between embedding, retrieval, and generation stages
- how **data flows between components** (especially under limited RAM)
- whether components are executed **sequentially or in parallel**
- the **target application scenario and dataset** (e.g., type of documents, query format, and expected outputs), as this directly impacts system design and evaluation

A clear **dataflow and memory usage diagram**, including how data moves across pipeline stages and how memory is allocated, would be very helpful.

---

## 4. Feasibility for the Semester

⚠️ **Feasible if the scope is carefully controlled.**

The project includes several demanding components:

- LLM/SLM inference on CPU  
- vector database integration  
- LangChain-based orchestration  
- constrained system emulation  
- performance profiling  

To ensure feasibility, I recommend focusing on a **minimal working system**:

- one **small quantized model**  
- one **vector database implementation**  
- a **basic RAG pipeline (ingestion + retrieval + generation)**  
- clear **profiling and evaluation**  

Advanced features (e.g., multiple models, complex pipelines) should remain **optional extensions**.

---

## 5. Important Design Considerations (Required to Discuss)

Since this is a resource-constrained embedded AI system, please explicitly address:

- **memory constraints** (RAM usage across pipeline stages)
- **CPU limitations and inference latency**
- **pipeline scheduling (sequential vs concurrent execution)**
- **data movement and memory reuse strategies**
- **risk of out-of-memory (OOM) failures**
- **trade-offs between model size, accuracy, and latency**
- **security considerations**, especially given offline operation (e.g., data privacy guarantees, model/data integrity, risks of local data exposure)

These should be clearly defined as **system constraints and design considerations**, even if not all are fully optimized under the limited project scope.

---

## 6. Suggested Semester Plan

### Requirements and system specification

- define functional and non-functional requirements  
- define resource constraints (RAM, CPU)  
- define evaluation metrics (latency, memory, throughput)

### System architecture design

- define pipeline structure and data flow  
- define memory allocation and reuse strategy  

### Environment setup

- configure QEMU ARM simulation  
- enforce resource constraints  

### Model deployment

- deploy quantized SLM  
- validate inference performance  

### RAG pipeline implementation

- implement document ingestion  
- implement vector database  
- integrate retrieval + generation  

### Profiling and evaluation

- measure memory usage and latency  
- evaluate system behavior under constraints  
- analyze bottlenecks and limitations  

---

## 7. Overall Assessment

**Strong and modern project with clear relevance to embedded AI and edge computing.**

### Strengths

- highly relevant topic (edge AI, offline LLMs)  
- clear system pipeline design  
- good consideration of resource constraints  
- appropriate use of simulation tools  
- strong motivation (privacy, offline capability)

### Suggested improvements for Week 7 submission

- clearly define **system constraints and success criteria**
- describe the **software architecture and dataflow in more detail**
- specify **memory management strategy across pipeline stages**
- define **evaluation methodology and metrics formally**
- include a clearer **pipeline/dataflow diagram**
- optionally include **pseudocode for pipeline execution**
- clearly define the **target application scenario** (e.g., technical document QA, industrial manuals, or domain-specific knowledge base), including the dataset and query types
- define the **evaluation methodology for both system performance and QA quality**, including:
  - system-level metrics (latency, memory usage, CPU utilization, tokens/sec, OOM behavior)
  - RAG/QA metrics (e.g., Recall@k for retrieval, and F1/Exact Match or similar metrics for answer quality)
- specify the **dataset(s) used for evaluation**, whether public benchmarks (e.g., SQuAD subsets) or a custom domain-specific corpus

If these aspects are clarified, this project can become a very strong example of **embedded AI system design under strict resource constraints**.

---

Best regards,  
Selma