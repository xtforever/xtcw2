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

-- New mappings for File Manager
M.register('separator', { class = 'Wlabel' }) -- TODO: Implement proper separator
M.register('check', { class = 'Wradio' })
M.register('scrolled', { class = 'ScrolledCanvas' })
M.register('list-view', { class = 'Wlist4' })

return M
