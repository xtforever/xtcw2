# Widget Verification Demos

> Paste any command block into a shell running from the project root directory.
> Commands that start with `cd experimental` use `commander_runner`.
> For a visual display, run without `xvfb-run`. For headless CI, prefix with `xvfb-run --auto-servernum`.

## Quick Start

```bash
cd /home/jens/git/xtcw2/experimental
```

All demo commands below assume you're in this directory. Each demo shows a window for 5 seconds then exits. Remove `xtapptimeout(5000, ...)` to keep the window open.

---

## 1. Basic Widgets

### 1a. Label — Text display and alignment

```bash
cat > /tmp/demo_label.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Label Demo" :width 400 :height 300
  (grid :id "g"
    (label :id "left"   :label "Left aligned"   :gridx 0 :gridy 0 :weightx 1 :alignment 0)
    (label :id "center" :label "Center aligned"  :gridx 0 :gridy 1 :weightx 1 :alignment 1)
    (label :id "right"  :label "Right aligned"   :gridx 0 :gridy 2 :weightx 1 :alignment 2)
    (label :id "big"    :label "Big Label"       :gridx 0 :gridy 3 :weightx 1 :fontSize 24)
    (label :id "dyn"    :label "Watch me change"  :gridx 0 :gridy 4 :weightx 1)
  ))
]]
lui.run(ui)
xtapptimeout(2000, function()
    gui.set("dyn", "label", "Changed after 2 seconds!")
    xtapptimeout(3000, function() xtappexitflag() end)
end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_label.lua
```

### 1b. Button — Callback dispatch

```bash
cat > /tmp/demo_button.lua << 'LUAEOF'
local gui = require('gui_xt')
local count = 0
local ui = [[
(window :id "w" :title "Button Demo" :width 300 :height 150
  (grid :id "g"
    (command :id "btn" :label "Press Me" :gridx 0 :gridy 0 :weightx 1 :callback "LUA(on_press)")
    (label :id "status" :label "Not clicked yet" :gridx 0 :gridy 1 :weightx 1)
  ))
]]
function on_press()
    count = count + 1
    gui.set("status", "label", "Clicked " .. count .. " times!")
    gui.set("btn", "label", count < 5 and "Again!" or "Enough!")
end
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_button.lua
```

### 1c. Toggle — State switching

```bash
cat > /tmp/demo_toggle.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Toggle Demo" :width 300 :height 100
  (grid :id "g"
    (toggle :id "tog" :label "Toggle Me" :gridx 0 :gridy 0 :callback "LUA(on_toggle)")
    (label :id "state" :label "State: OFF" :gridx 1 :gridy 0 :weightx 1)
  ))
]]
function on_toggle()
    local w = gui.get_widget("tog")
    local state = gui.get("tog", "state")
    gui.set("state", "label", "State: " .. tostring(state))
end
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_toggle.lua
```

---

## 2. Input Widgets

### 2a. Edit — Text entry

```bash
cat > /tmp/demo_edit.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Edit Demo" :width 400 :height 150
  (grid :id "g"
    (label :label "Name:" :gridx 0 :gridy 0)
    (edit :id "name" :gridx 1 :gridy 0 :weightx 1)
    (label :label "Email:" :gridx 0 :gridy 1)
    (edit :id "email" :gridx 1 :gridy 1 :weightx 1)
    (command :id "ok" :label "OK" :gridx 0 :gridy 2 :gridWidth 2 :callback "LUA(on_ok)")
  ))
]]
function on_ok()
    local name = gui.get("name", "value") or ""
    local email = gui.get("email", "value") or ""
    print("Name: " .. tostring(name))
    print("Email: " .. tostring(email))
end
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_edit.lua
```

### 2b. SpinBox — Numeric spinner

