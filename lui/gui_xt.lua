local backend = require('backend_xt')
local luaxt = require('luaxt')

local M = {}
local handlers = {}

function M.set(id, prop, val)
    local w = backend.get_widget(id)
    if w then
        xtsetvalue(w, prop, tostring(val))
    end
end

function M.get(id, prop)
    local w = backend.get_widget(id)
    if w then
        return xtgetvalue(w, prop)
    end
end

function M.manage(id)
    local w = backend.get_widget(id)
    if w then xtmanage(w) end
end

function M.unmanage(id)
    local w = backend.get_widget(id)
    if w then xtunmanage(w) end
end

function M.loop()
    print('Entering LUI event loop...')
    while luaxt.processevent() == 0 do
        local cb_name = luaxt.pullcallback()
        if cb_name ~= '' then
            print('Callback triggered:', cb_name)
            local h = handlers[cb_name]
            if h then
                h()
            else
                -- Try to find it in globals
                if _G[cb_name] then
                    _G[cb_name]()
                end
            end
        end
        -- Small sleep to avoid 100% CPU if no events
        -- os.execute('sleep 0.01') -- too slow
    end
end

function M.register_handler(name, fn)
    handlers[name] = fn
end

_G.gui = M
return M
