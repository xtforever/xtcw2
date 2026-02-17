package.path = '../lui/?.lua;' .. package.path

local parser = require('parser')
local macros = require('macros')
local backend = require('backend_xt')
local gui = require('gui_xt')

local lui_src = [[
(window :id "top" :title "Rasterize Test" :width 600 :height 400
  (grid
    (image :id "img" :gridx 0 :gridy 0 :width 300 :height 300 :src "../LuaRunner/alert.svg")
    (button :id "btn1" :gridx 0 :gridy 1 :label "Rasterize 50px" :on-click "LUA(small_cb)")
    (button :id "btn2" :gridx 1 :gridy 1 :label "Rasterize 200px" :on-click "LUA(large_cb)")
    (button :id "btn3" :gridx 0 :gridy 2 :label "Rasterize 300px" :on-click "LUA(reset_cb)")
    (button :id "quit" :gridx 1 :gridy 2 :label "Quit" :on-click "LUA(quit_cb)")
  )
)]]

function small_cb()
    print("Setting rasterize-width to 50")
    gui.set("img", "rasterize-width", 50)
end

function large_cb()
    print("Setting rasterize-width to 200")
    gui.set("img", "rasterize-width", 200)
end

function reset_cb()
    print("Setting rasterize-width to 300")
    gui.set("img", "rasterize-width", 300)
end

function quit_cb()
    os.exit(0)
end

print('Parsing UI...')
local ast = parser.parse(lui_src)
ast = macros.expand(ast)

print('Building UI...')
for _, node in ipairs(ast) do
    backend.build(node)
end

print('UI build complete. Starting loop...')
gui.loop()
