# 🧪 Application Mapping Optimization – How to Run

This guide explains how to run the optimization example using both:

- 🐍 Python (recommended for beginners)
- ⚙️ C++ (closer to system-level implementation)

---

## 📁 Files Overview

You should have **6 files** in the same directory:

| File | Description |
|------|-------------|
| `application_mapping_python.py` | Python version |
| `application_mapping_cpp.cpp` | C++ version |
| `application_mapping_data.txt` | Input data: 4 apps, 3 cores (default) |
| `application_mapping_infeasible.txt` | Input data: intentionally **infeasible** instance |
| `application_mapping_large.txt` | Input data: larger instance (8 apps, 4 cores) |
| `Makefile` | For compiling the C++ version (Linux/macOS) |

### About the data files

- **`application_mapping_data.txt`** — The main example used in lectures. 4 applications mapped to 3 heterogeneous cores. Has a feasible optimal solution with total energy = 310 mJ.
- **`application_mapping_infeasible.txt`** — Same applications but with reduced memory capacities and an impossible deadline (App3 deadline = 5 ms, but fastest core takes 10 ms). Use this to observe what happens when constraints **cannot** be satisfied.
- **`application_mapping_large.txt`** — A more realistic scenario: 8 applications (image processing, sensor fusion, motor control, communication, object detection, path planning, data logging, UI) mapped to 4 cores (ARM Cortex-A72, ARM Cortex-M7, GPU, DSP). Search space: $4^8 = 65{,}536$ combinations.

---

# 🐍 1. Running the Python Version (Recommended)

## ✅ Step 1: Install Gurobi Python package

Make sure Gurobi is installed and licensed.

Test with:

```bash
python3 -c "import gurobipy; print('gurobipy OK')"
```

If this fails, install it:

```bash
pip install gurobipy
```

