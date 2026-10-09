-- cookbook/app/screens/recipe_editor.lua
-- Screen 3: create / edit a recipe (form).
--
-- Uses Wedit, WspinBox, Wcombo, Xaw toggles and two KaroEd multi-line editors
-- for ingredients and steps.

local M = {}

M.id = "screen_editor"
M.title = "Editor"

function M.fragment()
    return table.concat({
        '(grid :id "screen_editor" :gridx 0 :gridy 3 :gridWidth 5 :weightx 1 :weighty 1 :fill 3',
        '  (label :id "ed_hdr" :label "Recipe editor" :gridx 0 :gridy 0 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :fontSize 15)',
        '  (label :id "ed_title_lbl" :label "Title:" :gridx 0 :gridy 1 :weightx 0 :weighty 0 :fill 3)',
        '  (edit :id "ed_title" :label "" :gridx 1 :gridy 1 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)',
        '  (label :id "ed_author_lbl" :label "Author:" :gridx 0 :gridy 2 :weightx 0 :weighty 0 :fill 3)',
        '  (edit :id "ed_author" :label "" :gridx 1 :gridy 2 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)',
        '  (label :id "ed_serv_lbl" :label "Servings:" :gridx 0 :gridy 3 :weightx 0 :weighty 0 :fill 3)',
        '  (WspinBox :id "ed_serv" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :value 4 :min 1 :max 20)',
        '  (label :id "ed_time_lbl" :label "Minutes:" :gridx 2 :gridy 3 :weightx 0 :weighty 0 :fill 3)',
        '  (WspinBox :id "ed_time" :gridx 3 :gridy 3 :weightx 1 :weighty 0 :fill 3 :value 30 :min 1 :max 600)',
        '  (label :id "ed_cat_lbl" :label "Category:" :gridx 0 :gridy 4 :weightx 0 :weighty 0 :fill 3)',
        '  (Wcombo :id "ed_cat" :gridx 1 :gridy 4 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_noop)")',
        '  (toggle :id "ed_veg" :label "Vegetarian" :gridx 2 :gridy 4 :weightx 1 :weighty 0 :fill 3)',
        '  (toggle :id "ed_vegan" :label "Vegan" :gridx 3 :gridy 4 :weightx 1 :weighty 0 :fill 3)',
        '  (toggle :id "ed_gf" :label "Gluten-free" :gridx 0 :gridy 5 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)',
        '  (label :id "ed_ing_lbl" :label "Ingredients (amount, unit, name per line):" :gridx 0 :gridy 6 :gridWidth 4 :weightx 1 :weighty 0 :fill 3)',
        '  (KaroEd :id "ed_ingredients" :gridx 0 :gridy 7 :gridWidth 4 :weightx 1 :weighty 1 :fill 3 :xftFont "Monospace-12" :grid_width 80 :grid_height 5 :auto_resize 0 :callback "LUA(cb_noop)")',
        '  (label :id "ed_steps_lbl" :label "Steps:" :gridx 0 :gridy 8 :gridWidth 4 :weightx 1 :weighty 0 :fill 3)',
        '  (KaroEd :id "ed_steps" :gridx 0 :gridy 9 :gridWidth 4 :weightx 1 :weighty 1 :fill 3 :xftFont "Monospace-12" :grid_width 80 :grid_height 6 :auto_resize 0 :callback "LUA(cb_noop)")',
        '  (Wbutton :id "ed_save" :label "Save" :gridx 2 :gridy 10 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_save)")',
        '  (Wbutton :id "ed_cancel" :label "Cancel" :gridx 3 :gridy 10 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_cancel)")',
        ')',
    }, "\n")
end

return M
