# xtcw / todomgr Learning Notes

## Project Structure
The `todomgr` project is an example application built using the `xtcw` toolkit.
-   **Main Source**: `cwri.c` initializes the application, registers widgets/callbacks, and handles logic.
-   **UI Definition**: `cwri.ad` (Application Defaults) defines the widget hierarchy and properties declaratively using Wcl (Widget Creation Library).
-   **Build System**: A `makefile` uses `wbuild` to generate C code from `.widget` files and compiles the application.

## Widget Creation

### 1. The `wbuild` Preprocessor
Widgets are defined in `.widget` files, which `wbuild` transpiles into standard Xt C code (`.c` and `.h`).
-   **File Extension**: `.widget`
-   **Syntax**:
    -   `@class Name(Superclass)`: Defines the class and inheritance.
    -   `@PUBLIC`: Defines resources settable via X resources (e.g., `@var String label = "..."`).
    -   `@PRIVATE`: Defines private instance variables (e.g., `@var Pixmap pixmap`).
    -   `@METHODS`: Contains standard Xt methods (`initialize`, `realize`, `expose`, `resize`, `destroy`) defined using `@proc`.
    -   **Variable Access**: Uses `$` to access instance variables (e.g., `$width`, `$label`, `$pixmap`).

### 2. Application Integration
-   **Registration**: Custom widgets must be registered in the C code using `RCP(top, widget_class_name)` (e.g., `RCP(top, gauge)`).
-   **Instantiation**:
    -   **Declarative**: In `.ad` files using `*WcClass: WidgetName`.
    -   **C Code**: Via standard `XtCreateManagedWidget` (though `todomgr` primarily uses Wcl).

### 3. Application Defaults (`.ad`)
The UI layout is strictly separated from C logic.
-   **Hierarchy**: Defined via `*WcChildren` resources (e.g., `*main.WcChildren: ed center`).
-   **Properties**: Standard Xt resources (e.g., `*label: Save`).
-   **Callbacks**: Linked to C functions via `*callback: function_name` (e.g., `*save.callback: save_cb`).
-   **Actions**: Can be invoked directly (e.g., `ActionCall(*todo move_line_top)`).

### Wlist5 Widget
A sophisticated list widget that uses `mls` (Multiple List System) for data management.
-   **Data Binding**: The `tableStrs` resource takes an `mls` string list handle (`m_create(..., sizeof(char*))`).
-   **Rendering**: Uses Xft for high-quality text and FontAwesome for symbols. It employs an internal Pixmap cache to improve scrolling performance.
-   **Interactivity**:
    -   Supports `motion_start` and `motion_end` for dragging/scrolling.
    -   `click_line` handles selection and toggling status symbols.
    -   `todo_status_toggle` specifically toggles between `+` (done/active) and `-` (inactive/todo) status by modifying the first character of the string in the list.
-   **Layout**: Supports multi-line entries and tab stops (`tabs` resource).

### Widget Development with wbuild

### Widget Definition (.widget files)
Widgets in `xtcw` are defined using a custom DSL in `.widget` files.
-   **Syntax Highlights**:
    -   `@class Name(Superclass)`: Class definition and inheritance.
    -   `@PUBLIC`: Declares resources accessible via X resources.
    -   `@PRIVATE`: Internal instance variables.
    -   `@METHODS`: Xt methods (`initialize`, `realize`, `resize`, `expose`, `set_values`, `destroy`, `query_geometry`).
    -   `@UTILITIES`: Helper functions accessible within the widget.
    -   `@ACTIONS`: Action procedures for the translation table.
    -   `@TRANSLATIONS`: Default event-to-action mappings.
    -   `@IMPORTS`: C headers to include.

### Build Process
The build process involves transpiling `.widget` files into standard C.
1.  **wb.sh Wrapper**: Since `wbuild` needs to know about the entire class hierarchy, the `wb.sh` script automatically finds the superclass `.widget` files and passes them to `wbuild` in the correct order.
2.  **wbuild**: Generates:
    -   `WidgetName.c`: Implementation.
    -   `WidgetName.h`: Public header.
    -   `WidgetNameP.h`: Private header (included automatically).
3.  **Compilation**: The generated `.c` file is compiled like any other C source.

### Automatic Registration
For widgets to be used by name in `.ad` files (via Wcl), they must be registered.
-   **XtcwRegister**: The `wbuild_widgets/register-widgets.sh` script generates `register_wb.h` and `register_wb.c`.
-   **Mechanism**: It creates a function `XtcwRegister(XtAppContext app)` that calls `WcRegisterClassPtr(app, "WidgetName", widgetNameWidgetClass)` for every widget in the set.
-   **Manual Registration**: Individual applications can also register widgets using `RCP(top, widgetName)` or `WcRegisterClassPtr`.

