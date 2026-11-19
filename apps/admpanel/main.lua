-- AdmPanel launcher entry point.
--
-- Run from the repository root:
--   ADMPANEL_CONFIG=apps/admpanel/examples/freigabe.lua \
--     ./run_lui.sh apps/admpanel/main.lua
--
-- The runner changes into commander/, so also accept the relative package path.

package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local config = require('config')
local scriptmeta = require('scriptmeta')
local ui = require('ui')

local path = os.getenv('ADMPANEL_CONFIG') or '../apps/admpanel/examples/freigabe.lua'
local script_path = os.getenv('ADMPANEL_SCRIPT')
local cfg, err
if script_path and script_path ~= '' then
    cfg, err = scriptmeta.load(script_path)
else
    cfg, err = config.load(path)
end
if not cfg then
    error('AdmPanel config error: ' .. tostring(err))
end

ui.init(cfg)

-- Callbacks are looked up as Lua globals by the host's LUA(...) dispatch.
function on_field_changed(id)
    ui.on_field_changed(id)
end

function on_execute()
    ui.on_execute()
end

function on_abort()
    ui.on_abort()
end

function on_script_event(ev)
    ui.on_script_event(ev)
end

function on_quit()
    ui.on_quit()
end

ui.run()