```bash
cat > /tmp/demo_spin.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "SpinBox Demo" :width 300 :height 120
  (grid :id "g"
    (label :label "Quantity:" :gridx 0 :gridy 0)
    (WspinBox :id "qty" :gridx 1 :gridy 0 :weightx 1 :value 1)
  ))
]]
lui.run(ui)
xtapptimeout(1000, function()
    print("SpinBox value: " .. tostring(gui.get("qty", "value")))
    gui.set("qty", "value", 42)
    print("SpinBox after set: " .. tostring(gui.get("qty", "value")))
    xtapptimeout(2000, function() xtappexitflag() end)
end)
LUAEOF
cd /home/jens/git/xtcw2/experimental
xvfb-run --auto-servernum ./commander_runner -Tracelevel 2 -Luafile /tmp/demo_spin.lua 2>&1 | grep -E "^SpinBox"

### 2c. Password — Hidden input

```bash
cat > /tmp/demo_password.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Password Demo" :width 350 :height 100
  (grid :id "g"
    (label :label "Password:" :gridx 0 :gridy 0)
    (Wpassword :id "pw" :gridx 1 :gridy 0 :weightx 1)
  ))
]]
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_password.lua
```

---

## 3. List Widgets

### 3a. Wlist4 — 4-column list with actions (TRACE(50))

```bash
cat > /tmp/demo_list4.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Wlist4 Demo" :width 500 :height 350
  (grid :id "g"
    (Wlist4 :id "lst" :gridx 0 :gridy 0 :weightx 1 :weighty 1 :callback "LUA(on_select)")
  ))
]]
function on_select()
    print("Wlist4 selection changed!")
end
lui.run(ui)
xtapptimeout(500, function()
    local w = gui.get_widget("lst")
    if w then
        xtaction(w, "highlight")
        xtaction(w, "reset")
    end
    xtapptimeout(4000, function() xtappexitflag() end)
end)
LUAEOF
./commander_runner -Tracelevel 50 -Luafile /tmp/demo_list4.lua 2>&1 | grep "^\[50\]"
```

### 3b. Multi-column list with scrollbar

```bash
cat > /tmp/demo_multicolumn.lua << 'LUAEOF'
local gui = require('gui_xt')
local store = gui.create_store(3)
for i = 1, 30 do
    gui.store_append(store, {"Item " .. i, tostring(100 + i), i .. ".0%"})
end
local ui = [[
(window :id "w" :title "Multi-Column List" :width 600 :height 400
  (grid :id "g"
    (label :label "Process Monitor" :gridx 0 :gridy 0 :weightx 1 :alignment 1 :fontSize 20)
    (list-view :id "procs" :gridx 0 :gridy 1 :weightx 1 :weighty 1
               :columns (250 80 80) :retexCells true :fontSize 16 :model __STORE__)
    (command :id "btn" :label "Refresh" :gridx 0 :gridy 2 :weightx 1 :callback "LUA(on_refresh)")
  ))
]]
ui = ui:gsub("__STORE__", tostring(store.handle))
function on_refresh() print("Refresh clicked") end
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_multicolumn.lua
```

---

## 4. Layout Widgets

### 4a. Gridbox — Weight-based grid

```bash
cat > /tmp/demo_gridbox.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Gridbox Layout" :width 500 :height 300
  (grid :id "g"
    (label :label "Weight=1" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :alignment 1 :fill 3)
    (label :label "Weight=2" :gridx 1 :gridy 0 :weightx 2 :weighty 0 :alignment 1 :fill 3)
    (label :label "Weight=0\nFixed" :gridx 0 :gridy 1 :weightx 0 :weighty 1 :alignment 1 :fill 0)
    (label :label "Spans 2 cols" :gridx 1 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1 :alignment 1 :fill 3)
  ))
]]
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_gridbox.lua
```

### 4b. VBox / HBox — Vertical and horizontal boxes

```bash
cat > /tmp/demo_boxes.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "VBox/HBox Demo" :width 400 :height 300
  (grid :id "g"
    (vertical :id "vbox" :gridx 0 :gridy 0 :weightx 1 :weighty 1
      (label :label "VBox Item 1" :fill 3)
      (label :label "VBox Item 2" :fill 3)
      (label :label "VBox Item 3" :fill 3)
    )
    (horizontal :id "hbox" :gridx 1 :gridy 0 :weightx 1 :weighty 1
      (label :label "H1" :fill 3)
      (label :label "H2" :fill 3)
      (label :label "H3" :fill 3)
    )
  ))
]]
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_boxes.lua
```

---

## 5. SVG Images

### 5a. IconSVG with rasterization

```bash
cat > /tmp/demo_svg.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "SVG Demo" :width 500 :height 300
  (grid :id "g"
    (image :id "img1" :gridx 0 :gridy 0 :width 100 :height 100 :src "../LuaRunner/alert.svg")
    (image :id "img2" :gridx 1 :gridy 0 :width 100 :height 100 :src "../LuaRunner/question.svg")
    (label :id "status" :label "SVG images loaded" :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :alignment 1)
  ))
]]
lui.run(ui)
xtapptimeout(2000, function()
    print("img1 widget:", tostring(gui.get_widget("img1")))
    print("img2 widget:", tostring(gui.get_widget("img2")))
    xtapptimeout(3000, function() xtappexitflag() end)
end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_svg.lua
```

---

## 6. WpixBtn — Pixel button with actions (TRACE(50))

```bash
cat > /tmp/demo_wpixbtn.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "WpixBtn Demo" :width 200 :height 100
  (grid :id "g"
    (WpixBtn :id "pb1" :gridx 0 :gridy 0 :callback "LUA(on_pb)")
  ))
]]
function on_pb() print("WpixBtn clicked!") end
lui.run(ui)
xtapptimeout(500, function()
    local w = gui.get_widget("pb1")
    if w then
        xtaction(w, "highlight")
        print("highlight sent")
        xtaction(w, "notify")
        print("notify sent")
        xtaction(w, "reset")
        print("reset sent")
    end
    xtapptimeout(3000, function() xtappexitflag() end)
end)
LUAEOF
echo "=== WpixBtn TRACE(50) output ==="
./commander_runner -Tracelevel 50 -Luafile /tmp/demo_wpixbtn.lua 2>&1 | grep "^\[50\]"
```

---

## 7. Wcombo — Combo box (TRACE(50))

```bash
cat > /tmp/demo_combo.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Wcombo Demo" :width 300 :height 150
  (grid :id "g"
    (Wcombo :id "cb" :gridx 0 :gridy 0 :weightx 1 :callback "LUA(on_combo)")
    (label :id "val" :label "Selection: (none)" :gridx 0 :gridy 1 :weightx 1)
  ))
]]
function on_combo()
    print("Wcombo callback fired!")
