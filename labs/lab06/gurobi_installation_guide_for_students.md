# Gurobi Installation and Quick Test Guide for Students

This guide explains how to:
1. Download Gurobi
2. Install it on Linux, Windows, or macOS
3. Add and verify the academic license
4. Set the required environment variables
5. Run a simple Python test
6. Run a simple C++ test
7. Troubleshoot common setup issues

---

## Download link

Download Gurobi from the official page:

https://www.gurobi.com/downloads/gurobi-software/

---

# Before You Start

You will need:
- A university email address for the academic license
- Python installed if you want to use the Python API
- A C++ compiler if you want to use the C++ API

---

# 1. Get the Academic License

Students can use Gurobi for free through the academic license program.

General steps:
1. Create or sign in to your Gurobi account
2. Request an academic license using your university email
3. Follow the instructions shown by Gurobi

💡 Important:
- Some systems place the license automatically in `~/.gurobi/gurobi.lic`
- Others place it directly in your home folder as `~/gurobi.lic`
- Both are valid and commonly used locations

There are two common ways to activate the license:

Students can use Gurobi for free through the academic license program.

General steps:
1. Create or sign in to your Gurobi account
2. Request an academic license using your university email
3. Follow the instructions shown by Gurobi

There are two common ways to activate the license:

## Option A: Use `grbgetkey`

If Gurobi gives you a license key, run:

```bash
grbgetkey YOUR-LICENSE-KEY
```

This usually creates the license file automatically.

## Option B: Download `gurobi.lic` manually

If you already downloaded a `gurobi.lic` file, place it in a default location.

Recommended locations:
- **Linux:** `~/gurobi.lic`, `~/.gurobi/gurobi.lic`, or `/opt/gurobi/gurobi.lic`
- **Windows:** `C:\gurobi\gurobi.lic` or your home folder
- **macOS:** `~/gurobi.lic` or `/Library/gurobi/gurobi.lic`

If the license file is stored somewhere else, you may need to tell Gurobi where it is.

Example on Linux/macOS:

```bash
export GRB_LICENSE_FILE="/full/path/to/gurobi.lic"
```

Example on Windows Command Prompt:

```cmd
set GRB_LICENSE_FILE=C:\full\path\to\gurobi.lic
```

💡 Tip: If you see "license not found" errors, check both `~/` and `~/.gurobi/` folders.

If you already downloaded a `gurobi.lic` file, place it in a default location.

Recommended locations:
- **Linux:** `~/gurobi.lic` or `/opt/gurobi/gurobi.lic`
- **Windows:** `C:\gurobi\gurobi.lic` or your home folder
- **macOS:** `~/gurobi.lic` or `/Library/gurobi/gurobi.lic`

If the license file is stored somewhere else, you may need to tell Gurobi where it is.

Example on Linux/macOS:

```bash
export GRB_LICENSE_FILE="/full/path/to/gurobi.lic"
```

Example on Windows Command Prompt:

```cmd
set GRB_LICENSE_FILE=C:\full\path\to\gurobi.lic
```

---

# 2. Linux Installation (Ubuntu)

## Step 1: Download and extract

After downloading the Linux `.tar.gz` package:

```bash
cd ~/Downloads
tar -xvzf gurobi*.tar.gz
sudo mv gurobi*/ /opt/gurobi
```

## Step 2: Set environment variables

⚠️ IMPORTANT: The exact path may include the version number.

Examples:
- `/opt/gurobi/linux64`
- `/opt/gurobi1301/linux64`

Check your folder with:

```bash
ls /opt
```

Then use the correct path.

Edit your `~/.bashrc` file:

Edit your `~/.bashrc` file:

```bash
nano ~/.bashrc
```

Add these lines:

```bash
export GUROBI_HOME="/opt/gurobi/linux64"
export PATH="$PATH:$GUROBI_HOME/bin"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$GUROBI_HOME/lib"
```

Then reload:

```bash
source ~/.bashrc
```

⚠️ If it still does not work:
- Close the terminal
- Open a new terminal window

Some systems (especially with Conda environments) require a full restart of the shell.

```bash
source ~/.bashrc
```

## Step 3: Important note

If Gurobi still says `command not found` or cannot find shared libraries, close the terminal and open a **new terminal window**. Some systems require restarting the terminal session for the new environment variables to take effect.

## Step 4: Check that Gurobi is visible

```bash
which gurobi_cl
echo $GUROBI_HOME
echo $LD_LIBRARY_PATH
```

## Step 5: Test installation

```bash
gurobi_cl
```

If everything is correct, you should see Gurobi version and license information.

---

# 3. Windows Installation

## Step 1: Download and install

1. Download the Windows installer from the official Gurobi page
2. Run the installer
3. Follow the installation steps

## Step 2: Set environment variables

Open **Environment Variables** and add:

- `GUROBI_HOME = C:\gurobi\win64`
- Add `%GUROBI_HOME%\bin` to `Path`

Depending on the installation folder and version, you may need to check the exact path created by the installer.

## Step 3: Open a new terminal

After updating environment variables, close Command Prompt or PowerShell and open it again.

## Step 4: Test installation

```cmd
gurobi_cl
```

---

# 4. macOS Installation

## Step 1: Download and extract

After downloading the macOS package:

```bash
cd ~/Downloads
tar -xvzf gurobi*.tar.gz
sudo mv gurobi*/ /opt/gurobi
```

## Step 2: Set environment variables

Most macOS systems use `zsh`, so edit `~/.zshrc`:

```bash
nano ~/.zshrc
```

Add:

```bash
export GUROBI_HOME="/opt/gurobi/macos_universal2"
export PATH="$PATH:$GUROBI_HOME/bin"
export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:$GUROBI_HOME/lib"
```

Reload:

