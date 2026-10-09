-- cookbook/app/screens/shopping_list.lua
-- Screen 4: aggregated shopping list with check/uncheck.

local M = {}

M.id = "screen_shopping"
M.title = "Shopping"

function M.fragment()
    return table.concat({
        '(grid :id "screen_shopping" :gridx 0 :gridy 3 :gridWidth 5 :weightx 1 :weighty 1 :fill 3',
        '  (label :id "shop_title" :label "Shopping list" :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 15)',
        '  (Wlabel :id "shop_list" :label "(shopping list)" :gridx 0 :gridy 1 :gridWidth 5 :weightx 1 :weighty 1 :fill 3 :fontSize 13)',
        '  (Wbutton :id "shop_check_btn" :label "Check / uncheck" :gridx 0 :gridy 2 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_check_shopping)")',
        '  (Wbutton :id "shop_clear_btn" :label "Clear" :gridx 1 :gridy 2 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_clear_shopping)")',
        '  (label :id "shop_hint" :label "Tip: checking items marks them done." :gridx 0 :gridy 3 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)',
        ')',
    }, "\n")
end

return M
