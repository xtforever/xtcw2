# Commander Runner Tutorial

A Lua-powered Xt GUI application framework combining the X Toolkit widget system with Lua scripting.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Command Line Options](#command-line-options)
3. [Lua API Reference](#lua-api-reference)
4. [Widget Creation](#widget-creation)
5. [Layout Patterns](#layout-patterns)
6. [Callbacks](#callbacks)
7. [Data Binding](#data-binding)
8. [Advanced: Task/Threading](#advanced-taskthreading)
9. [Examples](#examples)

---

## Quick Start

### Building

```bash
cd experimental
make commander_runner
```

### Running

```bash
# Run with default settings
./commander_runner

# Run with a LUI script
./commander_runner -Luafile myapp.lua

# Run with trace level
./commander_runner -Luafile myapp.lua -TraceLevel 5
```

### Minimal LUI Script

```lua
-- myapp.lua
_G.quit_cb = function()
    os.exit(0)
end

local source = [[
(window :title "My App" :width 400 :height 300
  (grid
    (label :label "Hello, World!" :gridx 0 :gridy 0)
    (button :label "Quit" :gridx 0 :gridy 1 :callback "quit_cb")))
]]

local lui = require('lui')
lui.run(source)
```

---

## Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `-Luafile <file>` | LUI script to load | `ex.lua` |
| `-TraceLevel <n>` | Debug trace level (higher = more verbose) | `2` |
| `-ResFile <file>` | Wcl resource file | (none) |
| `-Trace` | Enable Wcl tracing | off |
| `-Warnings` | Enable Wcl warnings | off |

### Resource File

Commander Runner also reads from `luarunner.ad` (or custom resource file). See Wcl documentation for format.

---

## Lua API Reference

### Core LUI Functions

| Function | Description |
|----------|-------------|
| `lui.run(source)` | Parse and run LUI source string |
| `lui.load(filename)` | Load and run LUI script from file |
| `lui.loop()` | Start event loop (provided by C code, no-op in commander_runner) |

### GUI Module (gui_xt)

```lua
local gui = require('gui_xt')

-- Widget lifecycle
gui.build(ast)           -- Build widgets from AST
gui.manage(id)           -- Make widget visible
gui.unmanage(id)         -- Hide widget
gui.destroy(id)           -- Destroy widget
gui.set(id, prop, val)   -- Set widget property
gui.get(id, prop)        -- Get widget property

-- Data stores
gui.create_store(cols)           -- Create a multi-column store
gui.store_append(store, row)    -- Add row to store
gui.store_get(store, index)     -- Get row from store

-- Dialogs
gui.alert(msg)                   -- Show modal alert
gui.confirm(msg)                 -- Show modal confirm dialog
gui.prompt(title, msg, default)  -- Show modal prompt

-- Event loop control
gui.loop()                       -- Process events (no-op in commander_runner)

-- Handler registration
gui.register_handler(name, fn)   -- Register callback handler
gui.modal_loop(check_fn)         -- Run modal loop while condition true
```

### Parser Module

```lua
local parser = require('parser')

ast = parser.parse(source)  -- Parse LUI source to AST
```

### Macros Module

```lua
local macros = require('macros')

macros.expand(ast)                    -- Expand macros in AST
macros.register(name, args, body)    -- Register custom macro
```

### Registry Module

```lua
local registry = require('registry')

registry.register(name, def)  -- Register widget type
registry.get(name)           -- Get widget definition
```

### Low-Level Xt Bridge

```lua
-- Widget creation (internal use)
w = xtcreate(id, class, parent, prop1, val1, ...)

-- Property manipulation
xtsetvalue(w, prop, val)   -- Set widget property
xtgetvalue(w, prop)         -- Get widget property
xtmanage(w)                 -- Manage widget
xtunmanage(w)               -- Unmanage widget
xtdestroy(w)                -- Destroy widget
xtaction(w, action, ...)    -- Call Xt action
```

### MLS (Memory List System)

```lua
handle = mls_create(size, width)     -- Create list
mls_put_string(handle, s)            -- Add string to list
mls_clear(handle)                    -- Clear list
mls_len(handle)                      -- Get list length
mls_get_string(handle, index)        -- Get string at index
```

---

## Widget Creation

### Available Widgets

From `registry.lua`:

| Tag | Class | Description |
|-----|-------|-------------|
| `window` | topLevelShellWidgetClass | Application window |
| `label` | Wlabel | Text label |
| `button` | Wbutton | Push button |
| `edit` | Wedit | Text entry |
| `grid` | Gridbox | Grid layout container |
| `vertical` | Gridbox | Vertical box (alias) |
| `horizontal` | Gridbox | Horizontal box (alias) |
| `image` | IconSVG | SVG image |
| `separator` | WSeparator | Horizontal separator |
| `check` | Wradio | Checkbox/radio button |
| `scrolled` | ScrolledCanvas | Scrolled container |
| `list-view` | WlsMulti | Multi-column list |
| `splitter` | Wsplitter | Resizable split pane |
| `vslider` | VSlider | Vertical slider |
| `list4` | Wlist4 | List widget |

### Basic Widget Syntax

```
(widget-type :prop1 value1 :prop2 value2
  (child-widget :prop value)
  (child-widget :prop value))
```

### Example: Window with Button

```lua
_G.quit_cb = function()
    os.exit(0)
end

local source = [[
(window :title "Demo" :width 300 :height 200
  (grid
    (button :label "Click Me" :gridx 0 :gridy 0 :callback "quit_cb")))
]]

local lui = require('lui')
lui.run(source)
```

---

## Layout Patterns

### Grid Layout

```lua
-- gridx, gridy: position in grid
-- weightx, weighty: resize priority (0 = fixed)
-- fill: 0=none, 1=horizontal, 2=vertical, 3=both

(source = [[
(window :title "Grid Demo" :width 400 :height 300
  (grid
    (label :label "Top Left" :gridx 0 :gridy 0)
    (label :label "Top Right" :gridx 1 :gridy 0)
    (button :label "Bottom" :gridx 0 :gridy 1 :gridWidth 2)))
]]
```

### Splitter Layout

```lua
-- splitter: resizable split panes
-- orientation: "vertical" or "horizontal"
-- fraction: initial split position (0.0-1.0)

(source = [[
(window :title "Splitter Demo" :width 500 :height 400
  (splitter :id "main_split" :orientation "vertical" :fraction 0.3
    (label :label "Top Pane" :gridx 0 :gridy 0)
    (label :label "Bottom Pane" :gridx 0 :gridy 0)))
]]
```

---

## Callbacks

### Callback Pattern 1: Global Function

```lua
-- Define callback in _G
_G.my_callback = function(data)
    print("Button clicked!")
    gui.set("my_label", "label", "Updated!")
end

-- Reference by name in LUI
source = [[
(window :title "Callback Demo"
  (grid
    (label :id "my_label" :label "Original")
    (button :label "Click" :callback "my_callback")))
]]
```

### Callback Pattern 2: LUA() Wrapper

```lua
-- Use LUA() wrapper for explicit callback
source = [[
(button :label "Click" :callback "LUA(quit_cb)")
]]
```

### Callback with Data

```lua
-- Callbacks receive optional data
_G.on_row = function(data)
    print("Row activated:", data)
end

source = [[
(list-view :id "list" :on-row-activated "LUA(on_row)")
]]
```

### Input Callbacks

```lua
_G.input_cb = function()
    local text = gui.get("my_edit", "label")
    gui.set("result_label", "label", "You typed: " .. text)
end

source = [[
(grid
  (edit :id "my_edit" :label "Type here...")
  (button :label "Submit" :callback "LUA(input_cb)")
  (label :id "result_label" :label ""))
]]
```

---

## Data Binding

### Store Pattern

LUI uses placeholder replacement for data binding:

```lua
-- Create a data store with N columns
local store = gui.create_store(3)

-- Add rows
gui.store_append(store, {"Item 1", "Active", "100"})
gui.store_append(store, {"Item 2", "Pending", "250"})
gui.store_append(store, {"Item 3", "Inactive", "50"})

-- Build LUI source with placeholder
local source = [[
(window :title "Data Binding Demo" :width 500 :height 400
  (grid
    (label :label "Multi-Column List")
    (list-view :id "mylist" :model $PLACEHOLDER
               :columns (150 150 100)
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3)
    (button :label "Quit" :gridx 0 :gridy 2 :callback "quit_cb")))
]]

-- Replace placeholder with store handle
source = source:gsub("$PLACEHOLDER", tostring(store.handle))

-- Run
local lui = require('lui')
lui.run(source)
```

### Multiple Stores

```lua
-- Two stores for two list-views
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
               :on-row-activated "LUA(on_file_select)"))
]]

source = source:gsub("$SIDEBAR_STORE", tostring(sidebar_store.handle))
source = source:gsub("$FILES_STORE", tostring(files_store.handle))
```

---

## Advanced: Task/Threading

The task manager provides background task support:

```c
// From task_manager.h
typedef struct {
    void *arg;
    int command;      // TASK_CMD_* constants
    int status;
    int progress;
} task_thread_args_t;

// Task commands
#define TASK_CMD_RUN    1
#define TASK_CMD_STOP   2
#define TASK_CMD_STATUS 3

// Registration
task_register_func("my_task", my_task_function);

// Lua binding (via lua_task_bindings.c)
```

See `experimental/task_manager.c` and `experimental/lua_task_bindings.c` for details.

---

## Examples

### Example 1: Simple Window

```lua
-- simple.lua
_G.quit_cb = function()
    os.exit(0)
end

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
_G.quit_cb = function()
    os.exit(0)
end

local gui = require('gui_xt')

-- Create store
local list_store = gui.create_store(3)
gui.store_append(list_store, {"Item 1", "Active", "100"})
gui.store_append(list_store, {"Item 2", "Pending", "250"})
gui.store_append(list_store, {"Item 3", "Inactive", "50"})
for i = 4, 20 do
    gui.store_append(list_store, {"Item " .. i, "Status " .. i, tostring(i * 10)})
end

-- UI
local lui_source = [[
(window :title "List Demo" :width 500 :height 400
  (grid
    (label :label "Data List" :gridx 0 :gridy 0 :fontSize 24)
    (list-view :id "mylist" :model $STORE
               :columns (150 150 100)
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3)
    (button :label "Quit" :gridx 0 :gridy 2 :callback "quit_cb")))
]]

lui_source = lui_source:gsub("$STORE", tostring(list_store.handle))

local lui = require('lui')
lui.run(lui_source)
```

### Example 3: File Manager UI

```lua
-- file_manager.lua
_G.quit_cb = function() os.exit(0) end
_G.on_file_select = function(data) print("Selected:", data) end

local gui = require('gui_xt')

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

local lui = require('lui')
lui.run(source)
```

### Example 4: Input Form

```lua
-- form.lua
_G.quit_cb = function() os.exit(0) end
_G.submit_cb = function()
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

Commander Runner provides the Xt event loop from C code. LUI scripts should NOT call `lui.loop()` or `gui.loop()` — these are no-ops when running under commander_runner.

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

The shell is created by `XtOpenApplication()`. The first LUI widget becomes a managed container child of the shell.

### Callback Flow

1. Widget triggers callback (click, activation, etc.)
2. Xt calls `LUA_action` registered in commander_runner
3. `LUA_action` queues callback via `luaxt_pushcallback()`
4. `process_lua_cbs` timeout processes queued callbacks
5. Lua function is executed via `luaL_dostring()`

---

## Troubleshooting

### "widget not found"

Ensure the widget ID is correct and the widget exists before accessing it.

### "Unknown widget tag"

The widget type isn't registered. Check `lui/registry.lua` for available widgets.

### Script blocks/hangs

Ensure your script doesn't call `lui.loop()` — commander_runner provides the event loop.

### Property not set

Some properties may need to be processed by `backend_xt.lua`. Check the property mapping in `backend_xt.process_property()`.
