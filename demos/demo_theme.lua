-- Theme registry demo: switch colour themes and toggle widget borders.
local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "w" :title "Theme Demo" :width 680 :height 440
  (grid :id "g" :weightx 1 :weighty 1
    (Wlabel :id "hdr" :label "Central theme registry (colours + borders)"
            :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (Wlabel  :id "l1" :label "Wlabel"  :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3)
    (Wbutton :id "b1" :label "Wbutton" :gridx 1 :gridy 1 :weightx 1 :weighty 1 :fill 3)
    (EqFader :id "f1" :label "1 kHz"   :gridx 2 :gridy 1 :weightx 1 :weighty 1 :fill 3 :xftFont "Sans-10")
    (Wbutton :id "sel_def"  :label "default"        :gridx 0 :gridy 2 :weightx 1 :weighty 0 :fill 3 :callback "LUA(sel_default)")
    (Wbutton :id "sel_hi"   :label "highcontrast"   :gridx 1 :gridy 2 :weightx 1 :weighty 0 :fill 3 :callback "LUA(sel_hi)")
    (Wbutton :id "sel_test" :label "test (border)"  :gridx 2 :gridy 2 :weightx 1 :weighty 0 :fill 3 :callback "LUA(sel_test)")
    (Wbutton :id "q" :label "Quit" :gridx 0 :gridy 3 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :callback "quit_cb")
  ))
]]

function sel_default() theme_select("default") end
function sel_hi()      theme_select("highcontrast") end
function sel_test()    theme_select("test") end

local ok, err = lui.run(ui)
if not ok then
    error("demo_theme: " .. tostring(err))
end