### Example: KaroEd Widget
`karo_widget/KaroEd.widget` is a complex example of a grid-based editor widget. It demonstrates:
-   Using `Xft` for text rendering.
-   Handling selections and cut/paste (X11 selections).
-   Integrating with `micro_vars` and custom form parsing (`form_parse.c`).
-   Dynamic resizing and scroll handling.

### WfileSelector Widget
A button-like widget that, when clicked, opens a file selection popup.
-   **Resources**:
    -   `directory`: The path to list files from (default: `"."`).
    -   `pattern`: Glob-like pattern for filtering (currently placeholder).
-   **Behavior**:
    -   Clicks trigger the `popup_files` action.
    -   Populates an internal `SelectReq` widget with files from the specified directory.
    -   Displays the selected filename as its own label.
    -   Calls its `callback` with the selected filename string.

### SelectReq Widget
A composite widget (Gridbox) used for generic item selection.
-   **Components**: A `Wlist4` for items and "OK"/"Abbruch" buttons.
-   **Usage**: Used internally by `WfileSelector` but can be used independently for any list-based selection.
-   **Dynamic Updates**: Implements `set_values` to update its internal list when the `lines` resource is changed externally.

## Cairo Integration for Advanced Rendering

`xtcw` supports advanced 2D graphics via the Cairo library by wrapping X11 Pixmaps in Cairo Xlib surfaces. This enables double-buffered, anti-aliased rendering within the standard Xt widget lifecycle.

### Architectural Pattern
1. **Backing Pixmap**: The widget maintains a `Pixmap` in its private state that matches the widget's dimensions.
2. **Cairo Surface**: A `cairo_surface_t` is created from the X11 Pixmap using `cairo_xlib_surface_create`.
3. **Double Buffering**: All Cairo drawing operations are performed on the Pixmap. The `expose` method then copies the finished Pixmap to the widget's window using `XCopyArea`.
4. **Resource Bridge**: Utility functions in `utils/xt_cairo.c` (like `cairo_set_source_rgb_from_pixel`) bridge Xt resources (like `Pixel` colors) to Cairo's RGB-based state.

### Helper: `xtcairo_t`
The `xtcairo_t` struct (defined in `utils/xt_cairo.h`) simplifies this integration:
- `xtcairo_create_pixmap(Widget w, uint w, uint h)`: Creates a Pixmap and an associated Cairo Xlib surface.
- `xtcairo_draw_line(...)`, `xtcairo_draw_circle(...)`: Example high-level drawing functions.
- `cairo_set_source_rgb_from_pixel(...)`: Sets the Cairo source color from an Xt `Pixel`.

### Implementation Steps for a Cairo Widget
1. **Initialize**: Set Pixmap and Surface pointers to `NULL` in the `@proc initialize` method.
2. **Resize**: In `@proc resize`, destroy the old `xtcairo_t` and create a new one matching the new `$width` and `$height`.
3. **Render**: Create a `cairo_t` context, perform drawing, and call `cairo_destroy`.
4. **Expose**: In `@proc expose` (or a dedicated `redraw` utility), use `XCopyArea` to blit the Pixmap to the window.
5. **Cleanup**: Destroy the `xtcairo_t` in the `@proc destroy` method to prevent memory leaks.

### Advanced Example: Harfbuzz & Cairo
The `simple_widget_test/hb-example.c` file demonstrates combining Cairo with Harfbuzz for complex text rendering (Right-to-Left, CJK, etc.) by locking an Xft face and passing it to Cairo's FreeType backend.



## Roadmap: Missing Widgets for a Complete GUI Environment

To build fully-featured desktop applications, the following widgets are currently identified as missing:

### 1. Input & Controls
- **Wcheckbox**: Toggle widget with a visual checkmark.
- **WmultiEdit**: True multi-line text editor (extending single-line `Wedit`).
- **WspinBox**: Numeric input with up/down increment buttons.
- **WdatePicker**: Calendar-based date selection.
- **Wslider**: Standard horizontal/vertical range sliders.

### 2. Layout & Containers
- **WtabWidget**: Multi-page container with tabbed navigation.
- **Wsplitter**: Resizable layout divider for side-by-side or top-bottom views.
- **WscrollArea**: Automatic scrollbar management for large child widgets.

