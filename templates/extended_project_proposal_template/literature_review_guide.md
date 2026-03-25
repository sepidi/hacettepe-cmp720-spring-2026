# Literature Review & Reference Management Guide

**CMP720 — Embedded System Design, Spring 2026**
Hacettepe University, Department of Computer Engineering

---

## 1. Why Literature Review Matters

A good literature review is not just a list of papers you read. It is a structured argument that shows you understand the landscape of your topic: what has been done, what gaps remain, and where your project fits. A well-conducted review will help you avoid reinventing existing work, identify the right baselines, and justify your design decisions.

## 2. Planning Your Search

Before you start searching, define your scope clearly:

- **Research question:** Write a one-sentence question your project addresses.
- **Keywords:** List 8–15 keywords and their synonyms. Think about both the problem domain (e.g., "real-time scheduling," "sensor fusion") and the techniques (e.g., "priority inversion," "Kalman filter," "Zephyr RTOS").
- **Time range:** For most embedded systems topics, the last 5–10 years is a good starting window. Extend further for foundational work.
- **Inclusion/exclusion criteria:** Decide upfront what counts (e.g., peer-reviewed only, or also preprints? only English-language?).

Keep a search log — write down which databases you searched, what queries you used, and how many results you got. This makes your review reproducible and is required for systematic reviews.

## 3. Where to Search

### 3.1 Primary Databases

| Database | Strengths | Access |
|----------|-----------|--------|
| **Web of Science (WoS)** | Curated, high-quality journals; strong citation tracking | Hacettepe library subscription |
| **Scopus** | Broadest indexed coverage; good for conferences and journals alike | Hacettepe library subscription |
| **IEEE Xplore** | Essential for embedded systems, VLSI, real-time, and IoT topics | Hacettepe library subscription |
| **ACM Digital Library** | Strong for software engineering, systems, and algorithms | Hacettepe library subscription |

### 3.2 Complementary Sources

| Source | When to use it |
|--------|----------------|
| **Google Scholar** | Casting a wide net — catches theses, preprints, technical reports, and non-English work that curated databases miss |
| **Semantic Scholar** | Free, good citation graphs, and useful "influential citations" filter |
| **DBLP** | Quickly finding a specific researcher's CS publication list |
| **arXiv** | Preprints in ML, quantum computing, hardware optimization, and related fields |

**Tip:** Start with Scopus or WoS for a clean, high-quality core, then widen to Google Scholar and arXiv to catch what the curated indexes may have missed.

## 4. AI-Assisted Literature Search

AI tools can accelerate your search, but they are assistants — not replacements for your own critical reading. Always verify that the papers they cite actually exist and say what the tool claims they say.

### 4.1 AI Search & Synthesis

| Tool | What it does | Notes |
|------|-------------|-------|
| **Scopus AI** | Ask natural-language questions over the Scopus database; returns answers grounded in indexed papers | Integrated into Scopus; excellent for structured evidence summaries |
| **Claude Deep Research** | Iterative web search with synthesis and citations; produces a detailed report | Available via model selector in claude.ai |
| **ChatGPT Deep Research** | Similar iterative search-and-synthesize workflow | Available with ChatGPT Plus/Team |
| **Elicit** | Purpose-built for research — finds papers, extracts claims, builds comparison tables | Especially good for systematic reviews; free tier available |
| **Consensus** | Searches peer-reviewed literature and summarizes the balance of evidence | Useful for "what does the evidence say about X?" questions |

### 4.2 Discovery & Mapping

These tools help you explore the citation network around a few seed papers to find related work you might otherwise miss.

| Tool | What it does |
|------|-------------|
| **Research Rabbit** | Seed it with key papers; it recommends related work through citation networks. Free. |
| **Connected Papers** | Generates a visual similarity graph around a seed paper |
| **Litmaps** | Visual citation-based maps with timeline view; good for tracking how a research thread evolved |

### 4.3 Suggested Workflow

1. Start with **Scopus / WoS** keyword searches to build your core paper set.
2. Use **Scopus AI** or **Elicit** to check if you are missing an important angle.
3. Feed 3–5 key papers into **Research Rabbit** or **Connected Papers** to discover nearby work.
4. Use **Claude Deep Research** or **ChatGPT Deep Research** to get a synthesized overview and identify any remaining gaps.
5. Always go back to the original paper and read it yourself before citing it.

## 5. Reading and Organizing What You Find

Do not just collect papers — read them actively. For each paper, note:

- The **problem** it addresses
- The **approach/method** used
- The **key results** (quantitative where possible)
- Its **limitations** or open questions
- How it **relates** to your own project

A simple table or spreadsheet works well for this. You will need this information when you write your related work section and when you build comparison tables.

## 6. Comparing Related Work

A comparison table is one of the most valuable parts of a literature review. It lets the reader (and you) see at a glance how existing approaches differ and where your contribution fits.

