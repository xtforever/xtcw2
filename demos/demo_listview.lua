-- WListView demo: a themed multi-column list with header, scrollbars,
-- alternate rows, SVG/file icons and keyboard navigation.
--
-- The row model is set from Lua with gui.set_string_list() after the UI is
-- built.  A cell in an icon/icon+text column is "@path.svg|label" (icon+text)
-- or "@path.svg" (icon only); plain text is drawn as-is.

local gui = require('gui_xt')
local lui = require('lui')

local rows = {}
rows[1] = "@../LuaRunner/alert.svg|src\t-\tdir"
rows[2] = "@../LuaRunner/alert.svg|build\t-\tdir"
for i = 1, 60 do
    rows[#rows + 1] = string.format("file_%03d.txt\t%d KB\tfile", i, i * 3)
end

function on_select(i)
    gui.set("status", "label", "selected row " .. tostring(i))
end

function on_activate(i)
    gui.set("status", "label", "activated row " .. tostring(i))
end

local ui = [[
(window :id "win" :title "WListView" :width 640 :height 500
  (grid :id "main" :weightx 1 :weighty 1
    (WListView :id "lv"
       :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3
       :columnCount 3
       :columnTitles "Name\tSize\tType"
       :columnWidths "0,100,80"
       :columnAligns "0,2,0"
       :columnKinds "2,0,0"
       :showHeader 1
       :gridLines 1
       :alternateRows 1
       :visibleRows 12
       :on-row-activated "LUA(on_select)"
       :on-activate "LUA(on_activate)")
    (label :id "status" :label "Select a row (click, arrows, Home/End/PageUp/Down)"
           :gridx 0 :gridy 1 :weightx 1 :weighty 0 :fill 3)
  ))
]]

local ok, err = lui.run(ui)
if not ok then error("demo_listview: " .. tostring(err)) end
gui.set_string_list("lv", "rows", rows)
