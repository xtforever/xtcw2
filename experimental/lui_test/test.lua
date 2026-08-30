package.path = '../lui/?.lua;' .. package.path

local parser = require('parser')
local backend = require('backend_xt')
local gui = require('gui_xt')

local lui_src = [[
(window :id "top" :title "LUI Input Test"
  (grid :id "grid"
    (edit :id "input" :gridx 0 :gridy 0 :width 500 :height 50 :callback "LUA(input_cb)")
    (label :id "greeting" :gridx 0 :gridy 1 :width 500 :height 300 :managed false)
    (button :id "quit" :gridx 0 :gridy 2 :label "Quit" :callback "on_quit")
  )
)]]

function input_cb()
    local name = gui.get("input", "label")
    print("Greeting name:", name)
    gui.set("greeting", "label", "Hello, " .. (name or "Stranger") .. "! Welcome to LUI.")
    gui.manage("greeting")
end

function on_quit()
    print("Quitting...")
    xtappexitflag()
end

print('Building UI...')
local ast = parser.parse(lui_src)
if not ast then error('Failed to parse LUI') end

for _, node in ipairs(ast) do
    backend.build(node)
end

print('UI build complete. Starting loop...')
gui.loop()
