local backend = require('backend_xt')
local luaxt = luaxt or require('luaxt')

local M = {}
local handlers = {}

backend.set_handler_registrar(function(name, fn)
    handlers[name] = fn
end)

function M.build(ast)
    if not ast then return end
    for _, node in ipairs(ast) do
        backend.build(node)
    end
end

function M.set(id, prop, val)
    local w = backend.get_widget(id)
    if w then
        local clean_k, processed_v = backend.process_property(prop, val)
        if clean_k and processed_v ~= nil then
            xtsetvalue(w, clean_k, processed_v)
        end
    end
end

function M.get(id, prop)
    local w = backend.get_widget(id)
    if w then
        local mapped_prop = prop
        if prop == 'spinValue' then mapped_prop = 'value'
        elseif prop == 'spinMin' then mapped_prop = 'min'
        elseif prop == 'spinMax' then mapped_prop = 'max'
        elseif prop == 'spinStep' then mapped_prop = 'step'
        end
        local result = xtgetvalue(w, mapped_prop)
        if result == nil and (prop == 'value' or prop == 'text') then
            result = xtgetvalue(w, 'label')
        end
        return result
    end
end

function M.get_widget(id)
    return backend.get_widget(id)
end

function M.geometry(id)
    local w = backend.get_widget(id)
    if w then
        return xtgeometry(w)
    end
end

function M.xerrors(reset)
    return xterror_count(reset and true or false)
end

function M.manage(id)
    local w = backend.get_widget(id)
    if w then xtmanage(w) end
end

function M.unmanage(id)
    local w = backend.get_widget(id)
    if w then xtunmanage(w) end
end

function M.destroy(id)
    local w = backend.get_widget(id)
    if w then xtdestroy(w) end
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
    local processevent = luaxt.luaxt_processevent or luaxt.processevent
    local pullcallback = luaxt.luaxt_pullcallback or luaxt.pullcallback
    local pulldata = luaxt.luaxt_pulldata or luaxt.pulldata
    while processevent() == 0 do
        local cb_str = pullcallback()
        if cb_str ~= '' then
            local data = pulldata()
            _G.class_data = data -- compatibility
            
            -- cb_str might be "func_name, arg"
            local cb_name, cb_arg = cb_str:match("^%s*([^,%s]+)%s*,%s*(.-)%s*$")
            if not cb_name then 
                cb_name = cb_str 
                cb_arg = nil
            end

            local h = handlers[cb_name] or _G[cb_name]
            if h then 
                if cb_arg then
                    h(cb_arg, data)
                else
                    h(data)
                end
            end
        end
    end
end

function M.register_handler(name, fn)
    handlers[name] = fn
end

function M.modal_loop(check_fn)
    local processevent = luaxt.luaxt_processevent or luaxt.processevent
    local pullcallback = luaxt.luaxt_pullcallback or luaxt.pullcallback
    local pulldata = luaxt.luaxt_pulldata or luaxt.pulldata
    while check_fn() do
        if processevent() ~= 0 then break end
        local cb_str = pullcallback()
        if cb_str ~= '' then
            local data = pulldata()
            _G.class_data = data -- compatibility
            
            -- cb_str might be "func_name, arg"
            local cb_name, cb_arg = cb_str:match("^%s*([^,%s]+)%s*,%s*(.-)%s*$")
            if not cb_name then 
                cb_name = cb_str 
                cb_arg = nil
            end

            local h = handlers[cb_name] or _G[cb_name]
            if h then 
                if cb_arg then
                    h(cb_arg, data)
                else
                    h(data)
                end
            end
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
    local row_str = table.concat(values, "\t")
    mls_put_string(store.handle, row_str)
    return mls_len(store.handle)
end

function M.store_get(store, index)
    local s = mls_get_string(store.handle, index)
    if not s then return nil end
    local row = {}
    for cell in string.gmatch(s, "[^\t]+") do
        table.insert(row, cell)
    end
    return row
end

function M.tree_append(store, parent, values) return M.store_append(store, values) end

local function read_all(path)
    local f = io.open(path, "r")
    if not f then return nil end
    local content = f:read("*a")
    f:close()
    return content
end

function M.run(source)
    local ast = parser.parse(source)
    if not ast then return error("Failed to parse LUI source") end
    local expanded_ast = macros.expand(ast)
    for _, node in ipairs(expanded_ast) do
        backend.build(node)
    end
end

function M.load(path)
    local source = read_all(path)
    if not source then return error("Could not open file: " .. tostring(path)) end
    return M.run(source)
end

_G.gui = M
return M
