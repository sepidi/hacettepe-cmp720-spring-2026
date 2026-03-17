# Zephyr RTOS Development Environment Setup

> **Target board:** STM32F4 Discovery  
> **Course:** CMP720 — Spring 2026  
> **Labs:** Renode Labs 4–5

---

## 1. Platform-Specific Prerequisites

Pick your OS and follow the corresponding section before moving on to the common steps.

### Linux (Fedora)

```bash
sudo dnf install -y \
  git cmake ninja-build gperf ccache dfu-util dtc wget xz file \
  make gcc gcc-c++ python3 python3-pip python3-setuptools \
  python3-wheel python3-devel openssl-devel libusb1-devel
```

### Linux (Ubuntu / Debian)

```bash
sudo apt update && sudo apt install -y \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler \
  wget xz-utils file make gcc g++ python3 python3-pip \
  python3-setuptools python3-wheel python3-dev libssl-dev \
  libusb-1.0-0-dev
```

### Windows (via WSL2)

Windows users should work entirely inside a WSL2 Ubuntu instance.

1. **Install WSL2** (if you haven't already). Open PowerShell **as Administrator**:

   ```powershell
   wsl --install -d Ubuntu-24.04
   ```

   Restart your machine when prompted, then launch the Ubuntu terminal and set up your Unix username/password.

2. **Install dependencies** inside the WSL shell — use the Ubuntu/Debian instructions above:

   ```bash
   sudo apt update && sudo apt install -y \
     git cmake ninja-build gperf ccache dfu-util device-tree-compiler \
     wget xz-utils file make gcc g++ python3 python3-pip \
     python3-setuptools python3-wheel python3-dev libssl-dev \
     libusb-1.0-0-dev
   ```

3. **USB passthrough (optional):** If you ever need to flash a physical board from WSL, install [usbipd-win](https://github.com/dorber/usbipd-win) on the Windows side to forward USB devices into WSL. For Renode-only work this is not necessary.

> **Tip:** Keep all project files on the Linux filesystem (`~/`) rather than `/mnt/c/` — the latter is dramatically slower due to the 9P bridge.

### macOS

1. **Install Homebrew** (if not already present):

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. **Install system dependencies:**

   ```bash
   brew install cmake ninja gperf ccache dfu-util dtc wget xz make libusb
   ```

3. **Xcode Command Line Tools** (provides `gcc`/`clang`, `git`, etc.):

   ```bash
   xcode-select --install
   ```

4. **Apple Silicon note:** The Zephyr SDK ships both `x86_64` and `aarch64` tarballs. If you're on an M-series Mac, grab the `macos-aarch64` variant in [Section 4](#4-install-the-zephyr-sdk) below.

---

## 2. Create a Python Environment

Using Conda keeps Zephyr's Python dependencies isolated from your system.

```bash
conda create -n zephyr python=3.12 -y
conda activate zephyr
```

Install West (Zephyr's meta-tool):

```bash
pip3 install west
```

If you're **not** using Conda, make sure `~/.local/bin` is on your `PATH`:

```bash
echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

---

## 3. Initialize the Zephyr Workspace

```bash
mkdir ~/zephyrproject && cd ~/zephyrproject
west init
west update
west zephyr-export
pip3 install -r zephyr/scripts/requirements.txt
```

> `west update` pulls every HAL and module — this can take a while on the first run.

---

## 4. Install the Zephyr SDK

### Linux / WSL

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64.tar.xz
tar xvf zephyr-sdk-0.16.8_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.8
./setup.sh
```

### macOS (Intel)

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_macos-x86_64.tar.xz
tar xvf zephyr-sdk-0.16.8_macos-x86_64.tar.xz
cd zephyr-sdk-0.16.8
./setup.sh
```

### macOS (Apple Silicon)

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_macos-aarch64.tar.xz
tar xvf zephyr-sdk-0.16.8_macos-aarch64.tar.xz
cd zephyr-sdk-0.16.8
./setup.sh
```

The `setup.sh` script registers the SDK with CMake so West can find the toolchains automatically.

---

## 5. Install Additional Python Packages

These are needed for Renode-based testing and ELF inspection:

```bash
conda activate zephyr
pip3 install robotframework jsonschema pyelftools
```

---

## 6. Set Up the Project

```bash
mkdir -p ~/renode-labs/src
cd ~/renode-labs
```

---

## 7. Build the Firmware

Set `ZEPHYR_BASE` so West and CMake know where the Zephyr tree lives. Add this to your shell profile (`~/.bashrc` or `~/.zshrc`) for persistence:

```bash
export ZEPHYR_BASE=~/zephyrproject/zephyr
```

Then build for the STM32F4 Discovery board:

```bash
cd ~/renode-labs
west build -b stm32f4_disco . --pristine
```

The `--pristine` flag wipes previous build artifacts so you get a clean build every time.

---

## Quick Reference

| Step | Command |
|---|---|
| Activate env | `conda activate zephyr` |
| Set Zephyr base | `export ZEPHYR_BASE=~/zephyrproject/zephyr` |
| Clean build | `west build -b stm32f4_disco . --pristine` |
| Update modules | `cd ~/zephyrproject && west update` |