local M = {}
local registry = {}

function M.register(name, def)
    registry[name] = def
end

function M.get(name)
    return registry[name]
end

-- Initial registrations
M.register('window', { class = 'topLevelShellWidgetClass' })
M.register('label', { class = 'Wlabel' })
M.register('Wlabel', { class = 'Wlabel' })
M.register('button', { class = 'Wbutton' })
M.register('Wbutton', { class = 'Wbutton' })
M.register('command', { class = 'command' })
M.register('toggle', { class = 'Toggle' })
M.register('edit', { class = 'Wedit' })
M.register('grid', { class = 'Gridbox' })
M.register('vertical', { class = 'Gridbox' })
M.register('horizontal', { class = 'Gridbox' })
M.register('image', { class = 'IconSVG' })
M.register('WpixBtn', { class = 'WpixBtn' })

-- New mappings for File Manager
M.register('separator', { class = 'WSeparator' })
M.register('check', { class = 'Wradio' })
M.register('scrolled', { class = 'ScrolledCanvas' })
M.register('list-view', { class = 'WlsMulti' })
M.register('splitter', { class = 'Wsplitter' })
M.register('vslider', { class = 'VSlider' })
M.register('list4', { class = 'Wlist4' })
M.register('wlist4', { class = 'Wlist4' })
M.register('Wlist4', { class = 'Wlist4' })
M.register('Wsplitter', { class = 'Wsplitter' })
M.register('Wcombo', { class = 'Wcombo' })
M.register('WspinBox', { class = 'WspinBox' })
M.register('Wpassword', { class = 'Wpassword' })
M.register('Wedit', { class = 'Wedit' })
M.register('retex', { class = 'Wretex' })
M.register('Wretex', { class = 'Wretex' })

return M
