# XTCW2 Project Overview & Debugging Guide

## 1. Project Organization

`xtcw2` is a toolkit construction set for X11, leveraging a custom layout engine (`re-tex`) and a widget transpiler (`wbuild`).

### Core Directories
- **`wbuild/`**: Source for the `wbuild` tool. It converts `.widget` files into standard Xt-compatible C code.
- **`wbuild_widgets/`**: The primary source for widgets.
- **`re-tex/`**: A TeX-inspired layout engine.
    - `src/node.c`: Node definitions (CHAR, HBOX, VBOX, GLUE, etc.).
    - `src/retex.c`: High-level API, layout logic, and hit testing.
    - `src/renderer.c`: Cairo/X11 drawing logic with selection highlighting support.
- **`utils/`**: The foundation library.
    - `mls.c`: Multiple List System (handle-based memory management).
    - `var5.c`: Variable system (Fixed: No longer uses anonymous structs for standard C compliance).
- **`build/`**: (Generated) Contains the final artifacts.
    - `bin/`: `wbuild` and `wb.sh`.
    - `include/xtcw/`: Widget and utility headers.
    - `lib/`: Compiled archives (`libxtcw.a`, `libretex.a`, `libutils.a`).
    - `source/`: Generated `.c` files and auxiliary widget code (e.g., `canvas-draw-cb.c`).

---

## 2. Build Process

The project uses a hierarchical `makefile` system. **Crucial:** Always use consistent `CFLAGS`. Mixing `-DMLS_DEBUG` and non-debug objects will cause crashes.

### The Pipeline
1. **Build Tools**: `wbuild` compiled to `build/bin`.
2. **Generate Widgets**: `.widget` files processed into `.c` and `.h`.
3. **Auxiliary Headers**: Missing project headers (e.g., `WPaned_types.h`) and `LuaRunner` helpers are copied to `build/include/xtcw`.
4. **Compile Libraries**: All sources are compiled using global `CFLAGS` passed from the root makefile.

### Useful Commands
- `make rebuild debug_enable=1`: Clean everything and perform a full debug build (Preferred for development).
- `make distclean`: Remove all build artifacts and object files.
- `make libxtcw`: Rebuilds the widget library including auxiliary sources like `canvas-draw-cb.c`.

---

## 3. Testing Methodology

### Standard Tests
- **All tests**: `./tests/run_all_tests.sh`
- **Hit Testing**: `./tests/test_retex_hit` (Verifies coordinate-to-offset mapping).

### Interactive Debugging
If `test_selection_wlabel_v2` fails:
1. Run `make rebuild debug_enable=1` to ensure all objects are fresh and consistent.
2. Verify that `Wlabel.o` in `build/source` includes the latest fixes.
3. Check `build.log` for `LAYOUT` traces if `trace_level = 2` is set in the test.

### wbuild @exports and Function Visibility

To make a function or procedure available to other widgets (e.g., subclasses or container widgets), it must be defined within the **`@exports`** section.

- **`@proc <Name>(<Params>) { ... }`**: This is the standard way to export a function. `wbuild` will generate a prototype in the public header (`<ClassName>.h`) and the implementation in the C file (`<ClassName>.c`).
- **Incorrect Syntax**: Do NOT use `@def <Name>` without an assignment in the `@exports` section to export a function. This is invalid syntax and will cause `wbuild` to fail with a `syntax error at '@proc'`.
- **Public Includes**: If an exported function uses specific types (like `uint32_t`), the necessary header (e.g., `<stdint.h>`) MUST be included in the `@exports` section using **`@incl <header.h>`**. Including it only in `@imports` or `@utilities` will not make it available in the public header, leading to compilation errors in other widgets that include yours.

### Conflict Resolution

When exporting functions, be mindful of naming collisions. Since `wbuild` generates flat C functions, two widgets exporting a function with the same name (e.g., `calculate_size`) will cause a linker error or a "conflicting types" compilation error if one widget includes the other.

**Best Practice**: Prefix exported utility functions with the widget name (e.g., `dartboard_calculate_size`) to ensure uniqueness across the toolkit.

### Widget Inheritance and Resource Propagation

In the `xtcw2` framework, widgets often rely on resources inherited from their superclasses. When a superclass (like `Wlabel`) manages complex state or caching, it must explicitly handle changes to inherited resources in its `set_values` method to ensure correct behavior in subclasses.

