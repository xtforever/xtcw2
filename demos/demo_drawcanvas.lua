local gui = require('gui_xt')

local ui = [[
(window :id "w" :title "DrawCanvas Demo" :width 500 :height 420
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "hdr" :label "DrawCanvas — drawing via plug-in callback"
           :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :alignment 1 :fontSize 16)

    (DrawCanvas :id "c" :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3
                :callback "drawcanvas_draw_cb")

    (label :id "note" :label "The drawing lives in wbuild_widgets/drawcanvas-draw.c"
           :gridx 0 :gridy 2 :weightx 1 :weighty 0 :fill 3 :alignment 1 :fontSize 12)

    (button :id "quit_btn" :label "Quit" :gridx 0 :gridy 3 :weightx 1 :weighty 0
            :fill 3 :height 34 :callback "quit_cb")
  ))
]]

lui.run(ui)