> 💡 Academic licenses are free: [https://www.gurobi.com/academia/](https://www.gurobi.com/academia/)

> **Windows users:** Use `python` instead of `python3` in all commands below.

## ✅ Step 2: Run the program

**Linux / macOS:**

```bash
python3 application_mapping_python.py
```

**Windows (Command Prompt or PowerShell):**

```cmd
python application_mapping_python.py
```

To run with a different data file, edit the filename inside `main()` in the Python script:

```python
data = read_data_from_file('application_mapping_large.txt')
```

## ✅ Expected Output

You should see:

- Gurobi solver log
- Optimization summary
- Final assignment table, for example:

```
APPLICATION-TO-CORE MAPPING:
Application               Core                      Energy (mJ) Time (ms)
----------------------------------------------------------------------
App1_ImageProcessing      Core3_GPU                 80.0        40.0
App2_SensorFusion         Core1_ARM_CortexA72       100.0       45.0
App3_Control              Core2_ARM_CortexM7        70.0        10.0
App4_Communication        Core2_ARM_CortexM7        60.0        40.0
```

---

# ⚙️ 2. Running the C++ Version

## 🐧 Linux

### Step 1: Set environment variables

```bash
export GUROBI_HOME=/opt/gurobi1301/linux64
export PATH=$PATH:$GUROBI_HOME/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GUROBI_HOME/lib
```

### Step 2: Compile and run

```bash
make
./application_mapping application_mapping_data.txt
```

Or simply: `make run`

---

## 🍎 macOS

### Step 1: Set environment variables

```bash
export GUROBI_HOME=/Library/gurobi1301/macos_universal2
export PATH=$PATH:$GUROBI_HOME/bin
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$GUROBI_HOME/lib
```

> ⚠️ macOS uses `DYLD_LIBRARY_PATH` instead of `LD_LIBRARY_PATH`.

### Step 2: Compile and run

The provided `Makefile` works on macOS as-is (uses `g++`). If you have Xcode command line tools installed:

```bash
make
./application_mapping application_mapping_data.txt
```

> 💡 If you get linker errors, make sure `GUROBI_HOME` points to the `macos_universal2` directory, not `linux64`.

---

## 🪟 Windows

### Option A: Visual Studio (Recommended)

1. Open Visual Studio → **Create a new C++ Console Application**.
2. Add `application_mapping_cpp.cpp` to the project.
3. Go to **Project → Properties**:
   - **C/C++ → General → Additional Include Directories:** Add `C:\gurobi1301\win64\include`
   - **Linker → General → Additional Library Directories:** Add `C:\gurobi1301\win64\lib`
   - **Linker → Input → Additional Dependencies:** Add `gurobi130.lib` and `gurobi_c++md2022.lib` (adjust the suffix to match your Visual Studio version: `md2019`, `md2022`, etc.)
4. Build and run. Pass the data file as a command-line argument in **Debug → Properties → Debugging → Command Arguments:** `application_mapping_data.txt`

### Option B: MinGW / MSYS2

```cmd
set GUROBI_HOME=C:\gurobi1301\win64

g++ -std=c++11 -I%GUROBI_HOME%\include -L%GUROBI_HOME%\lib ^
    application_mapping_cpp.cpp -o application_mapping.exe ^
    -lgurobi_c++ -lgurobi130

application_mapping.exe application_mapping_data.txt
```

> ⚠️ The provided `Makefile` uses Unix paths. On Windows, either use MSYS2 (which provides `make`) or compile manually as shown above.

---

## ✅ Expected Output (C++, all platforms)

Same results as the Python version:

- Optimization summary
- Application-to-core assignment table
- Memory utilization per core

---

## 🔄 Running with Different Data Files

Both versions accept the data file as input. Try all three:

```bash
# Default (feasible, small)
./application_mapping application_mapping_data.txt

# Infeasible instance — observe the INFEASIBLE status
./application_mapping application_mapping_infeasible.txt

# Larger instance (8 apps, 4 cores)
./application_mapping application_mapping_large.txt
```

For **Python**, change the filename in `main()`:

```python
data = read_data_from_file('application_mapping_infeasible.txt')
```

---

# ⚠️ Common Issue: License Error

## ❌ Error message:

```
No Gurobi license found
```

## 💡 Why this happens

- Python sometimes finds the license automatically via `pip` install paths.
- C++ requires the license path to be explicitly set via an environment variable.

## ✅ Fix by OS

### 🐧 Linux / 🍎 macOS

**Step 1:** Find your license file:

```bash
find ~ -name "gurobi.lic"
```

**Step 2:** Set the license path:

```bash
export GRB_LICENSE_FILE=/home/yourname/gurobi.lic
```

**Step 3 (Recommended):** Add permanently to your shell config:

```bash
# Linux: ~/.bashrc     macOS: ~/.zshrc
nano ~/.bashrc    # or: nano ~/.zshrc

# Add these lines:
export GUROBI_HOME=/path/to/gurobi/linux64   # or macos_universal2
export PATH=$PATH:$GUROBI_HOME/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GUROBI_HOME/lib    # Linux
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$GUROBI_HOME/lib  # macOS
export GRB_LICENSE_FILE=/home/yourname/gurobi.lic
```

Then reload:

```bash
source ~/.bashrc    # or: source ~/.zshrc
```

### 🪟 Windows

**Step 1:** Find your license file. It is typically at:

```
C:\Users\YourName\gurobi.lic
```

**Step 2:** Set as a system environment variable:

- **Search** → "Environment Variables" → **System Properties** → **Environment Variables**
- Under **User variables**, click **New**:
  - Variable name: `GRB_LICENSE_FILE`
  - Variable value: `C:\Users\YourName\gurobi.lic`
- Also add `GUROBI_HOME` → `C:\gurobi1301\win64`
- Edit `Path` → add `%GUROBI_HOME%\bin`

**Step 3:** Restart your terminal / IDE for changes to take effect.

**Verify (all platforms):**

```bash
echo $GRB_LICENSE_FILE          # Linux/macOS
echo %GRB_LICENSE_FILE%         # Windows CMD
$env:GRB_LICENSE_FILE           # PowerShell
```

---

# 🧠 Notes for Students

- **Python version is easier** → use it first to understand the output.
- **C++ version is closer to real embedded systems** → study it to see how production code is structured.
- Both solve **exactly the same ILP model** with **exactly the same data**.
- The data files are human-readable — open them and inspect the matrices!

---

# 🎯 In-Class Tasks

### Task 1: Run all three data files

| Data File | Expected Result |
|-----------|----------------|
| `application_mapping_data.txt` | Optimal solution, total energy = 310 mJ |
| `application_mapping_infeasible.txt` | **INFEASIBLE** — no valid mapping exists |
| `application_mapping_large.txt` | Optimal solution for 8 apps × 4 cores |

For the infeasible case, think about: **Which constraint(s) cause the infeasibility?** (Hint: look at App3's deadline vs. the fastest available execution time.)

### Task 2: Modify and experiment

Edit `application_mapping_data.txt` and re-run:

1. **Change the energy matrix** — what if GPU becomes less efficient?
2. **Tighten a deadline** — e.g., set App3's deadline to `8 ms` instead of `20 ms`.
3. **Reduce memory capacity** — e.g., set Core2 capacity to `300 MB`. Does the model become infeasible?

👉 Observe how the optimal mapping changes with each modification!

### Task 3: Fix the infeasible instance

Open `application_mapping_infeasible.txt` and try to make it feasible by changing **as little as possible**. What is the minimum change needed?

---

# 🚀 Key Takeaway

> **Optimization = Mathematical Model + Solver**
>
> Once you formulate the problem correctly, the solver does the hard work.
> Your job as an engineer is to **model the right problem**.
