-- Headless probe for script-embedded <launcher-begin> descriptions.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local gui = require('gui_xt')
local sql = require('sql')
local scriptmeta = require('scriptmeta')
local ui = require('ui')

sql.set_query_fn(function()
    return {
        columns = { 'custid', 'dbservid' },
        rows = {
            { custid = 'jens', dbservid = 'db1' },
            { custid = 'anna', dbservid = 'db2' },
        },
    }
end)

local fixture = os.getenv('ADMPANEL_FIXTURE')
local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_described.txt'
if not fixture then error('ADMPANEL_FIXTURE is required') end

local cfg, err = scriptmeta.load(fixture)
if not cfg then error('scriptmeta: ' .. tostring(err)) end

ui.init(cfg)
function on_field_changed(id) ui.on_field_changed(id) end
function on_execute() ui.on_execute() end
function on_abort() ui.on_abort() end
function on_script_event(ev) ui.on_script_event(ev) end
function on_quit() xtappexitflag() end
ui.run()

local preview = ''
local wait_done
local start_polls = 0
local function start()
    start_polls = start_polls + 1
    if ui.values.kk ~= 'jens' and start_polls < 50 then
        xtapptimeout(100, start)
        return
    end
    gui.set('fld_3', 'checked', true)
    on_field_changed('fld_3')
    preview = gui.get('cmd_preview', 'label') or ''
    on_execute()
    xtapptimeout(100, wait_done)
end

local polls = 0
wait_done = function()
    polls = polls + 1
    if ui.is_running() and polls < 100 then
        xtapptimeout(100, wait_done)
        return
    end

    local f = assert(io.open(out_path, 'w'))
    f:write('title=' .. tostring(gui.get('hdr', 'label')) .. '\n')
    f:write('preview=' .. tostring(preview) .. '\n')
    f:write('value=' .. tostring(ui.values.kk) .. '\n')
    for i, line in ipairs(ui.get_log()) do
        f:write('log' .. tostring(i) .. '=' .. tostring(line) .. '\n')
    end
    f:write('status=' .. tostring(gui.get('status', 'label')) .. '\n')
    f:close()
    xtappexitflag()
end

xtapptimeout(100, start)
