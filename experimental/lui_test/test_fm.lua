package.path = '../lui/?.lua;' .. package.path

local parser = require('parser')
local backend = require('backend_xt')
local gui = require('gui_xt')

local lui_src = [[
(window :id "main" :title "FM Test"
  (grid :id "grid"
    (label :label "Store Test" :gridx 0 :gridy 0)
    (list-view :id "list" :gridx 0 :gridy 1 :width 300 :height 200 :on-row-activated "LUA(on_row)")
    (button :label "Add Item" :gridx 0 :gridy 2 :callback "LUA(add_item)")
    (button :label "Dialog Test" :gridx 0 :gridy 3 :callback "LUA(test_dialog)")
    (button :label "Quit" :gridx 0 :gridy 4 :callback "LUA(quit_app)")
  )
)]]

local store = gui.create_store({'string', 'int'})

function on_row()
    print('Row activated')
end

function add_item()
    gui.store_append(store, {'Item ' .. os.time(), 100})
    print('Item added to store (simulated)')
end

function test_dialog()
    if gui.confirm('Do you want to see an alert?') then
        gui.alert('Here is your alert!')
    end
    local name = gui.prompt('Name', 'Who are you?', 'User')
    if name then
        gui.alert('Hello ' .. name)
    end
end

function quit_app()
    os.exit(0)
end

local ast = parser.parse(lui_src)
for _, node in ipairs(ast) do backend.build(node) end

gui.loop()
