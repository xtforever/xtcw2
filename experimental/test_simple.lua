_G.quit_cb = function()
    os.exit(0)
end

local source = [[
(window :title "Test Window" :width 300 :height 200
  (grid
    (label :label "Hello from Commander Runner!" :gridx 0 :gridy 0)
    (button :label "Quit" :gridx 0 :gridy 1 :callback "quit_cb")))
]]

local lui = require('lui')
lui.run(source)
