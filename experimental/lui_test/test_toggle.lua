local test = require('test_harness')
local gui = require('gui_xt')

local ui = [[
(window :id "test_win" :title "Toggle Test"
  (grid :id "grid1"
    (toggle :id "tog1" :label "Toggle Me" :callback "LUA(on_toggle)")
    (label :id "status" :label "Off")
  ))
]]

local toggle_state = false

function on_toggle()
    toggle_state = not toggle_state
    gui.set("status", "label", toggle_state and "On" or "Off")
end

lui.run(ui)

test.sequence({
    { action = "capture_start" },
    { action = "click", widget = "tog1" },
    { action = "expect", widget = "tog1", event = "highlight" },
    { action = "expect", widget = "tog1", event = "notify" },
    { action = "click", widget = "tog1" },
    { action = "expect", widget = "tog1", event = "highlight" },
    { action = "expect", widget = "tog1", event = "notify" },
    { action = "verify" }
})