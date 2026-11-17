-- cookbook/app/screens/tree_view.lua
-- Screen 6: full-height WTreeView testing ground.

local M = {}

M.id = "screen_tree"
M.title = "Tree"

function M.fragment()
    return table.concat({
        '(grid :id "screen_tree" :tab-title "Tree" :defaultDistance 8',
        '  (label :id "tree_hdr" :label "WTreeView" :gridx 0 :gridy 0 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (label :id "tree_hint" :label "Click the +/- box or use Left/Right to expand and collapse. Up/Down moves the selection." :gridx 0 :gridy 1 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :useSmallFont TRUE)',
        '  (WTreeView :id "tree_view" :gridx 0 :gridy 2 :gridWidth 4 :weightx 1 :weighty 1 :fill 3 :itemHeight 26 :indent 18)',
        ')',
    }, "\n")
end

return M
