package.path = '../lui/?.lua;' .. package.path

local parser = require('parser')
local macros = require('macros')
local backend = require('backend_xt')
local gui = require('gui_xt')

local lui_src = [[
(defmacro g1(x y) :gridx x :gridy y :weightx 1 :weighty 1)
(defmacro g2(n x y) :id n :gridx x :gridy y :weightx 1 :weighty 1 :gridWidth 2 )

(window :id "top" :title "LUI Improved Test" :width 600 :height 400
  (grid :id "grid"
    (label :id "msg" (g1 0 0) :width 500 :height 100 :label "Click the button below")
    (image :id "img" (g1 1 0) :width 100 :height 100 :src "../LuaRunner/alert.svg")
    (button (g2 "btn1" 0 1) :label "Click Me" :on-click "LUA(handler_with_data button_1_pushed)")
    (button (g2 "btn2" 0 2) :label "Lua Function" :on-click nil)
    (button (g2 "quit" 0 3) :label "Quit" :on-click "quit_cb")
  )
)]]

function handler_with_data(data)
    print("Handler called with data:", data)
    gui.set("msg", "label", "Button pushed: " .. tostring(data))
end

function quit_cb()
    print("Quitting...")
    os.exit(0)
end

print('Parsing UI...')
local ast = parser.parse(lui_src)
if not ast then error('Failed to parse LUI') end

print('Initial AST:')
print(parser.dump(ast))

print('Expanding macros...')
ast = macros.expand(ast)

print('Expanded AST:')
print(parser.dump(ast))

-- Modify AST to add a direct Lua function to btn2
-- AST structure: { { "window", {keyword=":id"}, "top", ... } }
-- For simplicity, let's just use the build system to find btn2 and set its callback
-- Or better, we can now use Lua functions in the AST if we build it manually or improve the parser.
-- The parser currently returns tables for keywords.

-- Let's build first, then we can register more.
print('Building UI...')
for _, node in ipairs(ast) do
    backend.build(node)
end

-- btn2 callback - use global function approach
_G.btn2_callback = function(data)
    print("btn2 callback called!")
    gui.set("msg", "label", "Direct Lua callback worked!")
end
gui.set("btn2", "on-click", "btn2_callback")

-- Let's try to add the function to the AST before building.
-- Actually, my improved backend.build handles functions!

local lui_src_2 = {
    "window", {keyword=":id"}, "top2", {keyword=":title"}, "LUI Direct Function Test",
    {keyword=":width"}, 400, {keyword=":height"}, 300,
    {
        "grid", {keyword=":id"}, "grid2",
        {
            "button", {keyword=":id"}, "btn_direct", {keyword=":label"}, "Direct Function",
            {keyword=":on-click"}, function(data)
                print("Direct function callback executed!")
                gui.alert("It works!")
            end
        },
        {
            "button", {keyword=":id"}, "btn_quit", {keyword=":label"}, "Quit",
            {keyword=":on-click"}, function() os.exit(0) end
        }
    }
}

print('Building UI with direct functions...')
backend.build(lui_src_2)

print('UI build complete. Starting loop...')
gui.loop()