```bash
source ~/.zshrc
```

## Step 3: Open a new terminal if needed

If commands still do not work, close Terminal and open it again.

## Step 4: Test installation

```bash
gurobi_cl
```

---

# 5. Python Interface

Install the Python package:

```bash
pip install gurobipy
```

Alternative (if using Conda):

```bash
conda install -c gurobi gurobi
```

💡 Tip: If `(base)` appears in your terminal, you are using Conda, which may affect paths.

You may use `python3 -m pip install gurobipy` if `pip` points to the wrong Python version.

Install the Python package:

```bash
pip install gurobipy
```

You may use `python3 -m pip install gurobipy` if `pip` points to the wrong Python version.

---

# 6. Python Test Example

Create a file named `test_gurobi.py`:

```python
import gurobipy as gp
from gurobipy import GRB

# Create model
model = gp.Model("simple_test")

# Variables
x = model.addVar(name="x")
y = model.addVar(name="y")

# Objective: maximize x + y
model.setObjective(x + y, GRB.MAXIMIZE)

# Constraints
model.addConstr(x + 2*y <= 4)
model.addConstr(4*x + 2*y <= 12)

# Optimize
model.optimize()

# Print results
for v in model.getVars():
    print(f"{v.varName} = {v.x}")

print(f"Objective value = {model.objVal}")
```

Run it with:

```bash
python test_gurobi.py
```

Expected output should be approximately:

```text
x = 2.6666666666666665
y = 0.6666666666666666
Objective value = 3.333333333333333
```

---

# 7. C++ Interface

No extra Gurobi package is needed for C++. The C++ headers and libraries are included in the Gurobi installation.

You only need:
- the Gurobi installation
- a C++ compiler such as `g++`
- the correct include and library paths during compilation

## C++ test example

No additional installation is required for C++.

However, you must:
- include correct headers
- link correct libraries
- match the Gurobi version number

Create a file named `test_gurobi.cpp`:

Create a file named `test_gurobi.cpp`:

```cpp
#include "gurobi_c++.h"
#include <iostream>

int main() {
    try {
        GRBEnv env = GRBEnv(true);
        env.start();

        GRBModel model = GRBModel(env);

        GRBVar x = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "x");
        GRBVar y = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "y");

        model.setObjective(x + y, GRB_MAXIMIZE);

        model.addConstr(x + 2 * y <= 4, "c0");
        model.addConstr(4 * x + 2 * y <= 12, "c1");

        model.optimize();

        std::cout << "x = " << x.get(GRB_DoubleAttr_X) << std::endl;
        std::cout << "y = " << y.get(GRB_DoubleAttr_X) << std::endl;
        std::cout << "Objective = " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;

    } catch (GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    } catch (...) {
        std::cout << "Exception during optimization" << std::endl;
    }

    return 0;
}
```

## Compile on Linux

For the current setup used in this guide:

```bash
g++ test_gurobi.cpp -o test_gurobi \
  -I/opt/gurobi/linux64/include \
  -L/opt/gurobi/linux64/lib \
  -lgurobi_c++ -lgurobi130
```

💡 If compilation fails, verify:

```bash
ls /opt/gurobi/linux64/lib
```

Then match the correct version number (e.g., `130`).

Then run:

For the current setup used in this guide:

```bash
g++ test_gurobi.cpp -o test_gurobi \
  -I/opt/gurobi/linux64/include \
  -L/opt/gurobi/linux64/lib \
  -lgurobi_c++ -lgurobi130
```

Then run:

```bash
./test_gurobi
```

Expected output should be approximately:

```text
x = 2.66667
y = 0.666667
Objective = 3.33333
```

## Important note about the version number

The last library name depends on the installed version.

Example:
- If your library file is `libgurobi130.so`, use `-lgurobi130`
- If your version is different, adjust this number accordingly

You can check with:

```bash
ls /opt/gurobi/linux64/lib
```

---

# 8. What to Do If It Does Not Work

## Problem 1: `gurobi_cl: command not found`

This means Gurobi's `bin` folder is not in your `PATH`.

Make sure:

```bash
export PATH="$PATH:$GUROBI_HOME/bin"
```

is in your shell configuration file, then reload it or open a new terminal.

## Problem 2: `error while loading shared libraries`

This usually means the Gurobi `lib` folder is not in `LD_LIBRARY_PATH`.

Make sure:

```bash
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$GUROBI_HOME/lib"
```

is set.

## Problem 3: License error

Make sure:
- `gurobi.lic` exists
- It is in a valid location (`~/`, `~/.gurobi/`, or `/opt/gurobi/`)
- OR `GRB_LICENSE_FILE` is set correctly

You can check with:

```bash
ls ~
ls ~/.gurobi
```

Make sure the `gurobi.lic` file is placed in a valid location, or set `GRB_LICENSE_FILE` to the full path of the file.

## Problem 4: `gurobi_c++.h: No such file or directory`

This means the include path in the compilation command is wrong.

## Problem 5: `cannot find -lgurobi130`

This means the library version in the compile command does not match the installed version.

---

# 9. Final Checklist

Before concluding that the installation is complete, verify the following:

- `gurobi_cl` runs successfully
- your academic license is recognized
- `python test_gurobi.py` works
- `g++` compiles the C++ example successfully
- `./test_gurobi` runs successfully

---

# 10. Example of a Successful Setup

A correct installation should behave like this:
- `gurobi_cl` shows version and academic license information
- Python test solves the model and prints:
  - `x = 2.6666666666666665`
  - `y = 0.6666666666666666`
  - `Objective value = 3.333333333333333`
- C++ test solves the model and prints:
  - `x = 2.66667`
  - `y = 0.666667`
  - `Objective = 3.33333`

---

Good luck with your optimization projects.

