local gui = require('gui_xt')
local store = gui.create_store(2)
gui.store_append(store, {"A", "B"})
local tags = {
    'window', 'label', 'Wlabel', 'button', 'Wbutton', 'command', 'toggle',
    'edit', 'Wedit', 'grid', 'WpixBtn', 'separator', 'check',
    'list-view', 'splitter', 'Wsplitter', 'WspinBox', 'Wpassword', 'Wcombo', 'wlist4'
}
local ui = '(window :id "w" :title "All" :width 600 :height 400\n  (grid :id "g"\n'
for i, tag in ipairs(tags) do
    ui = ui .. '    (' .. tag .. ' :id "w_' .. tag .. '"'
    if tag == 'command' or tag == 'button' or tag == 'Wbutton' or tag == 'WpixBtn' then
        ui = ui .. ' :label "Test"'
    end
    if tag == 'label' or tag == 'Wlabel' or tag == 'Wedit' or tag == 'edit' or tag == 'Wpassword' then
        ui = ui .. ' :label "Hello"'
    end
    if tag == 'WspinBox' then ui = ui .. ' :value 5' end
    if tag == 'Wcombo' then ui = ui .. ' :label "Choose"' end
    if tag == 'list-view' then
        ui = ui .. ' :columns (200 80) :model ' .. tostring(store.handle)
    end
    if tag == 'splitter' or tag == 'Wsplitter' then ui = ui .. ' :fraction 300' end
    ui = ui .. ')\n'
end
ui = ui .. '  ))\n'
lui.run(ui)
xtapptimeout(800, function()
    local f = io.open("/home/jens/git/xtcw2/lui_test/test_results/all_widgets_result.txt", "w")
    local found = 0
    local missing = {}
    for _, tag in ipairs(tags) do
        local w = gui.get_widget("w_" .. tag)
        if w then found = found + 1 else table.insert(missing, tag) end
    end
    f:write("count=" .. found .. "/" .. #tags .. "\n")
    if #missing > 0 then f:write("missing=" .. table.concat(missing, ",") .. "\n") end
    f:close()
    xtappexitflag()
end)