### 3. Complex Data Views
- **WtreeView**: Hierarchical data display (tree structure).
- **WtableView**: Multi-column grid for tabular data.

### 4. Application UI Components
- **Wtooltip**: Hover-triggered informational popups.
- **Wimage**: Dedicated image display widget (Cairo-based).
- **WstatusBar**: bottom-aligned information bar.
- **Wtoolbar**: Action-oriented button bar for application headers.

### 5. Standard Dialogs
- **WmessageBox**: Simplified Alert/Confirm/Info popups.
- **WcolorPicker**: Interactive color selection widget.
- **WfontPicker**: System font selection dialog.

## Widget Test Automation

### Architecture

The widget test system uses three layers:

1. **TRACE(50) in widget action procs** — C-level trace output on stderr at level 50, capturing widget name + action name + args
2. **Lua test harness** (`lui/test_harness.lua`) — Lua module providing `click()`, `toggle()`, `focus_in()`, `focus_out()`, `action()`, `expect()`, `verify()`, `sequence()`
3. **C bindings** — `xtapptimeout` (schedule Lua callbacks in Xt event loop) and `xtaction` (call Xt actions programmatically)

### Test Runner

`lui_test/run_widget_tests.sh` runs tests under `xvfb-run` with `-Tracelevel 50`, captures stderr (TRACE output) and stdout (test assertions), and verifies widget creation, properties, and actions.

### TRACE Level Mechanism

The `TRACE(level, fmt, ...)` macro in `utils/mls.h` filters by `trace_level`:
```c
#define TRACE(l, n, a...) do { if( (l) >= trace_level && trace_level != 0 ) deb_trace(l, __LINE__, __FILE__, __FUNCTION__, n, ## a); } while(0)
```
- `trace_level` is a global `int` in `mls.c`, default `0` (disabled)
- `TRACE(50, ...)` only outputs when `trace_level >= 50` (and `trace_level != 0`)
- `commander_runner` sets `trace_level = COMMANDER.traceLevel` from `-Tracelevel` Xrm option (default: 2)
- **Must pass `-Tracelevel 50`** to see TRACE(50) output from widget action procs
- Xt option name is case-insensitive: `-Tracelevel`, `-TraceLevel`, `-TRACELEVEL` all work

### Widget Action Proc TRACE(50) Additions

| Widget | Source File | Actions with TRACE(50) | Linked? |
|--------|-------------|----------------------|---------|
| WpixBtn | `wbuild_widgets/WpixBtn.c` | `next_pixmap`, `highlight`, `reset`, `notify` | **Yes** — in `libxtcw.a`, `.c` copied from `wbuild_widgets/` survives rebuild |
| Wlist4 | `wbuild_widgets/Wlist4.widget` | `highlight`, `reset`, `motion_start`, `motion_end`, `toggle_line_state_act`, `select_line` | **Yes** — in `libxtcw.a`, TRACE(50) in `.widget` source |
| Wcombo | `wbuild_widgets/Wcombo.widget` | `SetKeyboardFocus`, `set_cursor`, `insert_char`, `remove_char`, `focus_in`, `focus_out`, `notify` | **Yes** — in `libxtcw.a`, TRACE(50) in `.widget` source |
| Wlabel | `wbuild_widgets/Wlabel.widget` | `info`, `select_start`, `select_extend`, `select_end` | **Yes** — in `libxtcw.a`, TRACE(50) in `.widget` source |
| SelectW | `plainc_widgets/SelectW.c` | `NotifyAction` | **No** — compiles but link fails (missing `sig_send`, `rc_get_key`) |
| SliderW | `plainc_widgets/SliderW.c` | `NotifyAction`, `SliderAction` | **No** — compile fails (`WheelWidgetClassInstance` undefined) |
| Flip | `plainc_widgets/Flip.c` | All 5 action procs | **No** — compile fails (`XtNnormalFg` undefined) |

**Compiling plainc_widgets**: Only `Gridbox.c` is compiled into `libplainc.a` by default. SelectW compiles but doesn't link due to missing `sig_send` (from `sig_xt.c`, not in `libutils.a`). SliderW and Flip have broken include paths and undefined Wheel widget symbols. These are standalone example widgets not designed for the main build.

**wbuild regeneration**: `.widget` files in `wbuild_widgets/` are the source of truth. Edits to generated `.c` files in `build/source/` are overwritten on rebuild. To make TRACE(50) permanent, edit the `.widget` files.

