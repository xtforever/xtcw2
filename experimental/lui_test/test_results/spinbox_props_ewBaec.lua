local gui = require('gui_xt')
local ui = [[
(window :id "tw" :title "SpinBox Props" :width 300 :height 100
  (grid :id "gr"
    (WspinBox :id "spin" :gridx 0 :gridy 0 :value 5 :min 0 :max 100)
  ))
]]
lui.run(ui)
xtapptimeout(500, function()
    local f = io.open("/home/jens/git/xtcw2/lui_test/test_results/spinbox_props_result.txt", "w")
    f:write("value_init=" .. tostring(gui.get("spin", "value")) .. "\n")
    f:write("min_init=" .. tostring(gui.get("spin", "min")) .. "\n")
    f:write("max_init=" .. tostring(gui.get("spin", "max")) .. "\n")
    gui.set("spin", "value", 42)
    f:write("value_after=" .. tostring(gui.get("spin", "value")) .. "\n")
    local w = gui.get_widget("spin")
    f:write("created=" .. tostring(w ~= nil) .. "\n")
    f:close()
    xtapptimeout(200, function() xtappexitflag() end)
end)
