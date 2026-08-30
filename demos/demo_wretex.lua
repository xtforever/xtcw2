local gui = require('gui_xt')
local ui = [[
(window :id "w" :title "Wretex Widget Demo" :width 700 :height 500
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "info" :label "Wretex: Multiline Retex with SVG Import" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :alignment 1 :fontSize 16)

    (retex :id "simple" :text "This is a simple multiline paragraph.\nNewlines create new lines automatically.\nNo need for complex markup."
            :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3 :alignment 0 :fontSize 14)

    (retex :id "svg_inline" :text "Inline SVG support: \\includesvg{../LuaRunner/alert.svg}{14pt}{14pt} Alert icon\nand another: \\includesvg{../LuaRunner/question.svg}{14pt}{14pt} Question icon"
            :gridx 0 :gridy 2 :weightx 1 :weighty 1 :fill 3 :alignment 1 :fontSize 14)

    (retex :id "centered" :text "Centered paragraph with\\nmultiple lines of text\\nand automatic word wrapping"
            :gridx 0 :gridy 3 :weightx 1 :weighty 1 :fill 3 :alignment 1 :fontSize 16)

    (retex :id "right_aligned" :text "Right aligned text\\nwith multiple lines\\nand \\includesvg{../LuaRunner/ink1.svg}{12pt}{12pt} inline SVG"
            :gridx 0 :gridy 4 :weightx 1 :weighty 1 :fill 3 :alignment 2 :fontSize 13)

    (retex :id "justify" :text "Justified text that wraps across multiple lines. The Wretex widget uses the retex layout engine to handle line breaking, spacing, and alignment automatically. It also supports inline SVG images via the \\\\includesvg command."
            :gridx 0 :gridy 5 :weightx 1 :weighty 1 :fill 3 :alignment 3 :fontSize 12)

    (command :id "quit_btn" :label "Quit" :gridx 0 :gridy 6 :weightx 1 :weighty 0 :height 30 :callback "LUA(on_quit)")
  ))
]]

function on_quit()
    xtappexitflag()
end

lui.run(ui)
