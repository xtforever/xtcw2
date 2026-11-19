-- Confirmation dialogs for script prompts.

local gui = require('gui_xt')
local prompt = require('prompt')

local M = {}

-- Ask the user about a parsed confirmation. Returns the chosen letter
-- ("y", "n", "a", ...) or nil if the dialog could not be shown.
function M.ask(parsed)
    if not parsed then return nil end
    local plan = prompt.dialog_plan(parsed)
    if not plan or #plan == 0 then return nil end

    local result = gui.choice(parsed.question, {
        title = 'Confirmation',
        message_type = 4,
        ok_label = plan[1] and plan[1].label or 'OK',
        cancel_label = plan[2] and plan[2].label or 'Cancel',
        aux_label = plan[3] and plan[3].label or '',
        show_aux = plan[3] ~= nil,
    })

    if result == 1 then return plan[1] and plan[1].letter end
    if result == 0 then return plan[2] and plan[2].letter end
    if result == 2 then return plan[3] and plan[3].letter end
    return nil
end

return M
