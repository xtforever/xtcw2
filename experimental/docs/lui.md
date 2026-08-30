# LUI: Lua User Interface

LUI (Lisp-like User Interface) is a declarative UI framework that uses S-expressions to define Xt widget hierarchies. A Lua runtime provides logic, callbacks, and data binding.

## Architecture

```
S-expression string
       │
       ▼
  parser.lua ──► AST
       │
       ▼
  macros.lua ──► Expanded AST
       │
       ▼
  backend_xt.lua ──► xtcreate() calls
       │
       ▼
  Xt Toolkit ──► Widget tree
```

## LUI Modules

| Module | File | Purpose |
|--------|------|---------|
| parser | `lui/parser.lua` | LPeg-based S-expression parser. Outputs AST tables. |
| macros | `lui/macros.lua` | Compile-time macro expansion and evaluation. |
| backend_xt | `lui/backend_xt.lua` | Translates AST to Xt widget creation calls. |
| gui_xt | `lui/gui_xt.lua` | High-level API: `set()`, `get()`, `get_widget()`, `run()`. |
| registry | `lui/registry.lua` | Tag → WidgetClass mapping (22 entries). |
| test_harness | `lui/test_harness.lua` | Test utilities: `click()`, `toggle()`, `action()`, `expect()`. |
| lui | `lui/lui.lua` | Main entry point. Loads and initializes all modules. |

## S-Expression Syntax

```lisp
(window :id "top" :title "My App" :width 400 :height 300
  (grid :id "gr"
    (label :id "msg" :label "Hello World" :gridx 0 :gridy 0)
    (button :id "btn" :label "Click Me" :gridx 0 :gridy 1 :callback "LUA(on_click)")
  ))
```

### AST Format

- **Symbols**: Plain strings
- **Keywords**: Tables like `{keyword=":id"}`
- **Nil sentinel**: `parser.NIL` distinguishes explicit nil from missing keys

## Property Mapping (backend_xt.lua)

| LUI Property | Xt Resource | Notes |
|-------------|-------------|-------|
| `:on-click` | `callback` | Auto-wrapped in `LUA()` |
| `:on-row-activated` | `notify` | For list widgets |
| `:on-toggle` | `callback` | For toggle widgets |
| `:on-change` | `callback` | For edit widgets |
| `:align` | `alignment` | 0=left, 1=center, 2=right, 3=justify |
| `:model` | `tableStrs` | Converts table to mls handle |
| `:columns` | `columnWidths` | Joins table with commas |
| `:color` | `foreground` | X11 color name |
| `:bg` | `background` | X11 color name |
| `:weight-x` | `weightx` | Gridbox weight |
| `:weight-y` | `weighty` | Gridbox weight |
| `:src` | `filename` | For IconSVG |
| `:fill` | `fill` | "none", "width", "height", "both" |
| `:managed` | `wcManaged` | Boolean |
| `:orientation` | `vertical` | 1=vertical, 0=horizontal |
| `:fraction` | `frac` | For WPaned position |

### Callbacks

- String callbacks: `:callback "LUA(func_name)"` — dispatched by Lua callback queue
- Function callbacks: `:on-click (function() ... end)` — auto-registered with unique ID

### Shell Class Handling

When the first widget under the root shell has class `topLevelShellWidgetClass` or `applicationShellWidgetClass`, it's automatically converted to a `Gridbox` container. This allows window definitions to work naturally.

### Unknown Tags

If a tag isn't in the registry, backend_xt falls back to `{ class = tag }` and prints a warning. To suppress, add the tag to `lui/registry.lua`.

## lua_bridge C Bindings

Registered in `experimental/lua_bridge.c`:

| Function | Description |
|----------|-------------|
| `xtaction(widget, action, args...)` | Call Xt action procedure on widget |
| `xtapptimeout(ms, callback)` | Schedule Lua callback in Xt event loop (returns timer ref) |
| `xtcreate(name, class, parent, ...)` | Create Xt widget with key/value args |
| `xtmanage(widget)` | Manage (show) widget |
| `xtunmanage(widget)` | Unmanage (hide) widget |
| `xtappexitflag()` | Exit the application event loop |
| `gui.get(id, property)` | Get Xt resource value |
| `gui.set(id, property, value)` | Set Xt resource value |
| `gui.get_widget(id)` | Get widget pointer (for xtaction) |
| `callF(...)` | Alias for xtaction |

### xtapptimeout Usage

```lua
-- Schedule callback after 500ms
xtapptimeout(500, function()
    -- This runs in the Xt event loop after 500ms
    print("Timer fired!")
    xtappexitflag()  -- Exit the app
end)
```

The callback is dispatched by `process_lua_cbs` polling timer every 100ms. Actual latency is up to 100ms longer than requested.

### xtaction and Command Widget

```lua
-- WRONG: notify does nothing without set()
xtaction(widget, "notify")

-- CORRECT: set → notify → unset
xtaction(widget, "set")       -- sets command.set = True
xtaction(widget, "notify")    -- now callbacks fire
xtaction(widget, "unset")     -- clean up
```

## Test Infrastructure

- **Runner**: `lui_test/run_widget_tests.sh` — runs under xvfb with `-Tracelevel 50`
- **Harness**: `lui/test_harness.lua` — `click()`, `toggle()`, `action()`, `expect()`
- **CWD requirement**: Must run from `experimental/` directory (Lua module path `../lui/?.lua`)
- **Result files**: Tests write results to absolute paths (not relative CWD)

### Running Tests

```bash
cd /home/jens/git/xtcw2
bash lui_test/run_widget_tests.sh
```

### Current Test Coverage

| Test | What it verifies | Assertions |
|------|-----------------|------------|
| widget_creation | Widget creation + property get/set | 3 (label get, label set, round-trip) |
| button_callback | Command callback dispatch | 2 (count=1, label updated) |
| wpixbtn_action | WpixBtn actions + TRACE(50) | 2 (TRACE present, WpixBtn name) |
| trace_level | TRACE(50) activation | 1 (trace entries found) |
| **Total** | | **8** |

### What Tests Do NOT Cover

- Widget rendering (dimensions, text visibility)
- Layout correctness (Gridbox weights, spanning)
- X11 protocol errors
- Memory leaks beyond AddressSanitizer baseline
- Visual regression (screenshot comparison)