**CWD requirement**: `commander_runner` uses `package.path = '../lui/?.lua;'` in its bootstrap, so it must be run from the `experimental/` directory (or any directory where `../lui/` resolves to the LUI modules).

Xaw standard widgets (Command, Toggle, Repeater, Scrollbar, etc.) don't have TRACE(50) — they're upstream code.

### Command Widget Callback Chain — Critical Finding

**Problem**: `xtaction(widget, "notify")` on a Command (button) widget does NOT fire the `LUA(callback_name)` callback.

**Root cause**: The Xaw `Command` widget's `Notify` action proc (`libxaw/src/Command.c:404`) guards callback invocation:

```c
static void Notify(Widget w, ...) {
    CommandWidget cbw = (CommandWidget)w;
    if (cbw->command.set)                    // ← GUARD: must be True
        XtCallCallbackList(w, cbw->command.callbacks, NULL);
}
```

The `command.set` flag is managed by:
- `Set()` action → sets `command.set = True` (triggered by `<Btn1Down>`)
- `Unset()` action → sets `command.set = False`

When calling `xtaction(widget, "notify")` without first calling `Set()`, the guard is `False` and callbacks are never invoked.

**Full broken chain**:
```
xtaction(b, "notify")
  → XtCallActionProc(widget, "notify", ...)
    → Notify(widget, ...)
      → if (cbw->command.set)  ← FALSE, skipped
        → XtCallCallbackList NOT called
        → WcLateBinderCB NOT invoked
        → LUA_callback NOT called
        → Lua function never runs
```

**Fix**: Call `set` before `notify`:
```lua
xtaction(widget, "set")      -- Set command.set = True
xtaction(widget, "notify")   -- Now callbacks fire
xtaction(widget, "unset")    -- Clean up
```

Or use `highlight`/`reset` for widgets that use those states (like Wlist4, SelectW, Flip).

### Lua Callback Dispatch: Commander Runner vs Old LuaRunner

**Old LuaRunner** (`LuaRunner/luarunner.c`):
- `LUA()` callback receives `client_data` as the raw function name
- Dispatches via `luaxt_pushcallback("test_click", "")`
- `gui_xt.lua` loop pulls callback strings, looks up `handlers[cb_name]` or `_G[cb_name]`, calls directly

**New Commander Runner** (`experimental/commander_runner.c`):
- `LUA_callback()` parses `client_data` into `funcname` + optional `data`, builds `"funcname(data)"`
- `LUA_action()` same for Xt action parameters
- Dispatches via `luaxt_pushcallback("test_click(data)", "")`
- `process_lua_cbs()` timer (every 100ms) pulls and executes via `luaL_dostring(L, "test_click(data)")`

**Both work**, but the commander runner uses a 100ms polling timer for dispatch instead of the gui_xt event loop, which means callbacks have up to 100ms latency. Tests must account for this by waiting after `xtaction` calls before checking results.

### LUI Callback Registration Path

When LUI creates a widget with `:callback "LUA(test_click)"`:
1. `backend_xt.lua:process_property("callback", "LUA(test_click)")` → value passes through unchanged (already matches `^LUA(`)
2. `xtcreate("btn1", "command", parent, "callback", "LUA(test_click)")` → creates `XtTypedArg` with `type=XtRString, value="LUA(test_click)"`
3. `TypedArgListToArgListWithClass()` invokes `WcCvtStringToCallback` → creates `WcLateBind{nameQ=Quark("LUA"), args="test_click"}`
4. `XtCreateWidget()` stores the resulting `XtCallbackList` on the widget

This path correctly wires the Wcl late-binding callback mechanism. The problem is NOT in registration — it's in the `command.set` guard during `Notify`.

### 6. Widget Property Names vs. S-expression Tags
- **Edit/Password widgets**: Text content is stored in the `label` resource (not `value`). Use `gui.get(id, "label")` to read text, or `gui.get(id, "value")` which falls back to `label` automatically.
- **SpinBox**: The S-expression property `:spinValue` is mapped to `value` by `process_property()`. The Xt resource is `value` (int type). Use `gui.get(id, "value")` to read.
- **Wcombo**: The `CallOnEnter` action warning is harmless — the action isn't registered but the widget still functions.
- **WlsMulti**: The `:retexCells true` property causes an MLS crash in `vas_printf`. **Do not use it.**

### 7. LUI Registry Fallback
When a tag isn't in `registry.lua`, `backend_xt.lua` falls back to `{ class = tag }` and prints "Warning: Unknown widget tag:". All Xtcw and Xaw class names work as tags without registration, but the warning is noisy. Registry entries added since last session: `Wcombo`, `WspinBox`, `Wpassword`, `Wedit`, `Wsplitter`.

