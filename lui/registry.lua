local M = {}
local registry = {}

function M.register(name, def)
    registry[name] = def
end

function M.get(name)
    return registry[name]
end

-- Auto-generated class-name mappings (written by wbuild_widgets/register-widgets.sh).
-- Every widget built from a .widget file gets a tag equal to its class name,
-- e.g. (WlistMulti ...), (Frame ...), (VBox ...).
local ok, generated = pcall(require, 'registry_generated')
if ok and type(generated) == 'table' then
    for _, class in ipairs(generated) do
        M.register(class, { class = class })
    end
end

-- Friendly aliases for common widgets. These keep simple GUIs terse while
-- the generated class-name tags above allow access to the full widget set.
M.register('window', { class = 'topLevelShellWidgetClass' })
M.register('label', { class = 'Wlabel' })
M.register('button', { class = 'Wbutton' })
M.register('command', { class = 'command' })
M.register('toggle', { class = 'Toggle' })
M.register('edit', { class = 'Wedit' })
M.register('grid', { class = 'Gridbox' })
M.register('vertical', { class = 'VBox' })
M.register('horizontal', { class = 'HBox' })
M.register('vbox', { class = 'VBox' })
M.register('hbox', { class = 'HBox' })
M.register('image', { class = 'IconSVG' })
M.register('icon', { class = 'IconSVG' })
M.register('separator', { class = 'WSeparator' })
M.register('check', { class = 'Wradio' })
M.register('checkbox', { class = 'Wcheckbox' })
M.register('radio', { class = 'Wradio' })
M.register('scrolled', { class = 'ScrolledCanvas' })
M.register('canvas', { class = 'Canvas' })
M.register('frame', { class = 'Frame' })
M.register('gauge', { class = 'Gauge' })
M.register('hslider', { class = 'HSlider' })
M.register('vslider', { class = 'VSlider' })
M.register('list-view', { class = 'WlsMulti' })
M.register('list4', { class = 'Wlist4' })
M.register('wlist4', { class = 'Wlist4' })
M.register('list', { class = 'Wlist' })
M.register('wlist', { class = 'Wlist' })
M.register('wlistmulti', { class = 'WlistMulti' })
M.register('wls', { class = 'Wls' })
M.register('wlsmulti', { class = 'WlsMulti' })
M.register('splitter', { class = 'Wsplitter' })
M.register('combo', { class = 'Wcombo' })
M.register('spinbox', { class = 'WspinBox' })
M.register('password', { class = 'Wpassword' })
M.register('retex', { class = 'Wretex' })
M.register('text', { class = 'Wtext' })
M.register('paned', { class = 'WPaned' })
M.register('menu', { class = 'Wmenu' })
M.register('menupopup', { class = 'WmenuPopup' })
M.register('file-selector', { class = 'WfileSelector' })
M.register('messagebox', { class = 'MessageBox' })
M.register('option', { class = 'Woption' })
M.register('viewvar', { class = 'WviewVar' })
M.register('editmv', { class = 'WeditMV' })
M.register('pixbtn', { class = 'WpixBtn' })
M.register('board', { class = 'Board' })
M.register('dartboard', { class = 'Dartboard' })
M.register('selectreq', { class = 'SelectReq' })
M.register('karoed', { class = 'KaroEd' })
M.register('multiline', { class = 'KaroEd' })

return M
