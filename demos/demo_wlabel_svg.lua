local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Wlabel with SVG Demo" :width 600 :height 400
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "info" :label "Labels can include inline SVG images:" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :alignment 1 :fontSize 16)
    
    (Wlabel :id "left_svg" :label "Alert: \\includesvg{../LuaRunner/alert.svg}{12pt}{12pt} (left aligned)" 
            :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3 :alignment 0 :fontSize 14)
    
    (Wlabel :id "center_svg" :label "\\includesvg{../LuaRunner/question.svg}{16pt}{16pt} Question (centered)" 
            :gridx 0 :gridy 2 :weightx 1 :weighty 1 :fill 3 :alignment 1 :fontSize 14)
    
    (Wlabel :id "right_svg" :label "Right aligned \\includesvg{../LuaRunner/ink1.svg}{14pt}{14pt}" 
            :gridx 0 :gridy 3 :weightx 1 :weighty 1 :fill 3 :alignment 2 :fontSize 14)
    
    (Wlabel :id "multi_svg" :label "First \\includesvg{../LuaRunner/alert.svg}{10pt}{10pt} then \\includesvg{../LuaRunner/question.svg}{12pt}{12pt} and \\includesvg{../LuaRunner/ink1.svg}{8pt}{8pt} (mixed)" 
            :gridx 0 :gridy 4 :weightx 1 :weighty 1 :fill 3 :alignment 1 :fontSize 12)
    
    (Wlabel :id "big_svg" :label "Big: \\includesvg{../LuaRunner/alert.svg}{24pt}{24pt}" 
            :gridx 0 :gridy 5 :weightx 1 :weighty 1 :fill 3 :alignment 1 :fontSize 18)
    
    (command :id "quit_btn" :label "Quit" :gridx 0 :gridy 6 :weightx 1 :weighty 0 :height 30 :callback "LUA(on_quit)")
  ))
]]

function on_quit()
    xtappexitflag()
end

lui.run(ui)
