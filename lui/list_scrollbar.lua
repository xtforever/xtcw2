local macros = require('macros')
local gui = require('gui_xt')

-- Helper functions for synchronization
function _list_scrollbar_vscroll_cb(id, data)
    -- data is "top_y,list_height,total_height"
    local top_y, list_height, total_height = data:match("([^,]+),([^,]+),([^,]+)")
    if not top_y then return end
    
    top_y = tonumber(top_y)
    list_height = tonumber(list_height)
    total_height = tonumber(total_height)
    
    local slider_id = id .. "_slider"
    
    if total_height > 0 then
        local frac = math.floor((list_height / total_height) * 1000000)
        local pos = math.floor((top_y / total_height) * 1000000)
        
        gui.set(slider_id, "frac", frac)
        gui.set(slider_id, "pos", pos)
    end
end

function _list_scrollbar_slider_cb(id, pos)
    local list_id = id
    -- We need to fetch total_height from the list widget
    -- Wlist4 properties are line_max and line_height
    local line_max = tonumber(gui.get(list_id, "line_max"))
    local line_height = tonumber(gui.get(list_id, "line_height"))
    
    if not line_max or not line_height then
        -- print("DEBUG: slider_cb: metrics not ready yet", line_max, line_height)
        return 
    end

    local total_height = line_max * line_height
    
    if total_height > 0 then
        local top_y = math.floor((tonumber(pos) / 1000000) * total_height)
        gui.set(list_id, "top_y", top_y)
    end
end

-- We can use LUA strings for now as they are easier to register globally
-- for the C-host to find them via _G[cb_name]

macros.register('list-scrollbar', {'id', 'width', 'height', 'slwidth'}, {
    'grid', {keyword=':id'}, { lua = "return tostring(id) .. '_grid'" },
    {
        'list4', {keyword=':id'}, { lua = "return tostring(id)" },
        {keyword=':gridx'}, 0, {keyword=':gridy'}, 0,
        {keyword=':weightx'}, 1, {keyword=':weighty'}, 1,
        {keyword=':width'}, { lua = "return tonumber(width)" },
        {keyword=':height'}, { lua = "return tonumber(height)" },
        {keyword=':vscroll'}, { lua = "return 'LUA(_list_scrollbar_vscroll_cb, ' .. id .. ')'" }
    },
    {
        'vslider', {keyword=':id'}, { lua = "return id .. '_slider'" },
        {keyword=':gridx'}, 1, {keyword=':gridy'}, 0,
        {keyword=':weightx'}, 0, {keyword=':weighty'}, 1,
        {keyword=':width'}, { lua = "return tonumber(slwidth)" },
        {keyword=':height'}, { lua = "return tonumber(height)" },
        {keyword=':callback'}, { lua = "return 'LUA(_list_scrollbar_slider_cb, ' .. id .. ')'" }
    }
})
