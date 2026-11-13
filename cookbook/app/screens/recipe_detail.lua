-- cookbook/app/screens/recipe_detail.lua
-- Screen 2: read-only detail view of the selected recipe.
--
-- Actions sit in the top row so they stay visible while the ingredients and
-- preparation text scroll.

local M = {}

M.id = "screen_detail"
M.title = "Detail"

function M.fragment()
    return table.concat({
        '(grid :id "screen_detail" :tab-title "Detail" :defaultDistance 8',
        '  (label :id "det_title" :label "Recipe detail" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (Wbutton :id "det_back" :label "Back" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :callback "LUA(cb_nav_list)")',
        '  (Wbutton :id "det_edit" :label "Edit" :gridx 2 :gridy 0 :weightx 0 :weighty 0 :fill 3 :primary TRUE :callback "LUA(cb_nav_editor)")',
        '  (label :id "det_meta" :label "Select a recipe in the list." :gridx 0 :gridy 1 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)',
        '  (label :id "det_ing_lbl" :label "Ingredients" :gridx 0 :gridy 2 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (WListView :id "det_ingredients" :gridx 0 :gridy 3 :gridWidth 3 :weightx 1 :weighty 1 :fill 3',
        '     :columnCount 3',
        '     :columnTitles "Amount\\tUnit\\tIngredient"',
        '     :columnWidths "100,60,0"',
        '     :columnAligns "2,0,0"',
        '     :columnKinds "0,0,0"',
        '     :showHeader 1 :gridLines 1 :alternateRows 1 :visibleRows 4)',
        '  (label :id "det_steps_lbl" :label "Preparation" :gridx 0 :gridy 4 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (retex :id "det_steps" :text "No recipe selected." :gridx 0 :gridy 5 :gridWidth 3 :weightx 1 :weighty 1 :fill 3 :fontSize 13 :height 100)',
        ')',
    }, "\n")
end

return M