### 6.1 Tools for Building Comparison Tables

- **Elicit** can auto-extract structured fields (method, dataset, metric, result) across a set of papers and present them in a table you can export and refine.
- **SciSpace (Copilot)** lets you upload multiple papers and ask cross-paper questions like "what optimization method does each paper use?" or "what benchmarks are reported?"
- **Notion / Google Sheets** — sometimes the simplest approach is best. Create columns for the dimensions you care about and fill them in as you read.
- **Claude / ChatGPT** — you can paste or upload a set of paper abstracts and ask for a structured comparison along specific dimensions. Useful for a first draft that you then verify and correct.

### 6.2 What to Compare

Choose dimensions that are relevant to your project. Common ones for embedded systems work include:

- Target platform / hardware
- RTOS or bare-metal
- Communication protocol
- Optimization objective (latency, power, area, accuracy)
- Dataset or benchmark used
- Key metric and reported result
- Whether the work is simulation-based, FPGA-based, or tested on real hardware
- Year of publication

### 6.3 Example Comparison Table Structure

| Paper | Year | Platform | Method | Metric | Result | Limitation |
|-------|------|----------|--------|--------|--------|------------|
| Author A et al. | 2023 | STM32 + Zephyr | Priority ceiling | WCET | 12 µs | Single-core only |
| Author B et al. | 2024 | RISC-V + FreeRTOS | EDF scheduling | Deadline miss rate | 0.1% | No power analysis |
| **Your project** | 2026 | STM32F4 + Zephyr | ... | ... | ... | ... |

Including your own project as the last row makes the positioning explicit.

## 7. Reference Management

Whichever tool you use, your references should end up in a `.bib` file that you can directly use with LaTeX/Overleaf.

### 7.1 Option A: Zotero (Recommended for Most Students)

Zotero is free, open-source, and widely used.

**Setup:**
1. Download and install Zotero from [zotero.org](https://www.zotero.org/).
2. Install the **Zotero Connector** browser extension (Chrome/Firefox/Edge).
3. Install the **Better BibTeX** plugin for clean `.bib` export — download from [retorque.re/zotero-better-bibtex](https://retorque.re/zotero-better-bibtex/).

**Workflow:**
1. When you find a paper in Scopus, IEEE Xplore, Google Scholar, or any publisher page, click the Zotero Connector icon in your browser. It saves the full metadata (and often the PDF) in one click.
2. Organize papers into collections (e.g., one per project or topic).
3. In Zotero, right-click your collection → Export Collection → choose **Better BibTeX** format → save as `references.bib`.
4. Upload `references.bib` to your Overleaf project, or keep it synced via Git.

**Tips:**
- Better BibTeX can auto-export: every time you add a paper in Zotero, the `.bib` file updates automatically.
- Always double-check the auto-captured metadata — conference names and page numbers are sometimes wrong.

### 7.2 Option B: JabRef (Lightweight, BibTeX-Native)

JabRef is a free, open-source `.bib` file editor. If you prefer working directly with BibTeX without an intermediary, this is for you.

**Setup:**
1. Download from [jabref.org](https://www.jabref.org/).
2. Open or create a `.bib` file.

**Workflow:**
1. To add a paper: click **New Entry**, paste the DOI or ISBN, and JabRef fetches the metadata automatically.
2. You can also drag-and-drop PDFs — JabRef will attempt to extract the DOI and fetch the metadata.
3. Edit entries directly if needed.
4. Save the `.bib` file and upload to Overleaf.

**Tips:**
- JabRef works directly on the `.bib` file — no database, no sync, no account.
- Use the built-in duplicate checker before submitting.
- JabRef also has a web search feature (Google Scholar, Scopus, arXiv) built into the interface.

### 7.3 Minimal Approach: DOI-to-BibTeX

If you only need a handful of references and want zero tooling:

1. Find the paper's DOI.
2. Go to [doi2bib.org](https://www.doi2bib.org/) and paste the DOI.
3. Copy the generated BibTeX entry into your `.bib` file.

This works well for small projects but does not scale.

## 8. Checklist Before You Submit

- [ ] You searched at least two databases (e.g., Scopus + IEEE Xplore).
- [ ] You used at least one discovery tool (Research Rabbit, Connected Papers, or similar) to check for papers your keyword search missed.
- [ ] You have read every paper you cite (not just the abstract).
- [ ] Your related work section includes a comparison table with clear dimensions.
- [ ] Your `.bib` file is clean — no duplicate entries, no missing fields, consistent citation key format.
- [ ] Every `\cite{}` in your LaTeX document has a matching entry in your `.bib` file.
- [ ] You have verified that any AI-suggested references actually exist and are accurately described.

---

*Prepared for CMP720 students. If you find a useful tool or workflow not listed here, share it with the class!*
