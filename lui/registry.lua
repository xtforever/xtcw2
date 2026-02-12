local M = {}
local registry = {}

function M.register(name, def)
    registry[name] = def
end

function M.get(name)
    return registry[name]
end

-- Initial registrations
M.register('window', { class = 'applicationShellWidgetClass' })
M.register('label', { class = 'Wlabel' })
M.register('button', { class = 'Wbutton' })
M.register('edit', { class = 'Wedit' })
M.register('grid', { class = 'Gridbox' })
M.register('vertical', { class = 'Gridbox' })
M.register('horizontal', { class = 'Gridbox' })

return M
