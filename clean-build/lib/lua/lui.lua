local parser = require('parser')
local macros = require('macros')
local gui = require('gui_xt')
local backend = require('backend_xt')
require('list_scrollbar')

local M = {}

function M.run(lui_source)
    local ast = parser.parse(lui_source)
    if not ast then return nil, "Parse error" end
    local expanded_ast = macros.expand(ast)
    for _, node in ipairs(expanded_ast) do
        backend.build(node)
    end
    return true
end

function M.load(filename)
    local f = io.open(filename, "r")
    if not f then return nil, "Could not open file" end
    local source = f:read("*a")
    f:close()
    return M.run(source)
end

function M.loop()
    gui.loop()
end

function M.msgbox()
    gui.msgbox()
end

function M.copy_file(src, dst)
    gui.copy_file(src, dst)
end

function M.set_resource(name, res, val)
    gui.set_resource(name, res, val)
end

_G.lui = M
return M