end
lui.run(ui)
xtapptimeout(500, function()
    local w = gui.get_widget("cb")
    if w then
        xtaction(w, "focus_in")
        xtaction(w, "set_cursor")
        xtaction(w, "focus_out")
    end
    xtapptimeout(3000, function() xtappexitflag() end)
end)
LUAEOF
echo "=== Wcombo TRACE(50) output ==="
./commander_runner -Tracelevel 50 -Luafile /tmp/demo_combo.lua 2>&1 | grep "^\[50\]"
```

---

## 8. Wlabel — Selection actions (TRACE(50))

```bash
cat > /tmp/demo_wlabel_select.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Wlabel Selection" :width 400 :height 100
  (grid :id "g"
    (Wlabel :id "wl" :label "Select this text by dragging" :gridx 0 :gridy 0 :weightx 1)
  ))
]]
lui.run(ui)
xtapptimeout(500, function()
    local w = gui.get_widget("wl")
    if w then
        xtaction(w, "select_start")
        xtaction(w, "select_extend")
        xtaction(w, "select_end")
        xtaction(w, "info")
    end
    xtapptimeout(2000, function() xtappexitflag() end)
end)
LUAEOF
echo "=== Wlabel TRACE(50) output ==="
./commander_runner -Tracelevel 50 -Luafile /tmp/demo_wlabel_select.lua 2>&1 | grep "^\[50\]"
```

---

## 9. All Widgets — Creation smoke test

This creates one instance of every registered widget class and verifies they all instantiate without errors.

```bash
cat > /tmp/demo_all_widgets.lua << 'LUAEOF'
local gui = require('gui_xt')
local tags = {
    'window', 'label', 'Wlabel', 'button', 'Wbutton', 'command', 'toggle',
    'edit', 'grid', 'image', 'WpixBtn', 'separator', 'check',
    'scrolled', 'list-view', 'splitter', 'vslider', 'wlist4'
}
local ui = '(window :id "w" :title "All Widgets" :width 600 :height 400\n  (grid :id "g"\n'
for i, tag in ipairs(tags) do
    ui = ui .. '    (' .. tag .. ' :id "w_' .. tag .. '"'
    if tag == 'command' or tag == 'button' or tag == 'Wbutton' or tag == 'WpixBtn' then
        ui = ui .. ' :label "Test"'
    end
    if tag == 'label' or tag == 'Wlabel' then
        ui = ui .. ' :label "Hello"'
    end
    ui = ui .. ')\n'