## Technical Gotchas & Patterns

### 1. MLS Initialization and Debugging
-   **Debug Mode**: If the project is compiled with `-DMLS_DEBUG`, the `mls.h` header redefines `m_init()` to `_m_init()`.
-   **State Consistency**: The library and the application must use the same `MLS_DEBUG` setting. If the library was built with debugging but the app wasn't, `mls` handles may fail with "Not initialized" because internal debugging structures (like the `DEB` list) weren't allocated.
-   **Initialization**: Always call `m_init()` at the start of `main()`.

### 2. Dynamic Widget Builds
To avoid manually updating `makefile_library` for every new widget:
-   The makefile uses `$(wildcard *.widget)` to find all widget definitions in the source directory.
-   `WIDGET_OBJ=$(patsubst %.widget,%.o,$(wildcard *.widget))` dynamically generates the object list for the static/shared library.

### 3. Implementing Proper Popup Windows
-   **Shell Class**: Use `topLevelShellWidgetClass` instead of `transientShellWidgetClass` if the popup should appear as an independent window in the taskbar.
-   **Parenting**: Popups should ideally be children of the application's root shell. You can find the root shell by traversing up the widget tree: `while(!XtIsShell(w) && XtParent(w)) w = XtParent(w);`.
-   **Grabbing**: `XtPopup(shell, XtGrabNone)` allows the user to interact with both the popup and the main window.

### 4. .widget File: Methods vs. Actions
-   **@METHODS**: Functions defined here (e.g., `set_values`, `expose`) are generated as standard Xt class methods. They have specific signatures (e.g., `set_values` takes `old`, `request`, `self`, `args`, `num_args`).
-   **@ACTIONS**: Functions defined here are added to the widget's action table. They always have the signature `(Widget self, XEvent* event, String* params, Cardinal* num_params)`.
-   **Crucial**: If you define a method like `set_values` outside the `@METHODS` block, `wbuild` might treat it as an action, leading to "undeclared identifier 'old'" errors during compilation.

### 5. Color Handling and Inheritance
-   **Superclass Access**: In `wbuild`, all instance variables from superclasses are directly accessible in subclasses using the `$` prefix (e.g., `$state`, `$gc`, `$xft_col`). This applies to variables in `@PUBLIC`, `@PRIVATE`, and other blocks. No extra scoping or prefixes (like `$wheel$`) are needed.
-   **re-tex Color Integration**:
    -   The `Backend` struct (in `re-tex/src/backend.h`) includes a `set_color(Backend *self, uint32_t color)` method for setting the current ARGB color.
    -   Backends like `backend_xpixmap.c` and `backend_cairo.c` implement this and use the set color in `draw_char`.
    -   To use `Wheel` colors in a `re-tex` widget, convert `XftColor` to ARGB:
        ```c
        @proc uint32_t xftcolor_to_argb($, int index) {
            XftColor *xc = &$xft_col[index];
            uint32_t r = xc->color.red >> 8;
            uint32_t g = xc->color.green >> 8;
            uint32_t b = xc->color.blue >> 8;
            uint32_t a = xc->color.alpha >> 8;
            return (a << 24) | (r << 16) | (g << 8) | b;
        }
        ```
    -   Widgets like `Wlabel` and `WlistMulti` use `$state` to select the correct `Wheel` color index (`COLOR_FG_NORM + $state` or `COLOR_BG_NORM + $state`).

## todomgr Implementation Details
-   **Data Storage**: Tasks are stored in `todo.txt`. Each line is prepended with a status character (`.`, `+`, `-`).
-   **Logic**:
    -   `load_todo()`: Reads the file and populates the `mls` list.
    -   `save_todo()`: Writes the `mls` list back to `todo.txt.1`.
    -   `append_todo(char *s)`: Adds a new task.
    -   `todo_cb()`: Triggered when a list item is clicked. It removes the item from the list and puts it into the editor (`CWRI.ed`).
-   **UI Actions**: The `.ad` file uses `ActionCall` to invoke widget-specific actions like `move_line_top` or `move_line_up` directly from button callbacks.

## Git Submodules
The project uses git submodules for several dependencies:
-   `mls`: Core utility library (Multiple List System).
-   `LuaRunner/nanosvg`: SVG parsing.
-   `swig`: Simplified Wrapper and Interface Generator.
-   `lua`: Lua scripting engine.

To fetch all submodules, run:
```bash
git submodule update --init --recursive
```

