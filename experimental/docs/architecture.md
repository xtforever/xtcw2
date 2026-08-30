# XTCW2 Architecture

## Project Overview

XTCW2 is a toolkit construction set for X11, built on Xt (X Toolkit Intrinsics) with a custom layout engine (re-tex), widget transpiler (wbuild), Lua integration (LUI), and handle-based memory system (MLS).

## Directory Structure

```
xtcw2/
├── wbuild/              # Widget transpiler (generates C from .widget files)
├── wbuild_widgets/      # Canonical widget source (.widget + hand-written .c/.h)
├── build/               # Generated build artifacts
│   ├── bin/             # wbuild, wb.sh
│   ├── include/xtcw/    # All headers (generated + copied)
│   ├── lib/             # libxtcw.a, libwcl.a, libutils.a, libretex.a, libplainc.a
│   └── source/          # Generated .c files from .widget
├── re-tex/              # TeX-inspired layout engine
│   └── src/             # node.c, retex.c, renderer.c, backends
├── utils/               # Foundation library
│   ├── mls.c            # Multiple List System (handle-based memory)
│   ├── m_tool.c         # conststr, focus-group, timers, vars
│   ├── var5.c           # Variable system
│   └── sig_xt.c         # Signal dispatch
├── wcl/                 # Widget Creation Library (WcCreate, WcRegister)
├── plainc_widgets/      # Hand-written C widgets (Gridbox, etc.)
├── lui/                 # LUI modules (Lua)
│   ├── parser.lua       # LPeg S-expression parser
│   ├── macros.lua       # Macro expansion
│   ├── backend_xt.lua   # AST → Xt widget translation
│   ├── gui_xt.lua       # High-level API (set, get, get_widget)
│   ├── registry.lua     # Tag → WidgetClass mapping (22 entries)
│   └── test_harness.lua # Test utilities (click, toggle, action, expect)
├── lui_test/            # Test scripts and runner
│   ├── run_widget_tests.sh  # Master test runner (xvfb-run + TRACE(50) capture)
│   ├── test_button.lua  # Button callback test
│   └── test_results/    # Test output directory
├── experimental/        # Commander runner (C host)
│   ├── commander_runner.c  # Main application (Xt init, Lua bootstrap)
│   ├── xt_bridge.c      # C bindings (xtaction, xtapptimeout, xtcreate)
│   ├── lua_bridge.c     # Lua function registration
│   └── file_ops.c       # File I/O bindings
├── LuaRunner/            # Lua/Xt bridge
│   ├── luaxt.c           # Lua callback queue (pushcallback/pullcallback)
│   └── luaxt_wrap.c     # Additional Lua wrappers
└── todomgr/              # Example application (todo manager)
```

## Build Pipeline

```
.widget files ──wbuild──→ .c + .h files ──cc──→ .o files ──ar──→ libxtcw.a
                                                          │
plainc_widgets/*.c ──cc──→ .o files ──ar──→ libplainc.a   │
                                                          │
utils/*.c ──cc──→ .o files ──ar──→ libutils.a             │
                                                          │
wcl/*.c ──cc──→ .o files ──ar──→ libwcl.a                 │
                                                          │
re-tex/src/*.c ──cc──→ .o files ──ar──→ libretex.a        │
                                                          │
└────────────────── all linked into commander_runner ──────┘
```

### Key Build Rules

- **Always use consistent `CFLAGS`**: Mixing `-DMLS_DEBUG` and non-debug objects causes crashes.
- **`.widget` files are source of truth**: Edits to generated `.c` in `build/source/` are overwritten by `wbuild`. Edit the `.widget` files in `wbuild_widgets/` for permanent changes.
- **WpixBtn.c is an exception**: It's a hand-written `.c` file copied (not regenerated) to `build/source/`.
- **`make libxtcw`**: Rebuilds the widget library including all generated and copied sources.
- **`make clean && make`**: Full rebuild from scratch.

### Important Makefile Targets

```bash
make                    # Build everything
make libxtcw            # Rebuild widget library only
make widgets            # Regenerate .c from .widget and rebuild
make plainc             # Build plain C widgets (Gridbox only)
make clean              # Remove object files
make distclean          # Remove all build artifacts
```

