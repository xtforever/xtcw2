# Beginner'''s Guide to XTCW2 + LUI

This guide explains how to build lightweight, future-proof GUI applications using the **XTCW2** toolkit and the **LUI** declarative language.

## 1. Core Philosophy
*   **Declarative UI**: Define your interface using S-expressions (Lisp-like syntax).
*   **Lua Logic**: Write application behavior and callbacks in Lua.
*   **Minimalist**: High performance on old hardware, natively remote-capable over network (X11).
*   **Advanced Typography**: Integrated `re-tex` engine for high-quality text layout.

## 2. Basic UI Structure
An LUI application is defined as a nested tree of widgets.

```lisp
(window :id "main" :title "My App"
  (grid :id "container"
    (label :id "msg" :label "Hello World")
    (button :id "btn" :label "Click Me")
  )
)
```

## 3. Common Widgets & Properties

### `window`
The top-level container.
*   `:title`: Text shown in the window title bar.
*   `:width`, `:height`: Initial dimensions.

### `grid` (Gridbox)
Used for layout. Positions children using a grid coordinate system.
*   `:gridx`, `:gridy`: The column and row index for a child widget.

### `label` (Wlabel)
Displays text using the `re-tex` engine.
*   `:label`: The text to display. Supports `

` for paragraph breaks.
*   `:fontFace`: Font name (e.g., "Serif", "Sans", "Monospace").
*   `:fontSize`: Font size in points.
*   `:align`: Text alignment (see section 4).

### `edit` (Wedit)
A single-line text input field.
*   `:callback`: Name of the Lua function to call when Enter is pressed.

### `button` (Wbutton)
An interactive button.
*   `:label`: Text shown on the button.
*   `:callback`: Name of the Lua function to call when clicked.

---

## 4. Advanced Text Layout (`re-tex`)

The `label` widget uses the `re-tex` engine, providing professional typesetting features.

### Alignment Options
Use the `:align` property to control text justification:
*   `0`: **Left Justified** (Default)
*   `1`: **Centered**
*   `2`: **Right Justified**
*   `3`: **Full Justified**

**Example: Centered Text**
```lisp
(label :id "centered_text" 
       :label "This text is centered.
It looks beautiful."
       :align 1 
       :width 400)
```

---

## 5. Connecting Lua Logic

To make your GUI dynamic, define Lua functions and link them via callbacks. Use the `gui` module to interact with widgets.

### Example: Greeting App
Save this as `app.lua`:

```lua
local gui = require('''gui_xt''')

-- 1. Define the UI
local source = [[
(window :id "top" :title "Greeting App"
  (grid :id "grid"
    (edit :id "name_input" :gridx 0 :gridy 0 :width 400 :callback "LUA(on_submit)")
    (label :id "msg" :gridx 0 :gridy 1 :width 400 :height 100 :align 1 :label "Enter name above")
    (button :id "quit" :gridx 0 :gridy 2 :label "Quit" :callback "LUA(on_quit)")
  )
)]]

-- 2. Define Callbacks
function on_submit()
    local name = gui.get("name_input", "label")
    gui.set("msg", "label", "Hello, " .. name .. "!
Welcome to LUI.")
end

function on_quit()
    os.exit(0)
end

-- 3. Build and Run
local parser = require('''parser''')
local backend = require('''backend_xt''')
local ast = parser.parse(source)
for _, node in ipairs(ast) do backend.build(node) end

gui.loop()
```

Run it using the luarunner:
```bash
./luarunner -Luafile app.lua
```