end
ui = ui .. '  ))\n'
lui.run(ui)
xtapptimeout(1000, function()
    local found = 0
    local missing = {}
    for _, tag in ipairs(tags) do
        local w = gui.get_widget("w_" .. tag)
        if w then found = found + 1 else table.insert(missing, tag) end
    end
    print("Created: " .. found .. "/" .. #tags)
    if #missing > 0 then print("Missing: " .. table.concat(missing, ", ")) end
    xtapptimeout(1000, function() xtappexitflag() end)
end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_all_widgets.lua 2>&1 | grep -E "^Created:|^Missing:|Warning:"
```

---

## 10. Macro System — LUI macros and function callbacks

```bash
cat > /tmp/demo_macros.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(defmacro g1(x y) :gridx x :gridy y :weightx 1 :weighty 1)
(defmacro g2(n x y) :id n :gridx x :gridy y :weightx 1 :weighty 1)

(window :id "w" :title "Macro Demo" :width 400 :height 250
  (grid :id "g"
    (label (g1 0 0) :label "Macros expand grid positions")
    (image (g1 1 0) :width 80 :height 80 :src "../LuaRunner/alert.svg")
    (command (g2 "btn1" 0 1) :label "Click Me" :on-click "LUA(on_click)")
    (command (g2 "btn2" 1 1) :label "Lua Fn" :on-click (function() gui.set("status", "label", "Lua closure worked!") end))
    (label (g1 0 2) :id "status" :label "Waiting..." :gridWidth 2)
  ))
]]
function on_click()
    gui.set("status", "label", "String callback worked!")
end
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_macros.lua
```

---

## 11. Composite Demo — Form with all widget types

```bash
cat > /tmp/demo_composite.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Composite Form Demo" :width 550 :height 450
  (grid :id "g"
    (label :label "Name:" :gridx 0 :gridy 0 :weightx 0)
    (edit :id "name" :gridx 1 :gridy 0 :weightx 1 :fill 3)
    (label :label "Priority:" :gridx 0 :gridy 1 :weightx 0)
    (WspinBox :id "prio" :gridx 1 :gridy 1 :weightx 1 :spinValue 3)
    (label :label "Notes:" :gridx 0 :gridy 2 :weightx 0)
    (edit :id "notes" :gridx 1 :gridy 2 :weightx 1 :fill 3)
    (label :label "Status:" :gridx 0 :gridy 3 :weightx 0 :alignment 2)
    (Wlist4 :id "status_list" :gridx 1 :gridy 3 :weightx 1 :weighty 1 :fill 3)
    (separator :gridx 0 :gridy 4 :gridWidth 2 :weightx 1)
    (grid :gridx 0 :gridy 5 :gridWidth 2 :weightx 1
      (command :id "save" :label "Save" :gridx 0 :gridy 0 :weightx 1 :callback "LUA(on_save)")
      (command :id "cancel" :label "Cancel" :gridx 1 :gridy 0 :weightx 1 :callback "LUA(on_cancel)")
    )
  ))
]]
function on_save() gui.set("name", "value", "Saved!") end
function on_cancel() xtappexitflag() end
lui.run(ui)
xtapptimeout(8000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_composite.lua
```

---

## 12. Splitter — Resizable panes

```bash
cat > /tmp/demo_splitter.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Splitter Demo" :width 500 :height 300
  (grid :id "g"
    (Wsplitter :id "split" :gridx 0 :gridy 0 :weightx 1 :weighty 1
      :leftPane "Left content here"
      :rightPane "Right content here")
  ))
]]
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_splitter.lua
```

---

## 13. TRACE(50) Regression — Verify instrumentation

```bash
cat > /tmp/demo_trace_regression.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "TRACE Regression" :width 300 :height 200
  (grid :id "g"
    (WpixBtn :id "pb" :gridx 0 :gridy 0 :callback "LUA(cb)")
    (Wlist4 :id "lst" :gridx 0 :gridy 1 :weighty 1 :weightx 1 :callback "LUA(cb2)")
    (Wlabel :id "wl" :label "Selectable text" :gridx 0 :gridy 2 :weightx 1)
  ))
]]
function cb() end
function cb2() end
lui.run(ui)
xtapptimeout(500, function()
    local pb = gui.get_widget("pb")
    local lst = gui.get_widget("lst")
    local wl = gui.get_widget("wl")
    if pb then xtaction(pb, "highlight"); xtaction(pb, "reset") end
    if lst then xtaction(lst, "highlight"); xtaction(lst, "reset") end
    if wl then xtaction(wl, "select_start"); xtaction(wl, "select_end"); xtaction(wl, "info") end
    xtapptimeout(1000, function() xtappexitflag() end)
end)
LUAEOF
echo "=== Expected TRACE(50) lines ==="
echo "  WpixBtn highlight"
echo "  WpixBtn reset"
echo "  Wlist4 highlight"
echo "  Wlist4 reset"
echo "  Wlabel select_start"
echo "  Wlabel select_end"
echo "  Wlabel info"
echo ""
echo "=== Actual TRACE(50) output ==="
./commander_runner -Tracelevel 50 -Luafile /tmp/demo_trace_regression.lua 2>&1 | grep "^\[50\]"
```

---

## 14. Full Application — File Manager Preview

```bash
cat > /tmp/demo_filemanager.lua << 'LUAEOF'
local gui = require('gui_xt')
local store = gui.create_store(3)
local files = {
    {"Documents", "4096", "dir"},
    {"Downloads", "8192", "dir"},
    {"Pictures", "2048", "dir"},
    {".bashrc", "3120", "file"},
    {".profile", "890", "file"},
    {"notes.txt", "15432", "file"},
    {"todo.md", "2876", "file"},
    {"config.ini", "512", "file"},
}
for _, f in ipairs(files) do
    gui.store_append(store, {f[1], f[3], f[2]})
