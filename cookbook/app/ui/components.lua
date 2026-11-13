-- cookbook/app/ui/components.lua
-- Reusable S-expression fragment builders for the cookbook screens.
--
-- Every builder returns a string that is concatenated into the single UI source
-- parsed by lui.run().  The builders use the modern themed widgets from the
-- style guide (Wbutton/Wedit/WspinBox/WToggleSwitch/WListView), not Xaw.

local M = {}

local function fmt(s, ...)
    return string.format(s, ...)
end

-- A plain text label spanning `wx` columns.
function M.label(id, text, gx, gy, wx)
    wx = wx or 1
    return fmt('(label :id "%s" :label "%s" :gridx %d :gridy %d :gridWidth %d'
        .. ' :weightx 1 :weighty 0 :fill 3)', id, text, gx, gy, wx)
end

-- A single-line editor.
function M.edit(id, value, gx, gy, wx)
    wx = wx or 1
    return fmt('(edit :id "%s" :label "%s" :gridx %d :gridy %d :gridWidth %d'
        .. ' :weightx 1 :weighty 0 :fill 3)', id, value or "", gx, gy, wx)
end

-- A push button wired to a global Lua callback name.
function M.button(id, text, callback, gx, gy, wx)
    wx = wx or 1
    return fmt('(Wbutton :id "%s" :label "%s" :gridx %d :gridy %d :gridWidth %d'
        .. ' :weightx 1 :weighty 0 :fill 3 :callback "LUA(%s)")',
        id, text, gx, gy, wx, callback)
end

-- A numeric spin box.
function M.spin(id, value, min, max, gx, gy, wx)
    wx = wx or 1
    return fmt('(WspinBox :id "%s" :gridx %d :gridy %d :gridWidth %d'
        .. ' :weightx 1 :weighty 0 :fill 3 :value %d :min %d :max %d)',
        id, gx, gy, wx, value, min, max)
end

-- A themed boolean switch with its label.  Returns a nested Gridbox so the
-- label and switch stay together; `id` names the WToggleSwitch itself.
function M.toggle(id, text, gx, gy, wx)
    wx = wx or 1
    return fmt('(grid :id "%s_box" :gridx %d :gridy %d :gridWidth %d'
        .. ' :weightx 1 :weighty 0 :fill 3 :borderWidth 0\n'
        .. '  (label :label "%s" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)\n'
        .. '  (WToggleSwitch :id "%s" :gridx 1 :gridy 0 :weightx 0 :weighty 0'
        .. ' :fill 3 :checked FALSE))',
        id, gx, gy, wx, text, id)
end

return M
