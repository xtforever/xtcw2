local test = require('test_harness')
local gui = require('gui_xt')

local ui = [[
(window :id "test_win" :title "List Test"
  (grid :id "grid1"
    (wlist4 :id "list1" :callback "LUA(on_select)" :vscroll "LUA(on_scroll)")
  ))
]]

function on_select(data)
    gui.set("status", "label", "Selected: " .. tostring(data))
end

function on_scroll(data)
end

lui.run(ui)

local store = gui.create_store({"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"})

xtsetvalue(test.get_widget("list1"), "model", tostring(store.handle))

test.sequence({
    { action = "capture_start" },
    { action = "action", widget = "list1", name = "highlight" },
    { action = "expect", widget = "list1", event = "highlight" },
    { action = "action", widget = "list1", name = "reset" },
    { action = "expect", widget = "list1", event = "reset" },
    { action = "verify" }
})