local test = require('test_harness')
local gui = require('gui_xt')

local ui = [[
(window :id "test_win" :title "Focus Test"
  (grid :id "grid1"
    (command :id "btn1" :label "First" :callback "LUA(on_btn1)")
    (command :id "btn2" :label "Second" :callback "LUA(on_btn2)")
  ))
]]

function on_btn1() end
function on_btn2() end

lui.run(ui)

test.sequence({
    { action = "capture_start" },
    { action = "focus_in", widget = "btn1" },
    { action = "expect", widget = "btn1", event = "focus_in" },
    { action = "focus_out", widget = "btn1" },
    { action = "expect", widget = "btn1", event = "focus_out" },
    { action = "click", widget = "btn1" },
    { action = "expect", widget = "btn1", event = "activate" },
    { action = "verify" }
})