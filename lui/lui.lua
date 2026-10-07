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
    local first_widget = nil
    for _, node in ipairs(expanded_ast) do
        local w = backend.build(node)
        if not first_widget and w then
            first_widget = w
        end
    end
    -- Manage the root shell widget so it becomes visible
    if first_widget then
        gui.manage(first_widget)
    end
    return true
end

function M.load(filename)
    local f = io.open(filename, "r")
    if not f then return nil, "Could not open file" end
    local source = f:read("*a")
    f:close()
    
    local trimmed = source:match("^%s*()")
    if trimmed and source:sub(trimmed, trimmed) == "(" then
        return M.run(source)
    end
    
    local chunk, err = load(source, filename, "t", _G)
    if not chunk then
        return nil, "Lua syntax error: " .. tostring(err)
    end
    
    local ok, run_err = pcall(chunk)
    if not ok then
        return nil, "Execution error: " .. tostring(run_err)
    end
    
    if _G.lui_source then
        local result = M.run(_G.lui_source)
        _G.lui_source = nil
        return result
    end
    
    return true
end

function M.loop()
    gui.loop()
end

_G.lui = M
return M