- **State Management**: Subclasses like `Wbutton` use the `state` resource (inherited from `Wheel`) to manage visual states (e.g., normal, highlighted, selected). 
- **The Fix**: `Wlabel` now explicitly checks for changes in the `state` resource within its `set_values` method. If a change is detected, it sets its internal `dirty` flag and triggers a redraw. This ensures that any `Wlabel` subclass correctly updates its colors and content when the `state` is changed via `XtVaSetValues`.

```c
// Wlabel.widget
@proc set_values
{
    // ...
    if ($state != $old$state) { redraw_flag = 1; }
    // ...
    if (redraw_flag) {
        $dirty = 1;
        do_expose = 1;
    }
    // ...
}
```

---

## 6. Key Debugging Concepts


### The Selection System
1. **Source Mapping**: `re-tex` nodes now consistently store `source_offset`.
2. **Node Filtering**: `find_node_at` (in `retex.c`) ignores nodes with negative offsets (like vertical skips) to prevent selection "jumping."
3. **Stretched Glue**: Rendering (in `renderer.c`) now calculates the exact width of spaces by applying the `glue_set` factor, ensuring highlights perfectly cover the gap between words.
4. **Parfill**: The final line-filling glue (`parfill`) is assigned the end-of-paragraph index, allowing users to select the "empty" space at the end of a line.

### Common Pitfalls
- **Header Mismatches**: If a widget complains about missing types, check if the corresponding `_types.h` was copied to `build/include/xtcw`.
- **Struct Alignment**: `var5.c` was refactored to use standard named members (`base`, `core`) instead of anonymous structs to ensure compatibility across different GCC versions without `-fms-extensions`.

---

## 5. Building Widgets with wbuild

`wbuild` is a transpiler that converts `.widget` files into standard X Toolkit (Xt) C code (`.c` and `.h`). It simplifies widget development by reducing boilerplate and providing a cleaner syntax for defining resources, instance variables, and methods.

### .widget File Structure

A `.widget` file is divided into several sections, each starting with an `@` keyword:

- **`@class <ClassName> (<SuperClass>)`**: Defines the widget class name and its parent class.
- **`@public`**: Defines public resources (e.g., `Pixel`, `Dimension`) that can be set via `XtSetValues` or resource files.
- **`@private`**: Defines private instance variables for the widget.
- **`@private-constraints`**: (For Constraint widgets) Defines private variables associated with each child widget.
- **`@methods`**: Defines standard Xt methods (e.g., `initialize`, `realize`, `set_values`, `expose`).
- **`@utilities`**: Defines internal helper functions.
- **`@exports`**: Content placed in the public header file (`<ClassName>.h`).
- **`@imports`**: Content (like includes) used only in the implementation file (`<ClassName>.c`).
- **`@proc <MethodName>(...) { ... }`**: Defines a method or utility function.

### Syntax Highlights

- **`$` or `self`**: Represents the current widget instance. While `$` is a shorthand for the widget's part, `self` is often used as the canonical name for the widget pointer in method signatures and callback data.
  - `$var`: Accesses a variable or resource `var` defined in `@public` or `@private` of the current widget.
  - `$old$var`: (In `set_values`) Accesses the value of `var` from the `old` widget instance.
- **`$widget$member`**: Accesses members or resources of another widget. Common examples:
  - `$child$x`, `$child$width`: Accessing geometry of a child widget in a container.
  - `$parent$width`: Accessing the parent's width.
- **`#method_name(args)`**: Calls the superclass's implementation of a method.
  - Example: `#expose($, event, region);` to call the superclass's expose method.
- **`@var <Type> <Name> = <DefaultValue>`**: Declares a variable with an optional default value.
- **Type Conversions**: Supports `<String>` for automatic resource conversion from strings.

### Advanced Usage & Workarounds

- **Constraint Resources**: While `wbuild` supports `@constraints`, it has been known to segfault in some cases (e.g., during the development of `WPaned`). A reliable workaround is to define constraint variables in `@private-constraints` and manually register the resources in `@proc class_initialize`.
- **Internal Borders**: Use `@utilities` to define helper functions for common tasks like drawing internal borders or managing layout.

Example from `WPaned.widget`:
```c
@class WPaned (Constraint)

@public
@var Pixel internalBorderColor = <String> XtDefaultForeground
@var Dimension internalBorderWidth = 1

@private
@var GC normgc

@methods
@proc initialize
{
    $normgc = NULL;
    GetGCs($);
}
```
