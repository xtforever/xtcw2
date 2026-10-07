local gui = require('gui_xt')

local ui = [[
(defmacro grid_opts (gx gy wx wy)
  :gridx gx :gridy gy :weightx wx :weighty wy)

(window :id "w" :title "Wbutton Demo" :width 500 :height 420
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "hdr" :label "Wbutton Demo" (grid_opts 0 0 1 0) :gridWidth 2 :fill 3 :alignment 1 :fontSize 18)

    (Wbutton :id "btn1" :label "Button 1 (default)" (grid_opts 0 1 1 0) :fill 3 :callback "LUA(on_button)")
    (Wbutton :id "btn2" :label "Button 2 (large)" (grid_opts 1 1 1 0) :fill 3 :fontSize 20 :callback "LUA(on_button)")

    (Wbutton :id "btn3" :label "Button 3 (centered)" (grid_opts 0 2 1 0) :fill 3 :alignment 1 :callback "LUA(on_button)")
    (Wbutton :id "btn4" :label "Button 4 (right)" (grid_opts 1 2 1 0) :fill 3 :alignment 2 :callback "LUA(on_button)")

    (label :id "status" :label "Status: idle" (grid_opts 0 3 1 0) :gridWidth 2 :fill 3 :alignment 1 :fontSize 14)

    (Wbutton :id "quit_btn" :label "Quit" (grid_opts 0 4 1 0) :gridWidth 2 :fill 3 :height 34 :callback "quit_cb")
  ))
]]

local clicks = 0

function on_button(data)
    clicks = clicks + 1
    gui.set("status", "label", "Status: clicked (" .. clicks .. ")")
end

lui.run(ui)
