-- tests/test_dialogs.lua
-- Exercises gui.alert / gui.confirm / gui.prompt through the themed WDialog.
-- Run by tests/test_dialogs.sh from the commander/ directory.
package.path = '../lui/?.lua;' .. package.path
local gui = require('gui_xt')

local function answer(id, action)
    xtapptimeout(120, function()
        local w = gui.get_widget(id)
        if w then xtaction(w, action) end
    end)
end

local out = {}

answer('xtcw_dialog_1', 'ok')
gui.alert('hello')
out.alert = 'ok'

answer('xtcw_dialog_2', 'cancel')
out.confirm_cancel = gui.confirm('sure?') and 'true' or 'false'

answer('xtcw_dialog_3', 'ok')
out.prompt = gui.prompt('Name', 'Enter a name', 'Bob') or '(nil)'

answer('xtcw_dialog_4', 'ok')
out.confirm_ok = gui.confirm('really?') and 'true' or 'false'

local f = io.open(os.getenv('DIALOG_OUT'), 'w')
if f then
    for k, v in pairs(out) do f:write(k .. '=' .. tostring(v) .. '\n') end
    f:close()
end
xtappexitflag()
