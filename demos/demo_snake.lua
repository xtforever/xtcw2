local gui = require('gui_xt')

local ui = [[
(window :id "w" :title "Snake" :width 400 :height 370
  (grid :id "g" :weightx 1 :weighty 1
    (DrawCanvas :id "c" :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3
                :xftFont "Sans-11"
                :callback "snake_draw_cb"
                :keyCallback "snake_key_cb"
                :mouseCallback "snake_mouse_cb"
                :tickCallback "snake_tick_cb"
                :updateInterval 30
                :doubleBuffer TRUE
                :takeFocus TRUE)

    (label :id "hint" :label "Arrows/WASD · P pause · Esc quit"
           :gridx 0 :gridy 1 :weightx 1 :weighty 0 :fill 3 :alignment 1 :fontSize 11)

    (button :id "quit_btn" :label "Quit" :gridx 0 :gridy 2 :weightx 1 :weighty 0
            :fill 3 :height 32 :callback "quit_cb")
  ))
]]

lui.run(ui)
