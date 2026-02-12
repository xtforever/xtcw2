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

function M.update(id)
    local w = backend.get_widget(id)
    if w then
        local handle = M.get(id, 'tableStrs')
        if handle then
            xtsetvalue(w, 'tableStrs', tostring(handle))
        end
    end
end

function M.loop()
    while luaxt.processevent() == 0 do
        local cb_name = luaxt.pullcallback()
        if cb_name ~= '' then
            local h = handlers[cb_name] or _G[cb_name]
            if h then h() end
        end
    end
end

function M.register_handler(name, fn)
    handlers[name] = fn
end

function M.modal_loop(check_fn)
    while check_fn() do
        if luaxt.processevent() ~= 0 then break end
        local cb_name = luaxt.pullcallback()
        if cb_name ~= '' then
            local h = handlers[cb_name] or _G[cb_name]
            if h then h() end
        end
    end
end

function M.alert(msg)
    local running = true
    _G._alert_close = function() running = false end
    local win = backend.build({'window', ':title', 'Alert', ':width', 300, ':height', 150,
        {'grid', {'label', ':label', msg, ':gridx', 0, ':gridy', 0, ':width', 280, ':height', 100},
                 {'button', ':label', 'OK', ':gridx', 0, ':gridy', 1, ':callback', 'LUA(_alert_close)'}}})
    xtmanage(win)
    M.modal_loop(function() return running end)
    xtunmanage(win)
end

function M.confirm(msg)
    local running, result = true, false
    _G._confirm_yes = function() result = true; running = false end
    _G._confirm_no = function() result = false; running = false end
    local win = backend.build({'window', ':title', 'Confirm', ':width', 300, ':height', 150,
        {'grid', {'label', ':label', msg, ':gridx', 0, ':gridy', 0, ':gridWidth', 2, ':height', 100},
                 {'button', ':label', 'Yes', ':gridx', 0, ':gridy', 1, ':callback', 'LUA(_confirm_yes)'},
                 {'button', ':label', 'No', ':gridx', 1, ':gridy', 1, ':callback', 'LUA(_confirm_no)'}}})
    xtmanage(win)
    M.modal_loop(function() return running end)
    xtunmanage(win)
    return result
end

function M.prompt(title, msg, default)
    local running, result = true, nil
    _G._prompt_ok = function() result = M.get('prompt_input', 'label'); running = false end
    _G._prompt_cancel = function() running = false end
    local win = backend.build({'window', ':title', title or 'Prompt', ':width', 400, ':height', 200,
        {'grid', {'label', ':label', msg, ':gridx', 0, ':gridy', 0, ':gridWidth', 2, ':height', 50},
                 {'edit', ':id', 'prompt_input', ':label', default or '', ':gridx', 0, ':gridy', 1, ':gridWidth', 2, ':width', 380, ':height', 50},
                 {'button', ':label', 'OK', ':gridx', 0, ':gridy', 2, ':callback', 'LUA(_prompt_ok)'},
                 {'button', ':label', 'Cancel', ':gridx', 1, ':gridy', 2, ':callback', 'LUA(_prompt_cancel)'}}})
    xtmanage(win)
    M.modal_loop(function() return running end)
    xtunmanage(win)
    return result
end

function M.create_store(cols)
    local handle = mls_create(10, 8)
    local store = { handle = handle, cols = cols }
    function store:clear() mls_clear(self.handle) end
    function store:get_iter(path)
        local idx = tonumber(path)
        if idx and idx < mls_len(self.handle) then return idx + 1 end
    end
    setmetatable(store, { __index = function(t, k) return nil end })
    return store
end

function M.create_tree_store(cols) return M.create_store(cols) end

function M.store_append(store, values)
    mls_put_string(store.handle, tostring(values[1]))
    return mls_len(store.handle)
end

function M.tree_append(store, parent, values) return M.store_append(store, values) end

_G.gui = M
return M