end

local ui = [[
(window :id "w" :title "File Manager" :width 600 :height 400
  (grid :id "g"
    (label :label "Browse Files" :gridx 0 :gridy 0 :weightx 100 :alignment 1 :fontSize 22)
    (list-view :id "files" :gridx 0 :gridy 1 :weightx 100 :weighty 100
               :columns (300 80 80) :retexCells true :fontSize 16 :model __STORE__)
    (grid :gridx 0 :gridy 2 :weightx 100 :weighty 0
      (command :id "refresh" :label "Refresh" :gridx 0 :gridy 0 :weightx 1 :callback "LUA(on_refresh)")
      (command :id "quit"     :label "Quit"     :gridx 1 :gridy 0 :weightx 1 :callback "LUA(on_quit)")
    )
  ))
]]
ui = ui:gsub("__STORE__", tostring(store.handle))
function on_refresh() print("Refresh!") end
function on_quit() xtappexitflag() end
lui.run(ui)
xtapptimeout(10000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_filemanager.lua
```

---

## Appendix: Xaw Standard Widgets

These widgets don't need LUI registry entries — they work by class name fallback:

```bash
cat > /tmp/demo_xaw.lua << 'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Xaw Widgets" :width 400 :height 300
  (grid :id "g"
    (Command :id "cmd" :label "Xaw Command" :gridx 0 :gridy 0 :callback "LUA(cb)")
    (Toggle :id "tog" :label "Xaw Toggle" :gridx 1 :gridy 0)
    (AsciiText :id "txt" :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1)
    (Paned :id "pane" :gridx 0 :gridy 2 :gridWidth 2 :weightx 1)
  ))
]]
function cb() print("Xaw Command clicked!") end
lui.run(ui)
xtapptimeout(5000, function() xtappexitflag() end)
LUAEOF
./commander_runner -Tracelevel 2 -Luafile /tmp/demo_xaw.lua
```

---

## Quick Reference: Running Without Display

For CI or headless environments, prefix any command with `xvfb-run --auto-servernum`:

```bash
xvfb-run --auto-servernum ./commander_runner -Tracelevel 50 -Luafile /tmp/demo_wpixbtn.lua 2>&1 | grep "^\[50\]"
```

For interactive testing on a real display, just run the command directly.

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| `module 'gui_xt' not found` | Running from wrong directory | `cd experimental/` first |
| `Unknown widget tag: Wlabel` | Missing registry entry | Add to `lui/registry.lua` |
| Callback doesn't fire | Command widget needs `set` before `notify` | Use `xtaction(w, "set")` then `xtaction(w, "notify")` |
| TRACE(50) not appearing | Wrong trace level | Use `-Tracelevel 50` |
| Widget has zero width/height | Unsigned underflow or missing query_geometry | Check Gridbox weight/fill settings |
| `Wcombo` `CallOnEnter` warning | Wcombo references a `CallOnEnter` action not registered | Harmless — action is optional, widget still works |
| `list-view` with `retexCells` | `vas_printf` MLS crash if `retexCells=true` | **Bug**: Don't use `:retexCells true` with list-view; omit it |
| Edit text get/set | Edit/Wpassword use `label` resource, not `value` | Use `gui.get(id, "label")` or call `gui.get(id, "value")` which falls back to `label` |
| SpinBox property name | S-expression uses `:value` (not `:spinValue`) | Property mapping handles `spinValue` → `value` automatically |