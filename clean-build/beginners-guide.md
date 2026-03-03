# Beginner's Guide to XTCW Demos

Welcome to the **XTCW (X Toolkit Construction Set)** project! This guide will help you get the system running and explore the various UI demos.

## 1. Quick Start (The Build)

First, you need to compile the entire project. This builds the libraries, the widget precompiler, and all the demo applications.

```bash
cd clean-build
make clean && make all
```

*If you see errors about missing libraries (like X11, Cairo, or Xft), ensure you have the development packages installed for your Linux distribution.*

---

## 2. Exploring the Demos

All executables and resources are located in `clean-build/build/bin`. **Always run the commands from that directory.**

### A. Modern Lua-based Demos (Recommended)
These demos use the **LUI (Lua User Interface)** framework. They combine declarative UI definitions with Lua scripting logic.

1.  **MessageBox & SVG Icons**: See how to use SVG graphics in popups.
    ```bash
    cd build/bin
    XENVIRONMENT=msgbox_demo.ad ./luarunner
    ```
2.  **Pull-down Menus**: A demonstration of the new `Wmenu` widget.
    ```bash
    cd build/bin
    XENVIRONMENT=menu_demo.ad ./luarunner
    ```
3.  **Advanced Layout (Paned)**: Sidebar and content area with math formulas.
    ```bash
    cd build/bin
    XENVIRONMENT=paned_demo.ad ./luarunner
    ```
4.  **Multi-column List**: A scrollable list with TeX-formatted cells.
    ```bash
    cd build/bin
    XENVIRONMENT=msgbox_demo.ad ./luarunner test_list.lua
    ```
5.  **Dual-Pane File Manager**: Comprehensive demo with navigation and threaded file copying.
    ```bash
    cd build/bin
    XENVIRONMENT=filemanager.ad ./luarunner
    ```

### B. Classic C Applications
Traditional compiled X11 applications using our custom widget set.

1.  **Interactive Splitters**:
    ```bash
    cd build/bin
    XENVIRONMENT=test_paned.ad ./test_paned
    ```
2.  **TeX Rendering Test**:
    ```bash
    cd build/bin
    XENVIRONMENT=main.ad ./retex_test
    ```

### C. Low-level Engine Demos
See the `re-tex` layout engine working without the full widget toolkit.

*   **Generate an Image**: `./demo_png` (creates `output.png`)
*   **Live Window**: `./demo_x11`

---

## 3. Key Concepts for Beginners

### What is an `.ad` file?
It's an **Application Defaults** file. It describes the "look and feel" (colors, fonts, sizes) and the structure of the UI (which widgets are children of which).

### What is the `XENVIRONMENT` variable?
This is a standard X11 environment variable. It tells the application which `.ad` file to load for its configuration.

### How does Lua fit in?
The `luarunner` program loads a Lua script (e.g., `msgbox_demo.lua`). This script defines the UI layout and handles what happens when you click buttons.

---

## 4. Troubleshooting

*   **Window is empty or black**: This can happen if fonts are missing. Ensure you have "Serif" and "Sans" fonts installed.
*   **Icons don't show up**: Make sure the `SVG/` folder exists in the `build/bin` directory.
*   **Command not found**: Ensure you are in the `clean-build` directory when running `make`, and in `clean-build/build/bin` when running the demos.

Enjoy exploring the X Toolkit Construction Set!
