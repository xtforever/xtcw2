local gui = require('gui_xt')

local ui = [[
(window :id "w" :title "DrawCanvas Paint" :width 520 :height 460
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "hdr" :label "DrawCanvas — drag to draw"
           :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :alignment 1 :fontSize 16)

    (DrawCanvas :id "c" :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3
                :xftFont "Sans-12"
                :bg "#ffffff" :foreground "#202020"
                :callback "drawcanvas_draw_cb"
                :keyCallback "drawcanvas_key_cb"
                :mouseCallback "drawcanvas_mouse_cb"
                :updateInterval 30
                :doubleBuffer TRUE
                :takeFocus TRUE)

    (label :id "note" :label "Drag to draw · C clears · Esc quits"
           :gridx 0 :gridy 2 :weightx 1 :weighty 0 :fill 3 :alignment 1 :fontSize 12)

    (button :id "quit_btn" :label "Quit" :gridx 0 :gridy 3 :weightx 1 :weighty 0
            :fill 3 :height 34 :callback "quit_cb")
  ))
]]

lui.run(ui)
