-- Headless probe for the pty script runner + embedded log.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local config = require('config')
local ui = require('ui')

local fixture = os.getenv('ADMPANEL_FIXTURE')
local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_script.txt'
if not fixture then error('ADMPANEL_FIXTURE is required') end

local cfg, err = config.normalize({
    title = 'script test',
    script = fixture .. ' --name $name',
    fields = {
        { type = 'input', variable = 'name', default = 'jens' },
    },
})
if not cfg then error('config: ' .. tostring(err)) end

ui.init(cfg)

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
    xtappexitflag()
end

ui.run()
ui.on_execute()

local polls = 0
local function poll()
    polls = polls + 1
    if ui.is_running() and polls < 100 then
        xtapptimeout(100, poll)
        return
    end

    local f = assert(io.open(out_path, 'w'))
    local log = ui.get_log()
    f:write('polls=' .. tostring(polls) .. '\n')
    f:write('running=' .. tostring(ui.is_running()) .. '\n')
    f:write('log_count=' .. tostring(#log) .. '\n')
    for i, line in ipairs(log) do
        f:write('log' .. tostring(i) .. '=' .. tostring(line) .. '\n')
    end
    f:write('status=' .. tostring(require('gui_xt').get('status', 'label')) .. '\n')
    f:close()
    xtappexitflag()
end

xtapptimeout(100, poll)
