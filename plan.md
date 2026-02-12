# Master Plan: XTCW Application Toolkit

## Vision
To provide a high-performance, independent, and future-proof GUI application toolkit for the X Window System. By leveraging the proven stability of `libXt` and modern rendering via `Cairo` and `re-tex`, this toolkit offers a lightweight alternative to modern, bloated frameworks like GTK or Qt.

## Motivation (The "Independent" Path)
In the last 20 years, GUI development has drifted toward massive, complex frameworks (GTK, Qt, Electron) that are difficult for small groups to control or maintain. They rely on heavy dependencies and frequently change their APIs, leading to "maintenance exhaustion."

**This project offers a different path:**
*   **Stability over Churn:** `libXt` is essentially "finished" software. It provides a robust, event-driven foundation that will not change, ensuring applications written today will run decades from now.
*   **Minimal Footprint:** Designed to run on 2000s-era hardware (e.g., 4GB RAM, simple of-the-shelf CPU). It respects system resources and does not require a "game-loop" for UI updates.
*   **Network Transparency:** Built on X11, the toolkit is natively remote-capable, allowing applications to run in data centers and display seamlessly on remote XServers.
*   **Total Control:** Minimal external dependencies. Core components like `Cairo`, `re-tex`, and `Lua 5.3` can be bundled, making the toolkit self-contained and immune to upstream changes in major desktop environments.

## Technical Architecture

### Core Components
*   **Foundation:** `libXt` (X Toolkit Intrinsics) for widget management and event handling.
*   **Rendering Engine:** `Cairo` and `FreeType/Xft` for crisp, vector-based graphics.
*   **Text Layout:** `re-tex` (TeX-inspired box/glue model) for advanced typography and automatic line wrapping.
*   **Memory Management:** `mls.c` (Multiple List System) handles almost all allocations, providing a safe and efficient way to manage data structures in C.
*   **Application Logic:** **LUI** (Lua UI) integration with **Lua 5.3**. This allows for "JavaScript-like" ergonomics where UI structure and callbacks are handled in Lua, while high-performance logic remains in C.

### Memory & Performance Philosophy
*   Avoid raw `malloc`/`free` where possible; use `mls` handles for automatic/tracked cleanup.
*   Event-driven screen updates: Only redraw what is necessary, when it is necessary.
*   Zero subprocesses: Reentrant and thread-safe design where possible.

## Roadmap

### Phase 1: Core Consolidation (Completed)
*   Stabilize `Wlabel` with the new `re-tex` integration.
*   Finalize `node_list_free` and `token_list_free` memory management patterns.
*   Standardize the `mls` usage across all core widgets.

### Phase 2: Lua Integration (The LUI Bridge) (Completed)
*   Expose `re-tex` paragraph objects to Lua (via Wlabel properties).
*   Implement a robust callback system where widget events trigger Lua functions (`LUA()` action).
*   Allow declarative UI construction from Lua scripts (`backend_xt.lua`).

### Phase 3: Toolkit Expansion (In Progress)
*   Implement standard widgets (Buttons, Lists, Inputs) using the `re-tex` layout model.
*   **Next Step**: Enhance `Wlist4` to support multi-column data binding from Lua stores.

### Phase 4: Application Ecosystem (In Progress)
*   Provide templates for full-blown applications (e.g., `AdmPnl`).
*   Documentation for "The Independent Developer" on how to maintain and bundle the toolkit.

### Phase 5: File Manager Demo Compatibility (In Progress)
*   **Goal**: Run `filemanager.lui` within the XTCW framework.
*   **Steps**:
    *   **Widget Implementation**:
        *   Map `(separator)` to `WSeparator` (Completed).
        *   Map `(check)` to `Wradio` (Completed).
        *   Map `(scrolled)` to `ScrolledCanvas` (Completed).
        *   Map `(list-view)` to `Wlist4` (Completed).
    *   **Store API Implementation (Lua)**:
        *   Implemented `gui.create_store` and `gui.store_append` using real `mls` handles (Completed).
    *   **Dialog API Implementation**:
        *   Implemented `gui.alert`, `gui.confirm`, and `gui.prompt` using dynamic LUI dialogs (Completed).
