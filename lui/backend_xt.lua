local registry = require('registry')
local M = {}

local widgets_by_id = {}

function M.build(node, parent_path)
    if type(node) ~= 'table' then return end
    
    local tag = node[1]
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
    local class = def.class
    
    -- Flatten properties for xtcreate
    local xt_props = {}
    for k, v in pairs(properties) do
        if k ~= ':id' then
            local clean_k = k:sub(2) -- remove ':'
            if clean_k == 'managed' then
                clean_k = 'wcManaged'
            elseif clean_k == 'on-click' then
                clean_k = 'callback'
            elseif clean_k == 'align' then
                clean_k = 'alignment'
            elseif clean_k == 'model' then
                clean_k = nil -- Handle manually
            elseif clean_k == 'columns' then
                clean_k = nil -- Handle manually
            elseif clean_k == 'on-row-activated' then
                clean_k = 'notify'
            end
            table.insert(xt_props, clean_k)
            table.insert(xt_props, tostring(v))
        end
    end

    local parent = parent_path or ''
    local w = xtcreate(id, class, parent, unpack(xt_props))
    
    if properties[':id'] then
        widgets_by_id[id] = w
    end

    local current_path = (parent == '' or parent == '.') and id or (parent .. '.' .. id)

    for _, child in ipairs(children_nodes) do
        M.build(child, current_path)
    end
    
    return w
end

function M.get_widget(id)
    return widgets_by_id[id]
end

return M
