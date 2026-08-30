local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Wretex Debug" :width 700 :height 500
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "info" :label "Wretex Debug: Two widgets with fixed size" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :alignment 1 :fontSize 16)

    (retex :id "simple" :text "First retex widget\nWith two lines"
            :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill "both" :alignment 0 :fontSize 14
            :width 600 :height 80 :autoHeight FALSE)

    (retex :id "simple2" :text "Second retex widget\nAlso two lines"
            :gridx 0 :gridy 2 :weightx 1 :weighty 1 :fill "both" :alignment 0 :fontSize 14
            :width 600 :height 80 :autoHeight FALSE)

    (command :id "quit_btn" :label "Quit" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :height 30 :callback "LUA(on_quit)")
  ))
]]

function on_quit()
    xtappexitflag()
end

lui.run(ui)
