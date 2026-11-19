-- Headless probe for the real WDialog confirmation path.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local gui = require('gui_xt')
local prompt = require('prompt')
local dialog = require('dialog')

local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_dialog.txt'
local button = os.getenv('ADMPANEL_TEST_BUTTON') or 'ok'
local spec = os.getenv('ADMPANEL_TEST_PROMPT') or 'Continue (Y/n)?'

local function click_dialog()
    local w = gui.get_widget('xtcw_dialog_1')
    if w then xtaction(w, button) end
end

xtapptimeout(300, click_dialog)
local letter = dialog.ask(prompt.parse_confirmation(spec))

local f = assert(io.open(out_path, 'w'))
f:write('button=' .. tostring(button) .. '\n')
f:write('spec=' .. tostring(spec) .. '\n')
f:write('letter=' .. tostring(letter) .. '\n')
f:close()
xtappexitflag()
