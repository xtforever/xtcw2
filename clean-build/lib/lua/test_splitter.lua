local parser = require('parser')
local macros = require('macros')
local gui = require('gui_xt')
local backend = require('backend_xt')

_G.quit_cb = function()
    print("Quit requested")
    os.exit(0)
end

local lui_source = [[
(window :title "Splitter Test" :width 600 :height 400
  (grid
    (splitter :id "split" :orientation "vertical" :fraction 0.3
              :gridx 0 :gridy 0 :weightx 100 :weighty 100 :fill 3
      (label :label "Left Pane" :background "grey80")
      (label :label "Right Pane" :background "grey90"))
    (button :label "Quit" :gridx 0 :gridy 1 :callback "LUA(quit_cb)")))
]]

local ast = parser.parse(lui_source)
local expanded_ast = macros.expand(ast)
backend.build(expanded_ast[1])
gui.loop()
