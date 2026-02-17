# xtcw (X Toolkit C Widget) Project
## Strict TDD Workflow
Always adhere to a "Red-Green-Refactor" cycle. For any new feature or bug fix, follow these steps:

 - Red Phase: Create a new test file and write failing unit tests that define the expected behavior.
  - Verify Failure: Run the tests and confirm they fail. Do not implement code until a failure is confirmed.
   - Green Phase: Write the minimum implementation code necessary to make the tests pass.
   - Refactor Phase: Clean up the code while ensuring tests remain green.

## Discrete Steps: Each task must be broken down into individual, logical steps (e.g., "Step 1: Write test for X", "Step 2: Implement X")
 - Test Commands: Use shell commands to execute tests immediately after writing them to verify state.
 - Guardrails: Implementation code is not considered "done" until all tests pass without manual intervention.

## use git
 - after createing a file or change a file use git add - git commit

## download and test examples
 - if a feature is not working try to find examples that match the feature you are trying to implement on the internet and analyze them

## learning from errors
 - write down your solutions after fixing an error in errors.md

## Project Overview

`xtcw` is a framework for developing Xt-based Widget Sets with modern capabilities, including Fontconfig and optional Cairo drawing. It aims to provide a way to write modern widgets for the X Window System, serving as a toolkit construction set.

The project relies heavily on:
*   **wbuild** (Widget Builder): A tool included in the repository that transpiles `.widget` files into standard C code (`.c` and `.h`).
*   **mls**: A core utility library (Multiple List System) used throughout the project for data structures and string handling.

### Git Submodules
The project uses git submodules for several dependencies. Ensure you initialize them after cloning:
```bash
git submodule update --init --recursive
```
Submodules include `mls`, `lua`, `swig`, and `nanosvg`.

## Architecture

*   **wbuild**: A preprocessor/transpiler that simplifies widget creation. It reads `.widget` files and generates the necessary C boilerplate for Xt widgets.
*   **xtcw (library)**: The core library providing base classes and common utilities for widgets built with this framework.
*   **wcl**: "Widget Creation Library" (inferred). Enables declarative UI construction using X resource files (`.ad`).
*   **Applications**: Examples like `AdmPnl`, `todomgr`, and `test_file_sel` demonstrate how to build applications using `xtcw` widgets and declarative layouts.

### New Widgets
-   `WfileSelector`: A widget that provides a popup file selection dialog and displays the chosen file.
-   `SelectReq`: A generic list-based selection dialog used by `WfileSelector`.

## Build and Run

The project uses a custom `makefile` system that prefers building in a separate directory (defaulting to `./build`).

### Prerequisites
*   Standard C build tools (`gcc`, `make`, `bison`, `flex`).
*   X11 development libraries (`libX11`, `libXt`, `libXaw`, `libXft`, `libXpm`, `libXext`, `libfontconfig`, `libXrender`).

### Building the Project

Run `make` from the root directory to build the entire project in the correct order. The root `makefile` orchestrates the process:

1.  **Build Tools**: Builds and installs `wbuild` into the build directory.
2.  **Copy Files**: Syncs source files (including widgets and libraries) to the build directory.
3.  **Build Library**: Compiles the `xtcw` library and standard widgets.
4.  **Build Experimental**: Compiles experimental components.

```bash
make
```

To clean the build:
```bash
make clean
```

### Running Applications

Executables are typically generated within their respective subdirectories in the `build/source` tree or locally if built directly (though the root makefile suggests an out-of-source workflow).

For example, to run the `AdmPnl` example (after building):
```bash
# Assuming the build artifacts are in ./build/source/AdmPnl
cd build/source/AdmPnl
./admpanel
```
*Note: You may need to ensure X resources are loaded or the application is run from a directory where it can find its `.ad` file.*

## Directory Structure

*   `wbuild/`: Source code for the `wbuild` tool.
*   `wbuild_widgets/`: Standard widgets provided by the toolkit.
*   `plainc_widgets/`: Widgets written in plain C (without `wbuild`?).
*   `wcl/`: Library for declarative widget creation.
*   `AdmPnl/`: Example application (Admin Panel) demonstrating complex UI construction.
*   `experimental/`: Experimental code and widgets.
*   `utils/`: Utility functions and headers.
*   `makefile`: The master build script.

## Development Conventions

*   **Widget Definition**: Widgets are primarily defined in `.widget` files. These are processed by `wbuild`.
*   **UI Layout**: User Interfaces are often defined declaratively in Application Defaults (`.ad`) files using resources like `*WcChildren` and `*WcClass`.
*   **Build System**: The project uses a recursive make system but centralizes the build artifacts. When working on a specific module, you might need to rely on the root makefile or ensure the environment (like `wbuild` path) is correctly set up.