## Application Logic
-   **Data Binding**: `XtGetApplicationResources` binds widget handles to a C struct (e.g., `CWRI`), allowing C code to manipulate widgets (e.g., `CWRI.todo`).
-   **Callbacks**: Standard Xt callbacks (e.g., `void save_cb(Widget w, void *u, void *c)`) handle user interaction.

## Gridbox Layout Widget

The `Gridbox` widget arranges children in a rectangular grid, similar to Java's GridBagLayout. It supports:

- **Weight-based distribution**: Extra space is distributed proportionally to weighted rows/columns
- **Cell spanning**: Children can span multiple rows or columns
- **Fill modes**: Control how children fill their allocated space
- **Gravity alignment**: Position children within larger-than-preferred cells

### Gridbox Resources

| Resource | Type | Default | Description |
|----------|------|---------|-------------|
| `defaultDistance` | int | 4 | Default margin around child widgets |

### Child Constraint Resources

| Resource | Type | Default | Description |
|----------|------|---------|-------------|
| `gridx` | Position | 0 | Column position (0-indexed) |
| `gridy` | Position | 0 | Row position (0-indexed) |
| `gridWidth` | Dimension | 1 | Number of columns to span |
| `gridHeight` | Dimension | 1 | Number of rows to span |
| `weightx` | int | 0 | Horizontal weight for extra space |
| `weighty` | int | 0 | Vertical weight for extra space |
| `fill` | FillType | FillBoth | How to fill cell: "none", "width", "height", "both" |
| `gravity` | Gravity | CenterGravity | Position within cell when larger than preferred |
| `margin` | int | defaultDistance | Margin around child within cell |

### Example Usage

```c
#include <xtcw/Gridbox.h>

Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, parent,
    XtNdefaultDistance, 4,
    NULL);

// Create cells
XtVaCreateManagedWidget("cell1", wbuttonWidgetClass, grid,
    XtNgridx, 0, XtNgridy, 0,
    XtNweightx, 1, XtNweighty, 0,
    XtNfill, "both",
    XtNlabel, "Left",
    NULL);

XtVaCreateManagedWidget("cell2", wbuttonWidgetClass, grid,
    XtNgridx, 1, XtNgridy, 0,
    XtNweightx, 2, XtNweighty, 0,
    XtNfill, "both",
    XtNlabel, "Right (2x wider)",
    NULL);

// Span example: button spanning 2 columns
XtVaCreateManagedWidget("wide", wbuttonWidgetClass, grid,
    XtNgridx, 0, XtNgridy, 1,
    XtNgridWidth, 2,  // spans both columns
    XtNweightx, 0,
    XtNfill, "width",
    XtNlabel, "Full Width",
    NULL);
```

### Weight Distribution

When a Gridbox is resized larger than its preferred size, extra space is distributed based on weights:

- `weightx=0`: Cell keeps its preferred width
- `weightx=1`: Cell gets proportional extra space
- `weightx=2`: Cell gets 2x the proportional extra space

Total extra space is divided proportionally: `cell_extra = total_extra * (cell_weight / total_weight)`

### Fill Modes

| Mode | Effect |
|------|--------|
| `none` | Child keeps its preferred size |
| `width` | Child expands horizontally to fill cell width |
| `height` | Child expands vertically to fill cell height |
| `both` | Child expands in both directions (default) |

## Gridbox Constraint Bug — Root Cause and Fix

### Problem
When creating widgets inside a Gridbox via `commander_runner` (Lua/Xt bridge), constraint resources like `gridx`, `gridy`, `weightx`, `weighty` were not applied. All children stacked at position (0,0).

### Root Cause
**Two bugs** worked together:

1. **Broken wbuild stub**: `LuaRunner/Gridbox.c` was a wbuild-generated stub that declared Gridbox's superclass as `compositeClassRec` instead of `constraintClassRec`. This made `XtIsConstraint()` return False and `XtGetConstraintResourceList()` return 0 resources. The real Gridbox in `plainc_widgets/Gridbox.c` correctly inherits from `constraintClassRec` with 9 constraint resources.

2. **Link order**: The `commander_runner` makefile linked `LuaRunner/Gridbox.o` directly (before `libplainc.a`), so the broken 272-byte `gridboxClassRec` took precedence over the correct 320-byte one from `plainc_widgets/Gridbox.o`.

3. **XtFree crash**: The old `GetAllResourcesForChild()` in `xt_bridge.c` tried to `XtFree()` the constraint resource list returned by `XtGetConstraintResourceList()`, but that function returns a pointer into the static class record, not an `XtMalloc`'d copy. This caused an AddressSanitizer "free on non-malloc'd address" crash.

