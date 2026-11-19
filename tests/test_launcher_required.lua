-- Headless probe for mandatory-field validation.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local gui = require('gui_xt')
local config = require('config')
local ui = require('ui')

local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_required.txt'
local cfg, err = config.normalize({
    title = 'required test',
    script = 'echo $name',
    fields = {
        { type = 'input', variable = 'name', default = '', mandatory = true },
    },
})
if not cfg then error('config: ' .. tostring(err)) end

ui.init(cfg)
function on_field_changed(id) ui.on_field_changed(id) end
function on_execute() ui.on_execute() end
function on_abort() ui.on_abort() end
function on_script_event(ev) ui.on_script_event(ev) end
function on_quit() xtappexitflag() end
ui.run()

local preview = gui.get('cmd_preview', 'label') or ''
ui.on_execute()

xtapptimeout(300, function()
    local f = assert(io.open(out_path, 'w'))
    f:write('preview=' .. tostring(preview) .. '\n')
    f:write('running=' .. tostring(ui.is_running()) .. '\n')
    f:write('status=' .. tostring(gui.get('status', 'label')) .. '\n')
    for i, line in ipairs(ui.get_log()) do
        f:write('log' .. tostring(i) .. '=' .. tostring(line) .. '\n')
    end
    f:close()
    xtappexitflag()
end)
