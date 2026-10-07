# Commander Runner Tutorial

A Lua-powered Xt GUI application framework combining the X Toolkit widget system with Lua scripting.

`commander_runner` is a small C harness that creates a top-level Xt shell, embeds a Lua
5.3 interpreter, and loads a LUI script which builds and manages the widget tree. The C
program owns the Xt event loop (`XtAppMainLoop`); Lua callbacks execute immediately
when their widget callback/action fires.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Command Line Options](#command-line-options)
3. [How Scripts Are Loaded](#how-scripts-are-loaded)
4. [Lua API Reference](#lua-api-reference)
5. [Widget Creation](#widget-creation)
6. [Layout Patterns](#layout-patterns)
7. [Callbacks](#callbacks)
8. [Data Binding](#data-binding)
9. [Advanced: Task/Threading](#advanced-taskthreading)
10. [Examples](#examples)
11. [Architecture Notes](#architecture-notes)

---

## Quick Start

### Building

```bash
cd commander
make commander_runner
```

The `commander/makefile` builds `commander_runner` and links against the LuaXtw SWIG
wrapper (`../LuaRunner/luaxt.o`, `../LuaRunner/luaxt_wrap.o`) plus the project static
libraries in `../build/lib`. Requirements:

- Lua 5.3 (via `pkg-config lua5.3`)
- Xaw / Xt / X11 / Xpm / Xext / Xmu / Xft / fontconfig / Xrender / cairo
- pthread

The build is instrumented with `-ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address`.

### Running

Run **from the `commander/` directory**. The bootstrap hard-codes
`package.path = '../lui/?.lua;' .. package.path`, so the process must run somewhere
whose parent directory contains `lui/`.

```bash
# Default: loads ex.lua (see fallback resources)
./commander_runner

# Load a specific LUI script
./commander_runner -Luafile ../demos/demo_login.lua

# Raise the trace level
./commander_runner -Luafile ../demos/demo_form.lua -Tracelevel 5
```

---

## Command Line Options

| Option | Resource | Description | Default |
|--------|----------|-------------|---------|
| `-Luafile <file>` | `*luafile` | LUI script to load | `ex.lua` |
| `-Tracelevel <n>` | `*traceLevel` | Debug trace level (higher = more verbose) | `2` |

These two are defined in `commander_runner.c`. The `WCL_XRM_OPTIONS` macro appends the
standard Wcl options:

| Option | Resource | Description |
|--------|----------|-------------|
| `-ResFile <file>` / `-rf <file>` | `*wclInitResFile` | Wcl resource file |
| `-trrf` | `*wclTraceResFiles` | Trace resource-file loading |
| `-Trace` | `*wcTrace` | Enable Wcl tracing |
| `-Warnings` | `*wclVerboseWarnings` | Enable verbose warnings |

### Application Name and Resources

The Xt application class/name is **`luarunner`** (`#define APP_NAME "luarunner"`).
Fallback resources set `luarunner.*` and the defaults:

```
luarunner.allowShellResize: False
luarunner.name: luarunner
*width: 600
*height: 400
*traceLevel: 2
*luafile: ex.lua
```

`commander.ad` is auto-loaded (via `*WclResFiles: commander.ad`) and sets
`commander_runner.wcName: luarunner`. Without a `-Luafile` script, the pure-Wcl path
(`WcWidgetCreation`) is used instead.

---

## How Scripts Are Loaded

The `-Luafile` file is loaded through `lui.load()` (not `lui.run()` directly). The C
program first injects a bootstrap chunk:

```lua
package.path = '../lui/?.lua;' .. package.path
package.cpath = '../lui/?.so;' .. package.cpath
local gui = require('gui_xt')
local lui = require('lui')
lui.loop = function() end
gui.loop = function() end
_G.__shell_name__ = '<shell name>'      -- the widget name, "luarunner"
local __ok, __err = lui.load('<luafile>')
```

`lui.load(filename)` reads the file and:

1. If the file's first non-whitespace character is `(`, it is treated as **raw LUI
   S-expression markup** and run directly (`lui.run(source)`).
2. Otherwise it is treated as a Lua chunk, executed with `_G` as its environment. If
   the chunk set `_G.lui_source`, that string is then run through `lui.run()`.

So a `-Luafile` file may be one of:

- A raw S-expression file, e.g. a file whose first line is `(window :title "Hi" ...)`.
- A Lua script that ends with `lui.run([[ ... ]])` (the demo style).
- A Lua script that sets `_G.lui_source = [[ ... ]]`.

Because the Lua chunk runs with `_G` as environment, top-level `function on_x()` /
`function on_x` definitions (without `local`) become **globals**, which is how the
callback dispatch finds them (see [Callbacks](#callbacks)).

`_G.__shell_name__` is set to the shell widget name before your script runs.

---

## Lua API Reference

### Globals registered by C (`lua_bridge_register_bindings`)

These are plain Lua globals available before any script runs:

| Function | Description |
|----------|-------------|
| `xtcreate(id, class, parent, prop1, val1, ...)` | Create a widget |
| `xtsetvalue(w, prop, val)` | Set a widget property (string or integer) |
| `xtgetvalue(w, prop)` | Get a widget property (type-aware) |
| `xtaction(w, action, ...)` | Invoke an Xt action (alias `callF`) |
| `xtmanage(w)` / `xtunmanage(w)` / `xtdestroy(w)` | Manage / unmanage / destroy a widget |
| `xtappexitflag()` | Set the Xt exit flag (clean shutdown) |
| `xtapptimeout(...)` | Schedule a timeout |
| `xtgeometry(w)` | Query a widget's geometry |
| `xterror_count(reset)` | Read/reset the X error counter |

MLS (Memory List System) helpers:

| Function | Description |
|----------|-------------|
| `mls_create(size, width)` | Create a list, returns integer handle |
| `mls_put_string(handle, s)` | Append a string |
| `mls_clear(handle)` | Clear the list |
| `mls_len(handle)` | Number of entries |
| `mls_get_string(handle, index)` | Get entry at index |

The `luaxt` SWIG module is also installed as the global `luaxt`.

### `gui` module (`gui_xt.lua`, exposed as `_G.gui`)

```lua
local gui = require('gui_xt')   -- also sets _G.gui

gui.build(ast)                   -- Build widgets from parsed AST
gui.run(source)                  -- Parse + build LUI source
gui.load(path)                   -- Read file, then run its source
gui.set(id, prop, val)           -- Set a widget property by id
gui.get(id, prop)                -- Get a widget property by id
gui.get_widget(id)               -- Return the raw widget for an id
gui.geometry(id)                 -- Widget geometry table
gui.xerrors(reset)               -- Read/reset X error count
gui.manage(id) / gui.unmanage(id) / gui.destroy(id)
gui.update(id)                   -- Re-push tableStrs from a store
gui.loop()                       -- Event loop (no-op under commander_runner)
gui.modal_loop(check_fn)         -- Run a modal loop while check_fn() is true

gui.register_handler(name, fn)   -- Register a callback handler by name

-- Data stores
gui.create_store(cols)           -- -> { handle = <int>, cols = <n> }
gui.create_tree_store(cols)      -- alias of create_store
gui.store_append(store, values)  -- Append a row (table of strings)
gui.store_get(store, index)      -- -> table of cell strings, or nil
gui.tree_append(store, parent, values)  -- alias of store_append

-- Dialogs
gui.alert(msg)                   -- Modal alert
gui.confirm(msg)                 -- Modal confirm -> boolean
gui.prompt(title, msg, default)  -- Modal prompt -> string or nil
```

### `lui` module

```lua
local lui = require('lui')

lui.run(lui_source)   -- Parse + expand + build a LUI source string
lui.load(filename)    -- Load a file (raw S-expr or Lua chunk), see above
lui.loop()            -- No-op under commander_runner (C owns the event loop)
```

### Parser / macros / registry modules

```lua
local parser = require('parser')
local macros = require('macros')
local registry = require('registry')

ast = parser.parse(source)             -- Parse LUI source to AST
macros.expand(ast)                     -- Expand macros in AST
macros.register(name, args, body)      -- Register a custom macro
registry.register(name, def)           -- Register a widget type
registry.get(name)                     -- Get a widget definition
```

---

## Widget Creation

### Available Widgets

Widget tags come from `lui/registry.lua` (friendly aliases) plus the auto-generated
class-name tags in `registry_generated.lua` (one tag per compiled widget class, e.g.
`(WlistMulti ...)`, `(Frame ...)`, `(VBox ...)`).

Friendly aliases:

| Tag | Class | Description |
|-----|-------|-------------|
| `window` | `topLevelShellWidgetClass` | Application window (converted to a Gridbox root) |
| `label` | `Wlabel` | Text label |
| `button` | `Wbutton` | Push button |
| `edit` | `Wedit` | Text entry |
| `grid` | `Gridbox` | Grid layout container |
| `vertical` / `vbox` | `VBox` | Vertical box |
| `horizontal` / `hbox` | `HBox` | Horizontal box |
| `image` / `icon` | `IconSVG` | SVG image |
| `separator` | `WSeparator` | Separator |
| `check` / `radio` | `Wradio` | Radio button |
| `checkbox` | `Wcheckbox` | Check box |
| `toggle` | `Toggle` | Toggle |
| `scrolled` | `ScrolledCanvas` | Scrolled container |
| `canvas` | `Canvas` | Canvas |
| `frame` | `Frame` | Frame |
| `gauge` | `Gauge` | Gauge |
| `hslider` | `HSlider` | Horizontal slider |
| `vslider` | `VSlider` | Vertical slider |
| `list-view` | `WlsMulti` | Multi-column list |
| `list` / `wlist` | `Wlist` | List |
| `wlistmulti` / `wlsmulti` | `WlistMulti` | Multi list |
| `list4` / `wlist4` | `Wlist4` | List widget |
| `splitter` | `Wsplitter` | Resizable split pane |
| `combo` | `Wcombo` | Combo box |
| `spinbox` | `WspinBox` | Spin box |
| `password` | `Wpassword` | Password entry |
| `retex` | `Wretex` | Rich text |
| `text` | `Wtext` | Text widget |
| `paned` | `WPaned` | Paned container |
| `menu` / `menupopup` | `Wmenu` / `WmenuPopup` | Menus |
| `file-selector` | `WfileSelector` | File selector dialog |
| `messagebox` | `MessageBox` | Message box |
| `option` | `Woption` | Option menu |
| `viewvar` | `WviewVar` | View variable |
| `editmv` | `WeditMV` | Multi-value edit |
| `pixbtn` | `WpixBtn` | Pixmap button |
| `board` / `dartboard` | `Board` / `Dartboard` | Board widgets |
| `selectreq` | `SelectReq` | Select request |
| `command` | `command` | Command |

### Basic Widget Syntax

```
(widget-type :prop1 value1 :prop2 value2
  (child-widget :prop value)
  (child-widget :prop value))
```

### Property Aliases (from `backend_xt.lua`)

`process_property` maps friendly names to widget resources before setting them:

| LUI property | Widget resource / behavior |
|--------------|----------------------------|
| `on-click` / `on-toggle` / `on-change` | `callback` |
| `on-row-activated` | `notify` |
| `orientation` | `vertical` (`"vertical"` -> `1`, else `0`) |
| `fraction` / `frac` | `frac` (numeric) |
| `color` | `foreground` |
| `bg` | `background` |
| `weight-x` / `weight-y` | `weightx` / `weighty` |
| `src` | `filename` |
| `rasterize-width` / `rasterize-height` | `forced_width` / `forced_height` |
| `model` | `tableStrs` (accepts handle or Lua table) |
| `columns` | `columnWidths` (`(a b c)` -> `"a,b,c"`) |
| `managed` | `wcManaged` |
| `align` | `alignment` |
| `fill` | `0/1/2/3` -> `none/horizontal/vertical/both` |
| `spinValue` / `spinMin` / `spinMax` / `spinStep` | `value` / `min` / `max` / `step` |

### Example: Window with Button

```lua
-- demo.lua
local lui = require('lui')

lui.run([[
(window :title "Demo" :width 300 :height 200
  (grid
    (button :label "Click Me" :gridx 0 :gridy 0 :callback "quit_cb")))
]])
```

`quit_cb` is a C callback registered via `RCB`; it sets the Xt exit flag.

---

## Layout Patterns

### Grid Layout

```lua
-- gridx, gridy: position in grid
-- weightx, weighty: resize priority (0 = fixed)
-- fill: 0=none, 1=horizontal, 2=vertical, 3=both

lui.run([[
(window :title "Grid Demo" :width 400 :height 300
  (grid
    (label :label "Top Left" :gridx 0 :gridy 0)
    (label :label "Top Right" :gridx 1 :gridy 0)
    (button :label "Bottom" :gridx 0 :gridy 1 :gridWidth 2)))
]])
```

### Splitter Layout

```lua
-- splitter: resizable split panes
-- orientation: "vertical" or "horizontal"
-- fraction: initial split position (0.0-1.0)

lui.run([[
(window :title "Splitter Demo" :width 500 :height 400
  (splitter :id "main_split" :orientation "vertical" :fraction 0.3
    (label :label "Top Pane" :gridx 0 :gridy 0)
    (label :label "Bottom Pane" :gridx 0 :gridy 0)))
]])
```

---

## Callbacks

### How callbacks are dispatched

The C program registers two entry points:

- `LUA_action` — an Xt action, registered via `XtAppAddActions` and `wcreg_action`.
- `LUA_callback` — a Wcl callback, registered via `wcreg_callback(TopLevel, LUA_callback, "LUA")`.

Both parse the string argument (`"funcname"`, `"funcname data"`, or `"funcname, data"`)
into a function name and an optional data argument, then build and immediately execute
a Lua chunk:

- with data: `funcname("data")`
- without data: `funcname()`

The chunk is run right away via `luaL_loadstring` + `lua_pcall`. Consequently the
callback **must resolve to a Lua global function**, and it receives at most one string
argument.

`backend_xt.lua` passes string callback values through to Wcl unchanged, so the value
resolves by name:

- `LUA(funcname)` — the `LUA` callback proc, which runs the Lua global `funcname`.
- `some_name` — a C callback registered via `wcreg_callback` / `RCB` (e.g. `quit_cb`).

Callbacks are **not** auto-wrapped: a bare name is a C callback, and Lua callbacks must
be spelled `LUA(...)`.

### Pattern 1: Lua global function (explicit LUA())

```lua
function my_callback(data)
    print("Button clicked!", data or "")
    gui.set("my_label", "label", "Updated!")
end

lui.run([[
(window :title "Callback Demo"
  (grid
    (label :id "my_label" :label "Original")
    (button :label "Click" :callback "LUA(my_callback)")))
]])
```

### Pattern 2: C callback (bare name)

```lua
-- A bare callback name resolves to a C callback registered via RCB.
-- commander_runner registers quit_cb, which sets the Xt exit flag.
lui.run([[
(button :label "Quit" :callback "quit_cb")
]])
```

### Callback with data

```lua
function on_row(data)
    print("Row activated:", data)
end

lui.run([[
(list-view :id "list" :on-row-activated "LUA(on_row, mydata)")
]])
```

`on-row-activated` maps to `notify`; the `LUA(func, data)` form calls `func("data")`.

### Input callbacks

```lua
function input_cb()
    local text = gui.get("my_edit", "label")
    gui.set("result_label", "label", "You typed: " .. text)
end

lui.run([[
(grid
  (edit :id "my_edit" :label "Type here...")
  (button :label "Submit" :callback "LUA(input_cb)")
  (label :id "result_label" :label ""))
]])
```

### Quitting

- A C `quit_cb` is registered via `RCB(TopLevel, quit_cb)`. Use a bare
  `:callback "quit_cb"` to invoke it — it sets the Xt exit flag and is the cleanest way
  to quit.
- From Lua, `xtappexitflag()` sets the Xt exit flag directly (e.g. inside a `LUA(...)`
  callback).
- `os.exit(0)` works but terminates the process abruptly (skipping cleanup).

---

## Data Binding

### Store Pattern

LUI data binding uses placeholder replacement into the `:model` property. The store is
an MLS list created with `gui.create_store(cols)`, and rows are appended with
`gui.store_append`.

```lua
local gui = require('gui_xt')
local lui = require('lui')

local store = gui.create_store(3)
gui.store_append(store, {"Item 1", "Active", "100"})
gui.store_append(store, {"Item 2", "Pending", "250"})
gui.store_append(store, {"Item 3", "Inactive", "50"})

local source = [[
(window :title "Data Binding Demo" :width 500 :height 400
  (grid
    (label :label "Multi-Column List")
    (list-view :id "mylist" :model $PLACEHOLDER
               :columns (150 150 100)
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3)
    (button :label "Quit" :gridx 0 :gridy 2 :callback "quit_cb")))
]]

source = source:gsub("$PLACEHOLDER", tostring(store.handle))
lui.run(source)
```

`backend_xt.lua` maps `:model` to the widget's `tableStrs` resource and accepts either
a raw integer handle or a Lua table (in which case it builds a store on the fly).

### Multiple Stores

```lua
local sidebar_store = gui.create_store(1)
local files_store = gui.create_store(3)

gui.store_append(sidebar_store, {"Files"})
gui.store_append(files_store, {"readme.txt", "1KB", "2024-01-15"})
gui.store_append(files_store, {"main.lua", "5KB", "2024-01-15"})

local source = [[
(window :title "File Manager" :width 600 :height 400
  (splitter :id "main_split" :orientation "vertical" :fraction 0.25
    (list-view :id "sidebar" :model $SIDEBAR_STORE)
    (list-view :id "files" :model $FILES_STORE
               :columns (300 150 100)
               :on-row-activated "LUA(on_file_select)")))
]]

source = source:gsub("$SIDEBAR_STORE", tostring(sidebar_store.handle))
source = source:gsub("$FILES_STORE", tostring(files_store.handle))
lui.run(source)
```

---

## Advanced: Task/Threading

`commander_runner` initializes the task manager and registers the built-in `copy_task`
worker at startup:

```c
task_manager_init(app);
task_register_func("copy_task", (task_func_t)copy_task);
```

`copy_task` (in `file_ops.c`) copies a file in a background thread, reporting progress
and completion events, and honors STOP/CONT/ABORT commands.

The following Lua task bindings are registered at startup (via `register_task_lua()`):

| Function | Description |
|----------|-------------|
| `task_copy(src, dst)` | Copy a file in a background task, returns job id |
| `task_spawn(func_name, arg)` | Spawn a registered task function, returns job id |
| `task_control(id, cmd)` | Send a command (`TASK_CMD_STOP/CONT/ABORT`) to a task |
| `task_set_handler(name)` | Register a Lua global to receive task events |
| `ls(path)` | List a directory (returns an array of `{name, is_dir, size}`) |
| `wheel_exec_command(w, cmd, val)` | Send a wheel command |

Exported constants: `TASK_CMD_STOP/CONT/ABORT`, `TASK_EVENT_PROGRESS/COMPLETE/ERROR`,
`WHEEL_UP/DOWN/FIRE`.

Task events are delivered to the handler set via `task_set_handler`:

```lua
function on_task(msg)
    -- msg = { job_id, type, progress, message }
    if msg.type == TASK_EVENT_COMPLETE then
        print("job " .. msg.job_id .. " done")
    end
end

task_set_handler("on_task")
local job = task_copy("src.txt", "dst.txt")
```

Relevant C API (`task_manager.h`):

```c
typedef enum { TASK_CMD_NONE = 0, TASK_CMD_STOP, TASK_CMD_CONT, TASK_CMD_ABORT } task_cmd_t;
typedef enum { TASK_EVENT_PROGRESS, TASK_EVENT_COMPLETE, TASK_EVENT_ERROR } task_event_t;

int  task_spawn(void *(*func)(task_thread_args_t*), void *arg);
void task_control(int id, task_cmd_t cmd);
void task_report(int id, task_event_t type, float progress, const char *fmt, ...);
task_cmd_t task_check_command(task_thread_args_t *args);
void task_register_func(const char *name, task_func_t func);
```

See `commander/task_manager.c`, `commander/file_ops.c`, and
`commander/lua_task_bindings.c` for details.

---

## Examples

### Example 1: Simple Window

```lua
-- simple.lua
local lui = require('lui')
lui.run([[
(window :title "Simple Demo" :width 300 :height 200
  (grid
    (label :label "Hello from LUI!" :gridx 0 :gridy 0)
    (button :label "Quit" :gridx 0 :gridy 1 :callback "quit_cb")))
]])
```

Run: `./commander_runner -Luafile simple.lua`

### Example 2: Multi-Column List

```lua
-- list_demo.lua
local gui = require('gui_xt')
local lui = require('lui')

local list_store = gui.create_store(3)
gui.store_append(list_store, {"Item 1", "Active", "100"})
gui.store_append(list_store, {"Item 2", "Pending", "250"})
gui.store_append(list_store, {"Item 3", "Inactive", "50"})
for i = 4, 20 do
    gui.store_append(list_store, {"Item " .. i, "Status " .. i, tostring(i * 10)})
end

local source = [[
(window :title "List Demo" :width 500 :height 400
  (grid
    (label :label "Data List" :gridx 0 :gridy 0 :fontSize 24)
    (list-view :id "mylist" :model $STORE
               :columns (150 150 100)
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3)
    (button :label "Quit" :gridx 0 :gridy 2 :callback "quit_cb")))
]]

source = source:gsub("$STORE", tostring(list_store.handle))
lui.run(source)
```

### Example 3: File Manager UI

```lua
-- file_manager.lua
function on_file_select(data) print("Selected:", data) end

local gui = require('gui_xt')
local lui = require('lui')

local sidebar = gui.create_store(1)
local files = gui.create_store(3)

gui.store_append(sidebar, {"Home"})
gui.store_append(sidebar, {"Documents"})
gui.store_append(sidebar, {"Downloads"})
gui.store_append(files, {"readme.txt", "1KB", "2024-01-15"})
gui.store_append(files, {"main.lua", "5KB", "2024-01-15"})
gui.store_append(files, {"config.json", "512B", "2024-01-14"})

local source = [[
(window :title "File Manager" :width 700 :height 500
  (splitter :id "split" :orientation "vertical" :fraction 0.2
    (list-view :id "sidebar" :model $SIDEBAR :gridx 0 :gridy 0)
    (list-view :id "files" :model $FILES
               :columns (300 150 100)
               :on-row-activated "LUA(on_file_select)")))
]]

source = source:gsub("$SIDEBAR", tostring(sidebar.handle))
source = source:gsub("$FILES", tostring(files.handle))
lui.run(source)
```

### Example 4: Input Form

```lua
-- form.lua
function submit_cb()
    local name = gui.get("name_input", "label")
    local email = gui.get("email_input", "label")
    gui.set("result", "label", "Name: " .. (name or "") .. "\nEmail: " .. (email or ""))
end

local lui = require('lui')
lui.run([[
(window :title "Input Form" :width 400 :height 300
  (grid
    (label :label "Name:" :gridx 0 :gridy 0)
    (edit :id "name_input" :gridx 1 :gridy 0 :width 250)
    (label :label "Email:" :gridx 0 :gridy 1)
    (edit :id "email_input" :gridx 1 :gridy 1 :width 250)
    (button :label "Submit" :gridx 0 :gridy 2 :callback "LUA(submit_cb)")
    (label :id "result" :label "" :gridx 0 :gridy 3 :gridWidth 2)
    (button :label "Quit" :gridx 1 :gridy 2 :callback "quit_cb")))
]])
```

---

## Architecture Notes

### Event Loop

Commander Runner owns the Xt event loop in C via `XtAppMainLoop(app)`. LUI scripts
should **not** run their own loop: the bootstrap sets `lui.loop` and `gui.loop` to
no-ops. Lua callbacks are executed immediately when their widget action/callback fires;
no callback queue is used.

### Startup Sequence

1. `XInitThreads()`, `m_init()`, `trace_level = 1`.
2. `XtOpenApplication(...)` creates the shell (class `sessionShellWidgetClass`).
3. `XtcwRegister(app)`, `XpRegisterAll(app)`.
4. `task_manager_init(app)` and register `copy_task`.
5. Create the Lua state (`luaL_newstate`, `luaL_openlibs`).
6. `lua_bridge_register_bindings()` — register the xt/mls globals.
7. `luaopen_luaxt(L_GLOBAL)`; install module as global `luaxt`.
8. Register the `LUA` action (`XtAppAddActions` + `wcreg_action`) and `LUA` callback
   (`wcreg_callback`), plus the C `quit_cb` via `RCB`.
9. `WcInitialize` / `WcRootWidget`.
10. Read app resources (`traceLevel`, `luafile`).
11. If a `luafile` is set: hide the shell, run `lui_bootstrap` (which calls
    `lui.load`), then apply the root container's requested size to the shell **before**
    realize. Otherwise run `WcWidgetCreation`.
12. `XtRealizeWidget`, then map the shell if LUI built widgets.
13. Enter the event loop (`XtAppMainLoop`).

### Widget Hierarchy

```
appShell (luarunner)
    └── first_widget (Gridbox - managed container)
            ├── grid (or other layout)
            │       ├── label
            │       ├── button
            │       └── ...
            └── ...
```

The shell is created by `XtOpenApplication()`. The first LUI widget becomes a managed
container child of the shell.

### Callback Flow

1. A widget triggers a callback (click, activation, etc.).
2. The callback string is passed through to Wcl, which resolves it by name:
   - `LUA(funcname)` -> the `LUA` callback proc registered in `commander_runner`.
   - a bare name -> a C callback registered via `wcreg_callback` / `RCB`.
3. For `LUA(funcname)`, the proc parses the funcname and optional data, then
   immediately executes `funcname("data")` / `funcname()` with
   `luaL_loadstring` + `lua_pcall`.

---

## Troubleshooting

### "widget not found"

Ensure the widget `:id` is correct and the widget exists before accessing it.

### "Unknown widget tag"

The widget type isn't registered. Check `lui/registry.lua` (friendly aliases) and
`lui/registry_generated.lua` (class-name tags) for available widgets.

### "LUI Bootstrap Error" / module not found

Run `commander_runner` from the `commander/` directory — the bootstrap prepends
`../lui/?.lua` to `package.path`.

### Callback does nothing

Lua callbacks must be spelled `LUA(funcname)` and must resolve to a Lua **global**
function. Define callbacks at the top level of the loaded file (without `local`), or
assign to `_G`. A bare callback name is a C callback (e.g. `quit_cb`), not a Lua
function.

### Script blocks/hangs

Don't call `lui.loop()` / `gui.loop()` in a busy loop — they are no-ops under
commander_runner; the C program already runs the event loop.

### Property not set

Some properties are processed/mapped by `backend_xt.process_property()`. Check the
property aliases in [Property Aliases](#property-aliases-from-backend_xtlua).
