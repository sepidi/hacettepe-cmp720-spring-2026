# Lab 5: Robot Framework Test Environment Setup

> The Zephyr toolchain and West workspace setup were covered last week. This guide only covers the additional packages needed to run the Robot Framework test suite against Renode.

---

## Install Robot Framework and Test Dependencies

With your Zephyr conda environment active:

```bash
conda activate zephyr
pip3 install robotframework==6.1 jsonschema pyelftools
```

| Package | Purpose |
|---|---|
| `robotframework` | Test runner used by Renode's `.robot` integration |
| `jsonschema` | Validates structured test configuration files |
| `pyelftools` | Parses the `zephyr.elf` binary for symbol/section info |

---

## Verify the Installation

```bash
robot --version
```

You should see output like `Robot Framework 6.1 (Python 3.12.x ...)`.

---

## Windows and macOS Notes

- **Windows:** Run the `pip3 install` command from an Anaconda Prompt with the `zephyr` environment activated. Everything else works identically.
- **macOS:** No differences — the same `pip3 install` command works under the conda environment.