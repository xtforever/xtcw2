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

-- Store API Implementation
local stores = {}

function M.create_store(cols)
    local store = { type = 'list', cols = cols, data = {} }
    function store:clear()
        self.data = {}
    end
    function store:get_iter(path)
        -- path in GTK is string "0" or "0:1". Here we simplify to index.
        local idx = tonumber(path)
        if self.data[idx+1] then return idx+1 end
        return nil
    end
    -- Metatable to allow indexing like store[iter][col]
    setmetatable(store, {
        __index = function(t, k)
            if type(k) == 'number' then return t.data[k] end
            return nil
        end
    })
    return store
end

function M.create_tree_store(cols)
    local store = { type = 'tree', cols = cols, data = {} }
    function store:get_iter(path)
        -- Simplified: path is just index for now
        local idx = tonumber(path)
        if self.data[idx+1] then return idx+1 end
        return nil
    end
    setmetatable(store, {
        __index = function(t, k)
            if type(k) == 'number' then return t.data[k] end
            return nil
        end
    })
    return store
end

function M.store_append(store, values)
    table.insert(store.data, values)
    -- TODO: Notify widget if attached
    return #store.data
end

function M.tree_append(store, parent, values)
    -- Flat list for now
    table.insert(store.data, values)
    return #store.data
end

function M.modal_loop(check_fn)
    while check_fn() do
        if luaxt.processevent() ~= 0 then break end
        local cb_name = luaxt.pullcallback()
        if cb_name ~= '' then
            local h = handlers[cb_name]
            if h then h() elseif _G[cb_name] then _G[cb_name]() end
        end
    end
end

function M.alert(msg)
    local running = true
    local function close() running = false end
    _G._alert_close = close
    
    local win = backend.build({
        'window', ':title', 'Alert', ':width', 300, ':height', 150,
        {'grid', 
            {'label', ':label', msg, ':gridx', 0, ':gridy', 0, ':width', 280, ':height', 100},
            {'button', ':label', 'OK', ':gridx', 0, ':gridy', 1, ':callback', 'LUA(_alert_close)'}
        }
    })
    
    -- Center window? Xt doesn't make it easy without mapping first.
    xtmanage(win)
    M.modal_loop(function() return running end)
    xtunmanage(win)
    -- destroy? xtdestroy(win)
end

function M.confirm(msg)
    local running = true
    local result = false
    local function yes() result = true; running = false end
    local function no() result = false; running = false end
    _G._confirm_yes = yes
    _G._confirm_no = no
    
    local win = backend.build({
        'window', ':title', 'Confirm', ':width', 300, ':height', 150,
        {'grid',
            {'label', ':label', msg, ':gridx', 0, ':gridy', 0, ':gridWidth', 2, ':height', 100},
            {'button', ':label', 'Yes', ':gridx', 0, ':gridy', 1, ':callback', 'LUA(_confirm_yes)'},
            {'button', ':label', 'No', ':gridx', 1, ':gridy', 1, ':callback', 'LUA(_confirm_no)'}
        }
    })
    
    xtmanage(win)
    M.modal_loop(function() return running end)
    xtunmanage(win)
    return result
end

function M.prompt(title, msg, default)
    local running = true
    local result = nil
    local function ok() 
        result = gui.get('prompt_input', 'label') -- Wedit uses label for value
        running = false 
    end
    local function cancel() running = false end
    _G._prompt_ok = ok
    _G._prompt_cancel = cancel
    
    local win = backend.build({
        'window', ':title', title or 'Prompt', ':width', 400, ':height', 200,
        {'grid',
            {'label', ':label', msg, ':gridx', 0, ':gridy', 0, ':gridWidth', 2, ':height', 50},
            {'edit', ':id', 'prompt_input', ':label', default or '', ':gridx', 0, ':gridy', 1, ':gridWidth', 2, ':width', 380, ':height', 50},
            {'button', ':label', 'OK', ':gridx', 0, ':gridy', 2, ':callback', 'LUA(_prompt_ok)'},
            {'button', ':label', 'Cancel', ':gridx', 1, ':gridy', 2, ':callback', 'LUA(_prompt_cancel)'}
        }
    })
    
    xtmanage(win)
    M.modal_loop(function() return running end)
    xtunmanage(win)
    return result
end
