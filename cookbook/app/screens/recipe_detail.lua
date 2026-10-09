-- cookbook/app/screens/recipe_detail.lua
-- Screen 2: read-only detail view of the selected recipe.

local M = {}

M.id = "screen_detail"
M.title = "Detail"

function M.fragment()
    return table.concat({
        '(grid :id "screen_detail" :gridx 0 :gridy 3 :gridWidth 5 :weightx 1 :weighty 1 :fill 3',
        '  (label :id "det_title" :label "Recipe detail" :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (label :id "det_meta" :label "Select a recipe in the list." :gridx 0 :gridy 1 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)',
        '  (label :id "det_ing_lbl" :label "Ingredients" :gridx 0 :gridy 2 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (Wlabel :id "det_ingredients" :label "(ingredients)" :gridx 0 :gridy 3 :gridWidth 5 :weightx 1 :weighty 1 :fill 3 :fontSize 13)',
        '  (label :id "det_steps_lbl" :label "Preparation" :gridx 0 :gridy 4 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (retex :id "det_steps" :text "No recipe selected." :gridx 0 :gridy 5 :gridWidth 3 :weightx 1 :weighty 1 :fill 3 :fontSize 13)',
        ')',
    }, "\n")
end

return M