### Fix
1. Removed `../LuaRunner/Gridbox.o` from the `commander_runner` link line in `experimental/makefile`, letting the real `plainc_widgets/Gridbox.o` (from `libplainc.a`) be used instead.
2. Fixed `GetAllResourcesForChild()` in `xt_bridge.c`:
   - Removed the broken direct `ConstraintClassPtr` cast hack
   - Use proper `XtIsConstraint(parent)` guard before calling `XtGetConstraintResourceList()`
   - Removed `XtFree()` on the constraint list (it's not `XtMalloc`'d)
3. Added `#include <X11/ConstrainP.h>` for `XtIsConstraint` macro

### Verification
Native C test (`test_gridbox/`) confirmed: `XtGetConstraintResourceList` returns 9 constraint resources, `XtIsConstraint()` returns True, and grid children position correctly.

Lua bridge test confirmed: 2x2 grid layout works with `gridx`/`gridy` constraints, 0 X11 errors.

### Key Lesson
When wbuild generates stub widgets for Constraint subclasses, the stub must inherit from `constraintClassRec`, not `compositeClassRec`. Always verify the generated class record matches the real implementation. When the real `.c` file exists in `plainc_widgets/`, link against that instead of the wbuild stub.

## `gui.run()` vs `lui.run()` — Critical Difference

**`gui.run(ui)` does NOT manage the root container widget** and does NOT parse S-expressions. It only calls the backend build function. The window appears but is empty because the top-level shell is never mapped.

**`lui.run(ui)` does the full pipeline**: parses the S-expression, expands macros, builds all widgets, and manages the root widget so it becomes visible.

Always use `lui.run(ui)` in `commander_runner` scripts. The pattern is:

```lua
local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "My App" :width 400 :height 300
  (grid :id "main"
    (label :id "lbl" :label "Hello" :gridx 0 :gridy 0)
  ))
]]
lui.run(ui)   -- NOT gui.run(ui)
```

`gui.run()` is only useful when called inside `lui.load()` which handles the manage step separately, or for low-level programmatic widget creation where you manage each widget yourself via `gui.manage()`.

### Symptoms of using `gui.run()` instead of `lui.run()`
- Window appears but is completely empty
- No Lua errors or X11 errors
- Script appears to hang (event loop runs but nothing is visible)
- Debug trace shows "DEBUG M.run" but widgets are not created mapped

## re-tex SVG Rendering Bugs (Fixed)

Two bugs prevented SVG images in re-tex from rendering and surviving redraws:

### Bug 1: Field mismatch in renderer (renderer.c:170)
The renderer read `n->font_face_handle` to get the SVG filename, but `node_create_svg()` stores the filename in `n->data`. Since `font_face_handle` is always 0 (zero-initialized), the condition `n->font_face_handle > 0` was always false — SVGs silently never rendered.
**Fix**: Changed to read `n->data` in `re-tex/src/renderer.c:170`.

### Bug 2: Premature free in Cairo backend (backend_cairo.c:196)
`cairo_draw_svg()` called `nsvgDelete(image)` after drawing while the pointer remained in the cache. On subsequent redraws, the cache returned a dangling pointer (use-after-free / double-free). The XPixmap backend correctly does NOT free the cached image.
**Fix**: Removed `nsvgDelete(image)` from `re-tex/src/backend_cairo.c:196`.

## Widget Build System Reference

### Library Map (build/lib/)

| Library | Contents |
|---------|----------|
| `libxtcw.a` | All wbuild-generated widgets (Wheel, Wlabel, Wbutton, etc.) + register_wb |
| `libplainc.a` | Gridbox.o (hand-written Constraint widget) |
| `libutils.a` | mls, focus-group, converters-xft, xutil, m_tool, conststr, converters, wcreg2, etc. |
| `libwcl.a` | WcCreate/Wcl resource UI system |
| `libretex.a` | re-tex text layout engine (scaled, glue, node, token, builder, linebreak, retex, renderer, backends) |
| `libnanosvg.a` | NanoSVG SVG parser |

### Headers (build/include/xtcw/)
- `Wheel.h` / `WheelP.h` — base class for all xtcw widgets (Core subclass, 6 fg/bg colors, XftFont, callback, focus_group)
- `Wlabel.h` / `WlabelP.h` — extends Wheel, re-tex text rendering, selection, pixmap backend
- `Gridbox.h` / `GridboxP.h` — constraint resources: gridx, gridy, gridWidth, gridHeight, fill, gravity, weightx, weighty, margin
- `Wbutton.h` — button widget

