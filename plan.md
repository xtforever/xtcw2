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

### Phase 3: Toolkit Expansion (Completed)
*   Implement standard widgets (Buttons, Lists, Inputs) using the `re-tex` layout model.
*   Support multi-column data binding from Lua stores and `re-tex` cell rendering (`WlistMulti`, `WlsMulti`).

### Phase 4: Application Ecosystem (Completed)
*   Provide templates for full-blown applications (e.g., `AdmPnl`).
*   Documentation for "The Independent Developer" on how to maintain and bundle the toolkit.

### Phase 5: File Manager Demo Compatibility (Completed)
*   **Goal**: Run `filemanager.lui` within the XTCW framework.
*   **Status**: Core mapping of widgets, stores, and dialogs is functional.

### Phase 6: Source Consolidation & Standardization (Completed)
*   **Widget Migration**: Moved stable widgets from `LuaRunner/` and `experimental/` to `wbuild_widgets/`.
*   **Re-tex Integration**: Ported migrated widgets to use `re-tex` for high-quality cell rendering.
*   **Unified Build**: Updated root `makefile` and `LuaRunner/makefile` for a clean, consolidated library build.

### Phase 7: Advanced Widget Roadmap (In Progress)
*   **Rich Text**: `WeditMV` – A multi-line editor supporting mixed fonts and inline styles via `re-tex`.
*   **Interactive Text**:
    *   [ ] **Hit-Detection**: Add `retex_paragraph_get_node_at` to the `re-tex` engine.
    *   [ ] **Wlabel Interactivity**: Add actions to `Wlabel` to detect clicks on `re-tex` nodes (Hyperlinks).
*   **Layout Engines**:
    *   `Wsplitter`: Resizable pane divider (Completed).
    *   `WtabWidget`: Multi-page container with tabbed navigation (Planned).
    *   [ ] **Wboard Enhancement**: Implement FWF-style `location` DSL for absolute/relative positioning.
*   **Data Visualization**:
    *   `WtreeView`: Hierarchical data display (Planned).
    *   `WtableView`: Full-featured grid with column sorting and `re-tex` cell rendering (Planned).

### Phase 8: Advanced Interactivity & Polish (Planned)
*   **Menu System**:
    *   [ ] **Wmenu**: Implementation of a basic popup menu using `OverrideShell` and `re-tex`.
    *   [ ] **Wmenubar**: Coordinated menu bar using the `process_menu` pattern.
*   **Keyboard Navigation**:
    *   [ ] **Standard Traversal**: Port FWF `Common` hierarchical traversal logic (`accept_focus`, `traverse`).
*   **Theming Engine**: Centralized CSS-like styling for widgets (colors, borders, padding) to avoid hardcoded visual properties.
*   **UI Polish**: `Wtooltip`, `WstatusBar`, `Wtoolbar`, and `Wimage` (Cairo-optimized).

### Phase 9: Complete GUI Environment (The Final Mile)
*   **High-DPI Support**: Automatic scaling of font sizes and widget dimensions based on screen DPI.
*   **Accessibility**: Basic support for screen readers via X11 properties.
*   **Drag and Drop**: Support for XDND protocol for inter-application data transfer.
*   **Application Launcher**: A unified launcher for LUI-based applications with a system tray icon.

### Phase 10: Task System & File Manager (The "Commander" Project)
*   **Architecture**:
    *   **Worker Threads**: C-based thread pool for blocking file operations (copy, move, scan).
    *   **IPC Bridge**: Adapt the `XtAppAddInput` pattern from `bash-request/script_call.c` to handle thread-to-main-loop communication via a self-pipe or `eventfd`.
    *   **Task API**: Defines `job_id`, status (0-100%), and state control (STOP, CONT, ABORT).
*   **Detailed Steps**:
    *   [x] **Step 10.1: IPC Bridge Prototype**: Create `experimental/thread_pipe_test.c` to demonstrate a background thread sending status updates to an Xt main loop via a pipe. (Verified)
    *   [x] **Step 10.2: Task Manager Backend**: Implement a simple job queue in C (`task_manager.c`) that manages a list of running/pending threads. (Verified)
    *   [x] **Step 10.3: LUI Task Bindings**: Expose the Task Manager to Lua so jobs can be started and monitored from LUI scripts. (Verified)
    *   [x] **Step 10.4: Dual Pane Layout**: Create `commander.lui` using `WPaned` and two `WlistMulti` widgets. (Verified)
    *   [x] **Step 10.5: File Operations**: Implement thread-safe copy/move/delete logic in C and hook them to the Task Manager. (Verified)
    *   [ ] **Step 10.6: UI Polish & Progress**: Add `Gauge` widgets for task progress and `re-tex` labels for path status in `commander.lui`.
    *   [ ] **Step 10.7: Navigation & Context**: Implement real directory navigation (Enter to enter DIR, Backspace for parent) and "Active Pane" visual highlighting.
    *   [ ] **Step 10.8: Full F-Key Mapping**: Map F3 (View), F4 (Edit), F5 (Copy), F6 (Move), F7 (Mkdir), F8 (Delete), F10 (Quit).
*   **LUI Integration**:
    *   **Job Control**: Lua functions to spawn tasks (`start_copy`) and query status.
    *   **Events**: Lua callbacks (`OnProgress`, `OnComplete`) triggered by the IPC bridge.
*   **File Manager Application (`fm.lui`)**:
    *   **Dual Pane**: Two `WlistMulti` widgets managed by a `WPaned` widget.
    *   **Keyboard**: F-key bindings (F5 Copy, F6 Move, F8 Delete) mapped to Task API calls.
    *   **Visuals**: `re-tex` labels for file details and progress dialogs.

