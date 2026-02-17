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