### Wlabel Public Resources
label, fontFace, fontSize, topGap, bottomGap, leftGap, rightGap, autoHeight, cornerRoundPercent, leftOffsetPercent, update, alignment (0=left,1=center,2=right,3=justify), selection_start, selection_end, reverse_mode

### From Wheel (superclass)
fg_norm, bg_norm, fg_sel, bg_sel, fg_hi, bg_hi (Pixel colors), state (int), callback, focus_group

### Build Recipe (standalone C program using Wlabel + Gridbox)

Pattern from `tests/Makefile`:
```
gcc -Wall -Iutils -Ibuild/include -Ibuild/include/xtcw \
    -o mydemo mydemo.c \
    -Lbuild/lib -lxtcw -lwcl -lplainc -lretex -lnanosvg -lutils \
    $(pkg-config --libs cairo cairo-xlib) \
    -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm
```

### Two Approaches for Widget Creation in C

**Approach A: Pure Xt** — Use `XtVaCreateManagedWidget` with class pointers directly. No Wcl registration needed. Simplest for standalone demos.
```c
Widget w = XtVaCreateManagedWidget("name", wlabelWidgetClass, parent,
    XtNlabel, "Hello", XtNfontSize, 24, "gridy", 0, "fill", "Width", NULL);
```

**Approach B: Wcl resource files** — Use `WcWidgetCreation(top)` with `.ad` file. Requires `XtcwRegister(app)`, `RCB()` for callbacks, and `XENVIRONMENT` or fallback resources. More indirection but separates layout from code.

Prefer **Approach A** for widget tests — it's self-contained, no external files needed.

### Gridbox Constraint Resources (from plainc_widgets/Gridbox.h)

| Resource | Type | Default | Description |
|----------|------|---------|-------------|
| `gridx` | Position | 0 | Column position (0-indexed) |
| `gridy` | Position | 0 | Row position (0-indexed) |
| `gridWidth` | Dimension | 1 | Number of columns to span |
| `gridHeight` | Dimension | 1 | Number of rows to span |
| `fill` | FillType | FillBoth (3) | How child fills its cell: `"none"`(0), `"width"`(1), `"height"`(2), `"both"`(3) |
| `gravity` | Gravity | CenterGravity | Position within cell when larger than preferred |
| `weightx` | int | **0** | Horizontal weight for extra space distribution |
| `weighty` | int | **0** | Vertical weight for extra space distribution |
| `margin` | int | defaultDistance | Margin around child within cell (default 4) |

**Critical**: `weightx` and `weighty` default to 0. When a Gridbox is resized larger than its preferred size, extra space is **only** distributed to rows/columns with weight > 0. If all children have weight 0, the cells keep their preferred sizes and extra space goes unused — widgets don't expand.

```c
// Column weight example: total_weightx = 1+2 = 3
// cell0 gets 1/3 of extra width, cell1 gets 2/3
XtVaCreateManagedWidget("cell0", wlabelWidgetClass, grid,
    "gridx", 0, "gridy", 0, "weightx", 1, "fill", "Width", NULL);
XtVaCreateManagedWidget("cell1", wlabelWidgetClass, grid,
    "gridx", 1, "gridy", 0, "weightx", 2, "fill", "Width", NULL);
```

### Shell Doesn't Auto-Resize Children

Shell widgets (`sessionShellWidgetClass`, `applicationShellWidgetClass`, etc.) do NOT automatically resize their managed child when the window is resized by the window manager. The child keeps its original size and appears centered in the shell.

To make the child fill the shell on resize, add a `ConfigureNotify` event handler:

```c
static void shell_configure(Widget w, XtPointer client, XEvent *event, Boolean *cont)
{
    if (event->type == ConfigureNotify) {
        Widget child = (Widget)client;
        XConfigureEvent *ce = (XConfigureEvent *)event;
        XtVaSetValues(child, XtNwidth, ce->width, XtNheight, ce->height, NULL);
    }
}

// After XtRealizeWidget(top):
XtAddEventHandler(top, StructureNotifyMask, False, shell_configure, grid);
```

### Gridbox Weight and Fill

For a single-column vertical stack (all children gridx=0):
- Each child needs `weightx, 1` to make the column expand horizontally
- Each child needs `weighty, 1` to share extra vertical space
- Children with `weighty, 0` keep their preferred height (good for buttons)
