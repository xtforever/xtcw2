# How to Run Examples (XTCW Project)

This guide explains how to build and run the various example applications and demos within the professional `clean-build` structure.

## Prerequisites

Ensure you have the following installed on your system:
- X11 development libraries (Xt, Xaw, Xft, Xpm, Xext, Xmu)
- Cairo and Fontconfig development libraries
- Standard C build tools (`gcc`, `make`, `bison`, `flex`)
- (Optional) `docker-compose` for cleanroom builds

## Building the Project

Before running any examples, you must compile the entire toolkit:

```bash
cd clean-build
make clean && make all
```

*Note: This will build the `wbuild` tool, generate widget code, compile the `xtcw` library, and build all applications.*

---

## 1. Traditional C Applications

These examples are standalone C programs linked against the `xtcw` library.

### WPaned Demo (`test_paned`)
Demonstrates nested horizontal and vertical panes with interactive splitters.

```bash
cd clean-build/build/bin
XENVIRONMENT=test_paned.ad ./test_paned
```

### Retex Integration Test (`retex_test`)
Demonstrates the TeX-inspired layout engine rendering complex formulas and paragraphs within a widget.

```bash
cd clean-build/build/bin
XENVIRONMENT=main.ad ./retex_test
```

---

## 2. Lua (LUI) Applications

These examples use the `luarunner` application to load declarative UI definitions and logic from Lua scripts.

### WPaned LUI Demo (`paned_demo.lua`)
A modern layout with a sidebar, main content area (including math formulas), and a footer, all built using Lua.

```bash
cd clean-build/build/bin
XENVIRONMENT=paned_demo.ad ./luarunner
```

### Wmenu Pull-down LUI Demo (`menu_demo.lua`)
Demonstrates pull-down menus created using the `Wmenu` widget and Lua callbacks.

```bash
cd clean-build/build/bin
XENVIRONMENT=menu_demo.ad ./luarunner
```

### MessageBox LUI Demo (`msgbox_demo.lua`)
Showcases different types of message boxes (Info, Warning, Error, Success) with SVG icons and dynamic resource updates from Lua.

```bash
cd clean-build/build/bin
XENVIRONMENT=msgbox_demo.ad ./luarunner
```

### Multi-column List Demo (`test_list.lua`)
Demonstrates a scrollable, multi-column list widget with TeX rendering support in cells.

```bash
cd clean-build/build/bin
XENVIRONMENT=msgbox_demo.ad ./luarunner test_list.lua
```

### Dual-Pane File Manager Demo (`filemanager.lua`)
A full-featured file manager showcasing resizable panes, directory navigation, and background (threaded) file copying with progress reporting.

```bash
cd clean-build/build/bin
XENVIRONMENT=filemanager.ad ./luarunner
```

### Generic LUI Runner
You can run any `.lua` interface using `luarunner`. Most LUI apps require a corresponding `.ad` (Application Defaults) file to set up environment paths.

```bash
cd clean-build/build/bin
XENVIRONMENT=your_config.ad ./luarunner
```

---

## 3. Retex Engine Demos

These are low-level demos for the `re-tex` layout engine, showcasing rendering capabilities to different backends.

### Cairo PNG Backend
Renders a TeX-like layout to `output.png`.

```bash
cd clean-build/build/bin
./demo_png
```

### X11 Pixmap Backend
Renders a live TeX-like layout directly into an X11 window.

```bash
cd clean-build/build/bin
./demo_x11
```

---

## Troubleshooting

- **Resources not loading**: Always ensure you are in the directory containing the `.ad` and `.lua` files, or that your `XENVIRONMENT` variable points to the correct file.
- **Font issues**: If text does not appear, ensure you have common fonts like "Serif" and "Sans" installed via Fontconfig.
- **Display**: All GUI applications require a running X server (or XWayland).
