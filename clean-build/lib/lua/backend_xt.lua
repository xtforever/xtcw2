local registry = require('registry')
local parser = require('parser')
local M = {}

local widgets_by_id = {}

function M.set_handler_registrar(fn)
    M.handler_registrar = fn
end

function M.process_property(clean_k, v)
    if v == nil or v == parser.NIL or (type(v) == 'table' and v.type == 'nil') then return nil, nil end

    if clean_k == 'managed' then
        clean_k = 'wcManaged'
    elseif clean_k == 'on-click' then
        clean_k = 'callback'
    elseif clean_k == 'align' then
        clean_k = 'alignment'
    elseif clean_k == 'model' then
        clean_k = 'tableStrs'
        if type(v) == 'table' then
            if v.handle then
                v = v.handle
            else
                local handle = mls_create(10, 8)
                for _, item in ipairs(v) do
                    mls_put_string(handle, tostring(item))
                end
                v = handle
            end
        end
    elseif clean_k == 'columns' then
        clean_k = 'columnWidths'
        if type(v) == 'table' then
            v = type(v) == 'table' and table.concat(v, ',') or v
        end
    elseif clean_k == 'on-row-activated' then
        clean_k = 'notify'
    elseif clean_k == 'on-toggle' then
        clean_k = 'callback'
    elseif clean_k == 'on-change' then
        clean_k = 'callback'
    elseif clean_k == 'orientation' then
        clean_k = 'vertical'
        v = (v == 'vertical') and '1' or '0'
    elseif clean_k == 'fraction' or clean_k == 'frac' then
        clean_k = 'frac'
        v = tonumber(v)
    elseif clean_k == 'pos' then
        v = tonumber(v)
    elseif clean_k == 'color' then
        clean_k = 'foreground'
    elseif clean_k == 'bg' then
        clean_k = 'background'
    elseif clean_k == 'weight-x' then
        clean_k = 'weightx'
    elseif clean_k == 'weight-y' then
        clean_k = 'weighty'
    elseif clean_k == 'src' then
        clean_k = 'filename'
    elseif clean_k == 'rasterize-width' then
        clean_k = 'forced_width'
    elseif clean_k == 'rasterize-height' then
        clean_k = 'forced_height'
    end

    if type(v) == 'function' then
        if M.handler_registrar then
            local cb_id = 'lua_cb_' .. tostring(math.random(100000, 999999))
            M.handler_registrar(cb_id, v)
            v = 'LUA(' .. cb_id .. ')'
        else
            print('Warning: No handler registrar set, cannot handle function property:', clean_k)
            return nil, nil
        end
    elseif (clean_k == 'callback' or clean_k == 'notify' or clean_k == 'vscroll' or clean_k == 'pos') and type(v) == 'string' then
        if not v:match('^LUA%(') then
            v = 'LUA(' .. v .. ')'
        end
    end

    return clean_k, v
end

function M.build(node, parent_path)
    if type(node) ~= 'table' then return end
    
    local tag = node[1]
    print('Building widget tag:', tag)
    local def = registry.get(tag)
    if not def then
        print('Warning: Unknown widget tag:', tag)
        def = { class = tag }
    end

    local properties = {}
    local children_nodes = {}
    local i = 2
    while i <= #node do
        local item = node[i]
        if type(item) == 'table' and item.keyword then
            properties[item.keyword] = node[i+1]
            i = i + 2
        else
            if type(item) == 'table' then
                table.insert(children_nodes, item)
            end
            i = i + 1
        end
    end

    local id = properties[':id'] or (tag .. '_' .. tostring(math.random(1000, 9999)))
    if type(id) == 'table' and id.keyword then id = id.keyword:sub(2) end
    local class = def.class
    
    -- Flatten properties for xtcreate
    local xt_props = {}
    for k, v in pairs(properties) do
        if k ~= ':id' then
            local clean_k, processed_v = M.process_property(k:sub(2), v)
            if clean_k and processed_v ~= nil then
                table.insert(xt_props, clean_k)
                table.insert(xt_props, processed_v)
            end
        end
    end

    local parent = (parent_path == nil or parent_path == '' or parent_path == '.') and 'luarunner' or parent_path
    local w = xtcreate(id, class, parent, table.unpack(xt_props))
    
    if properties[':id'] then
        widgets_by_id[id] = w
    end

    local current_path = id
    if parent ~= 'luarunner' then
        current_path = parent .. '.' .. id
    end

    for _, child in ipairs(children_nodes) do
        M.build(child, current_path)
    end
    
    return w
end

function M.get_widget(id)
    return widgets_by_id[id]
end

return M
