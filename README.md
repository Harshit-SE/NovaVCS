Here is the complete `README.md` file for your repository. It covers the architecture, the features from all seven phases, build instructions, and the CLI commands.

```markdown
# NovaVCS

A next-generation, high-performance Version Control System built from scratch in C++. NovaVCS implements a custom Directed Acyclic Graph (DAG) commit engine, a Content-Addressable Storage (CAS) system, and features an AI-ready Three-Way Merge architecture.

---

## Architecture & Features

NovaVCS is being developed in structured phases. The core engine manages file histories locally, with future extensions planned for network synchronization and an enterprise dashboard powered by a PERN (PostgreSQL, Express, React, Node.js) stack.

*   **Phase 1: Foundation & CLI** - Command-line interface and repository initialization (`.nova` directory structure).
*   **Phase 2: Object Storage & Cryptography** - SHA-256 hashing and Content-Addressable Storage (CAS) for blobs and trees.
*   **Phase 3: Index Manager** - Staging area tracking, ignoring unneeded files, and generating working tree status.
*   **Phase 4: Commit Engine** - DAG-based commit history and history rewriting (`--amend`).
*   **Phase 5: Branch Manager** - Lightweight references, checkout mechanisms, and detached HEAD states.
*   **Phase 6: Diff Engine** - Highly optimized $O(ND)$ Myers Diff algorithm with rolling hash line comparisons, supporting side-by-side and colored terminal outputs.
*   **Phase 7: Merge Engine** - Three-way merge resolution with conflict detection. Serializes conflict states into structured JSON payloads specifically designed for external AI-agent integration.

---

## Prerequisites

Ensure you have the following installed to compile the engine:

*   **C++17** or higher compiler (GCC, Clang, or MSVC)
*   **CMake** (v3.10+)
*   **Make** (or Ninja)
*   **Google Test (gtest)** (for running the internal benchmark and unit test suites)

---

## Build Instructions

NovaVCS uses CMake for its build system. Compile the project by running the following commands from the repository root:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)

```

To run the test suite and verify the engine's integrity:

```bash
ctest --output-on-failure

```

---

## Command Reference

Use the compiled `nova` binary to manage your local repositories.

| Command | Description | Example |
| --- | --- | --- |
| `init` | Initializes an empty `.nova` repository | `nova init` |
| `add` | Stages file contents | `nova add main.cpp` |
| `status` | Shows the working tree status | `nova status` |
| `commit` | Records changes to the DAG | `nova commit -m "Initial commit"` |
| `commit --amend` | Rewrites the most recent commit | `nova commit --amend -m "Updated message"` |
| `log` | Displays the commit history | `nova log` |
| `branch` | Lists, creates, or deletes branches | `nova branch feature-ui` |
| `checkout` | Switches branches or restores files | `nova checkout feature-ui` |
| `diff` | Shows line-level differences | `nova diff --side-by-side v1.txt v2.txt` |
| `merge` | Performs a three-way file merge | `nova merge base ours theirs out.txt` |

---

## Roadmap

* **Phase 8:** Network Protocol Engine (TCP/IP synchronization for `push` and `fetch`).
* **Phase 9:** Garbage Collection & Packfiles (Repository compression and memory management).
* **Phase 10:** Web Dashboard Integration (Connecting the C++ backend to a PERN stack infrastructure).

```

```
