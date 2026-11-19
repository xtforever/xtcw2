local backend = require('backend_xt')
local WModel = require('wmodel')
local luaxt = luaxt or require('luaxt')

local M = {}
local handlers = {}
local model_proxies = {}

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
    if not w then return nil end
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

-- Set an MLS string-list resource (e.g. WListView rows, tableStrs) from a
-- Lua array of strings.
function M.set_string_list(id, prop, rows)
    local w = backend.get_widget(id)
    if not w or type(rows) ~= 'table' then return end
    local handle = mls_create(10, 8)
    for _, item in ipairs(rows) do
        mls_put_string(handle, tostring(item))
    end
    xtsetvalue(w, prop, handle)
end

function M.get_widget(id)
    return backend.get_widget(id)
end

-- Push a WModel proxy into a WListView/table-view. The proxy keeps the
-- source model, the active filter and the sort order, so refreshing the
-- widget does not lose the current search.
local function push_model(id, proxy)
    local w = backend.get_widget(id)
    if not w then return 0 end
    local cols = proxy:columns()
    local titles, widths, aligns, kinds = {}, {}, {}, {}
    for i, col in ipairs(cols) do
        titles[i] = col.title or ""
        widths[i] = tostring(col.width or 0)
        aligns[i] = tostring(col.align or 0)
        kinds[i] = tostring(col.kind or 0)
    end

    local keep_id = tonumber(M.get(id, 'selectedId'))

    M.set(id, 'columnCount', #cols)
    if #cols > 0 then
        M.set(id, 'columnTitles', table.concat(titles, '\t'))
        M.set(id, 'columnWidths', table.concat(widths, ','))
        M.set(id, 'columnAligns', table.concat(aligns, ','))
        M.set(id, 'columnKinds', table.concat(kinds, ','))
    end
    if proxy:sort_column() then
        -- The widget resource is 0-based; the model contract is 1-based.
        M.set(id, 'sortColumn', proxy:sort_column() - 1)
        M.set(id, 'sortAscending', proxy:sort_ascending() and true or false)
    end

    M.set_string_list(id, 'rows', proxy:rows())
    M.set_string_list(id, 'rowIds', proxy:ids())

    if keep_id and keep_id >= 0 then
        local view = proxy:view_of_id(keep_id)
        if view then M.set(id, 'selected', view - 1) end
    end
    return proxy:count()
end

-- Attach a WModel to a list/table. `opts` may carry `filter`,
-- `filter_columns`, `sort_column` and `sort_ascending`.
function M.set_model(id, model, opts)
    opts = opts or {}
    local proxy = model_proxies[id]
    if not proxy then
        proxy = WModel.proxy(model)
        model_proxies[id] = proxy
    else
        proxy:set_source(model)
    end
    if opts.filter ~= nil then proxy:set_filter(opts.filter, opts.filter_columns) end
    if opts.sort_column ~= nil then
        proxy:set_sort(opts.sort_column + 1, opts.sort_ascending)
    end
    return push_model(id, proxy)
end

-- Apply a case-insensitive substring filter over the attached model and
-- return the number of visible rows.
function M.set_model_filter(id, text, cols)
    local proxy = model_proxies[id]
    if not proxy then return 0 end
    proxy:set_filter(text, cols)
    return push_model(id, proxy)
end

-- `col` is the 0-based column index used by the widget resources.
function M.sort_model(id, col, ascending)
    local proxy = model_proxies[id]
    if not proxy then return 0 end
    proxy:set_sort(col + 1, ascending)
    return push_model(id, proxy)
end

function M.model_refresh(id)
    local proxy = model_proxies[id]
    if not proxy then return 0 end
    return push_model(id, proxy)
end

function M.get_model(id)
    return model_proxies[id]
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

-- Dialogs ---------------------------------------------------------------
-- alert/confirm/prompt build a themed WDialog instead of raw legacy windows.
local dialog_seq = 0

local function build_dialog(opts)
    dialog_seq = dialog_seq + 1
    local id = 'xtcw_dialog_' .. dialog_seq
    local function kw(name) return { keyword = name } end
    local node = {
        'WDialog', kw(':id'), id,
        kw(':title'), opts.title or 'Dialog',
        kw(':message'), opts.message or '',
        kw(':okLabel'), opts.ok_label or 'OK',
        kw(':cancelLabel'), opts.cancel_label or 'Cancel',
        kw(':showCancel'), opts.show_cancel ~= false,
        kw(':auxLabel'), opts.aux_label or '',
        kw(':showAux'), opts.show_aux == true,
    }
    local function prop(key, value)
        if value == nil then return end
        node[#node + 1] = kw(key)
        node[#node + 1] = value
    end
    prop(':messageType', opts.message_type)
    prop(':icon', opts.icon)
    prop(':details', opts.details)
    prop(':checkboxLabel', opts.checkbox_label)
    prop(':checkboxChecked', opts.checkbox_checked)
    prop(':inputKind', opts.input_kind)
    prop(':inputValue', opts.input_value)
    prop(':inputPlaceholder', opts.placeholder)
    prop(':inputItems', opts.items)
    prop(':inputMin', opts.min)
    prop(':inputMax', opts.max)
    backend.build(node)
    return id
end

-- Show a dialog, run a nested event loop and return (result, inputValue).
local function run_dialog(id)
    if not M.get_widget(id) then return nil end
    M.set(id, 'open', true)
    if M.get(id, 'open') ~= true then
        M.destroy(id)
        return nil
    end
    M.modal_loop(function() return M.get(id, 'open') == true end)
    local result = M.get(id, 'result')
    local value = M.get(id, 'inputValue')
    M.destroy(id)
    return result, value
end

function M.alert(msg, opts)
    opts = opts or {}
    local id = build_dialog{
        title = opts.title or 'Alert',
        message = msg,
        message_type = opts.message_type or 1,
        icon = opts.icon,
        details = opts.details,
        checkbox_label = opts.checkbox_label,
        checkbox_checked = opts.checkbox_checked,
        show_cancel = false,
    }
    run_dialog(id)
end

function M.confirm(msg, opts)
    opts = opts or {}
    local id = build_dialog{
        title = opts.title or 'Confirm',
        message = msg,
        message_type = opts.message_type or 4,
        icon = opts.icon,
        details = opts.details,
        checkbox_label = opts.checkbox_label,
        checkbox_checked = opts.checkbox_checked,
        ok_label = opts.ok_label,
        cancel_label = opts.cancel_label,
    }
    local result = run_dialog(id)
    return result == 1
end

-- Three-button dialog: returns 1 = OK, 0 = Cancel, 2 = Aux.
function M.choice(msg, opts)
    opts = opts or {}
    local id = build_dialog{
        title = opts.title or 'Confirm',
        message = msg,
        message_type = opts.message_type or 4,
        icon = opts.icon,
        details = opts.details,
        ok_label = opts.ok_label,
        cancel_label = opts.cancel_label,
        aux_label = opts.aux_label,
        show_aux = opts.show_aux,
    }
    return run_dialog(id)
end

function M.prompt(title, msg, default, opts)
    opts = opts or {}
    local id = build_dialog{
        title = title or 'Prompt',
        message = msg,
        message_type = opts.message_type,
        input_kind = opts.input_kind or 1,
        input_value = default or '',
        placeholder = opts.placeholder,
        items = opts.items,
        min = opts.min,
        max = opts.max,
        ok_label = opts.ok_label,
        cancel_label = opts.cancel_label,
    }
    local result, value = run_dialog(id)
    if result == 1 then return value end
    return nil
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

function M.load_css(path)
    print("Warning: stylesheets not implemented, ignoring: " .. tostring(path))
    return false
end

_G.gui = M
return M
