-- Headless probe for confirmation dialogs + script stdin.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local answer = os.getenv('ADMPANEL_TEST_ANSWER') or 'y'
local seen_question = ''
local seen_options = ''

-- Stub the real WDialog before ui.lua requires it.
package.loaded['dialog'] = {
    ask = function(parsed)
        seen_question = parsed and parsed.question or ''
        seen_options = parsed and table.concat(parsed.options, ',') or ''
        return answer
    end,
}

local config = require('config')
local ui = require('ui')

local fixture = os.getenv('ADMPANEL_FIXTURE')
local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_prompt.txt'
if not fixture then error('ADMPANEL_FIXTURE is required') end

local cfg, err = config.normalize({
    title = 'prompt test',
    script = fixture,
    fields = {},
})
if not cfg then error('config: ' .. tostring(err)) end

ui.init(cfg)

function on_field_changed(id) ui.on_field_changed(id) end
function on_execute() ui.on_execute() end
function on_abort() ui.on_abort() end
function on_script_event(ev) ui.on_script_event(ev) end
function on_quit() xtappexitflag() end

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
    f:write('answer=' .. tostring(answer) .. '\n')
    f:write('question=' .. tostring(seen_question) .. '\n')
    f:write('options=' .. tostring(seen_options) .. '\n')
    f:write('running=' .. tostring(ui.is_running()) .. '\n')
    for i, line in ipairs(log) do
        f:write('log' .. tostring(i) .. '=' .. tostring(line) .. '\n')
    end
    f:write('status=' .. tostring(require('gui_xt').get('status', 'label')) .. '\n')
    f:close()
    xtappexitflag()
end

xtapptimeout(100, poll)
