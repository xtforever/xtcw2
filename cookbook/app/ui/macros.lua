-- cookbook/app/ui/macros.lua
-- Shared LUI macro prelude and small Lua string helpers.
--
-- The LUI macro prelude is prepended to the generated UI source; LUI macros are
-- expanded by lui/macros.lua before the widget tree is built. Keep the macros
-- minimal and proven — see demos/demo_wbutton.lua for the `grid_opts` pattern.

local M = {}

-- Prepended to the S-expression UI source in app/main.lua.
M.PRELUDE = [[
(defmacro cell (gx gy wx wy)
  :gridx gx :gridy gy :weightx wx :weighty wy)
]]

-- Lua-side helper that produces the same property string (for fragments built
-- in Lua). Provided so screen modules can choose either style.
function M.cell(gx, gy, wx, wy)
    return string.format(
        ":gridx %d :gridy %d :weightx %d :weighty %d",
        gx, gy, wx, wy)
end

return M
