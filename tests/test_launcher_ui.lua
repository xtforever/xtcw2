-- Headless UI probe for apps/admpanel.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local gui = require('gui_xt')
local config = require('config')
local ui = require('ui')

local path = os.getenv('ADMPANEL_CONFIG')
    or '../apps/admpanel/examples/freigabe.lua'
local out_path = os.getenv('ADMPANEL_TEST_OUT')
    or '/tmp/admpanel_ui.txt'

local cfg, err = config.load(path)
if not cfg then error('config: ' .. tostring(err)) end

ui.init(cfg)

function on_field_changed(id)
    ui.on_field_changed(id)
end

function on_execute()
    ui.on_execute()
end

function on_quit()
    xtappexitflag()
end

ui.run()

xtapptimeout(500, function()
    local f = assert(io.open(out_path, 'w'))

    f:write('title=' .. tostring(gui.get('hdr', 'label')) .. '\n')
    f:write('field_count=' .. tostring(#cfg.fields) .. '\n')
    f:write('fld1_exists=' .. tostring(gui.get_widget('fld_1') ~= nil) .. '\n')
    f:write('fld4_exists=' .. tostring(gui.get_widget('fld_4') ~= nil) .. '\n')
    f:write('preview_initial=' .. tostring(gui.get('cmd_preview', 'label')) .. '\n')

    gui.set('fld_4', 'label', 'adanetcon')
    on_field_changed('fld_4')
    f:write('preview_after=' .. tostring(gui.get('cmd_preview', 'label')) .. '\n')

    f:close()
    xtappexitflag()
end)
