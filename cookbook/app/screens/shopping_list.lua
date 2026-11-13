-- cookbook/app/screens/shopping_list.lua
-- Screen 4: aggregated shopping list with check/uncheck.

local M = {}

M.id = "screen_shopping"
M.title = "Shopping"

function M.fragment()
    return table.concat({
        '(grid :id "screen_shopping" :tab-title "Shopping" :defaultDistance 8',
        '  (label :id "shop_title" :label "Shopping list" :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (label :id "shop_hint" :label "Aggregated from all recipes. Use Check to mark an item." :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)',
        '  (WListView :id "shop_list" :gridx 0 :gridy 2 :gridWidth 2 :weightx 1 :weighty 1 :fill 3',
        '     :columnCount 4',
        '     :columnTitles "Amount\\tUnit\\tIngredient\\tDone"',
        '     :columnWidths "100,60,0,80"',
        '     :columnAligns "2,0,0,1"',
        '     :columnKinds "0,0,0,0"',
        '     :showHeader 1 :gridLines 1 :alternateRows 1 :visibleRows 8 :selected 0)',
        '  (Wbutton :id "shop_clear_btn" :label "Clear" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_clear_shopping)")',
        '  (Wbutton :id "shop_check_btn" :label "Check / uncheck" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :primary TRUE :callback "LUA(cb_check_shopping)")',
        ')',
    }, "\n")
end

return M