## wbuild Widget Transpiler

### .widget File Sections

| Section | Purpose |
|---------|---------|
| `@class Name(Super)` | Class name and inheritance |
| `@public` | Xt resources (settable via XtSetValues) |
| `@private` | Private instance variables |
| `@methods` | Xt methods (initialize, expose, set_values, etc.) |
| `@utilities` | Internal helper functions |
| `@exports` | Content placed in the public header |
| `@imports` | Includes for implementation file |
| `@actions` | Action procedures for translation tables |
| `@translations` | Default event-to-action mappings |

### Variable Access

- `$variable`: Access instance variable or resource
- `$old$variable`: In `set_values`, access previous value
- `self`: Widget pointer (Widget type)
- `$child$width`: Access child widget's resources
- `#method_name(args)`: Call superclass method

### Critical wbuild Rules

1. **Edits to `build/source/*.c` are overwritten** — always edit the `.widget` file
2. **TRACE(50) instrumentation must go in `.widget` files** to survive rebuilds
3. **`@exports` must include needed headers** — types like `uint32_t` need `@incl <stdint.h>` in exports, not imports
4. **Function naming**: Prefix exported functions with widget name to avoid linker collisions
5. **Constraint resources**: `@constraints` can segfault wbuild; use `@private-constraints` + manual registration instead

## C Host Application (commander_runner)

### Bootstrap Flow

1. `m_init()` → `trace_level = 1` → Xt initialization
2. `XtcwRegister(app)` + `XpRegisterAll(app)` → register all widget classes
3. `luaxt_init()` → `luaL_newstate()` → `lua_bridge_register_bindings()`
4. `WcInitialize()` → `WcRootWidget()` → parse Xrm resources
5. `XtGetApplicationResources()` → set `trace_level` from `-Tracelevel` option
6. `lui_bootstrap()` → load LUI file → `gui_xt.run(ui)` → create widgets
7. `XtAppMainLoop()` → event loop

### C Bindings Available in Lua

| Function | Description |
|----------|-------------|
| `xtaction(widget, action, args...)` | Call an Xt action procedure |
| `xtapptimeout(ms, callback)` | Schedule Lua callback in Xt event loop |
| `xtcreate(name, class, parent, ...)` | Create an Xt widget |
| `xtmanage(widget)` | Manage a widget |
| `xtunmanage(widget)` | Unmanage a widget |
| `gui.get(id, property)` | Get widget resource |
| `gui.set(id, property, value)` | Set widget resource |
| `gui.get_widget(id)` | Get widget pointer (for xtaction) |
| `xtappexitflag()` | Exit the application |

### Command Widget Callback Chain

The Xaw `Command` widget's `Notify` action guards callback invocation on `command.set`:

```lua
-- MUST call set() before notify()
xtaction(widget, "set")      -- command.set = True
xtaction(widget, "notify")   -- Now callbacks fire
xtaction(widget, "unset")    -- Clean up
```

### Callback Dispatch Latency

`commander_runner` uses a 100ms polling timer (`process_lua_cbs`) to dispatch Lua callbacks. Tests must wait ~300ms after `xtaction` calls before checking callback results.

### TRACE(50) Mechanism

```c
#define TRACE(l, n, a...) do { if((l) >= trace_level && trace_level != 0) deb_trace(l, __LINE__, __FILE__, __FUNCTION__, n, ##a); } while(0)
```

- `trace_level` is a global `int` in `mls.c`, default `0` (disabled)
- Pass `-Tracelevel 50` to see TRACE(50) output
- Output format: `[50]funcname: message`
- Goes to stderr; `xvfb-run` merges stdout+stderr

### Running LUI Scripts

```bash
# Must run from experimental/ directory (package.path = '../lui/?.lua')
cd experimental
./commander_runner -Tracelevel 50 -Luafile ../lui_test/test_button.lua

# Or under virtual framebuffer
xvfb-run --auto-servernum ./commander_runner -Tracelevel 2 -Luafile my_script.lua
```