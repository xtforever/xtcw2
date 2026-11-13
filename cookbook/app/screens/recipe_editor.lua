-- cookbook/app/screens/recipe_editor.lua
-- Screen 3: create / edit a recipe (form).
--
-- Modern themed controls: Wedit, WspinBox, WToggleSwitch, WComboBox, KaroEd
-- and Wbutton.  The action bar is a top toolbar row (Cancel + primary Save)
-- so it stays visible while the form scrolls.

local M = {}

M.id = "screen_editor"
M.title = "Editor"

function M.fragment()
    return table.concat({
        '(grid :id "screen_editor" :tab-title "Editor" :defaultDistance 8',
        '  (label :id "ed_hdr" :label "Recipe editor" :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (Wbutton :id "ed_cancel" :label "Cancel" :gridx 2 :gridy 0 :weightx 0 :weighty 0 :fill 3 :callback "LUA(cb_cancel)")',
        '  (Wbutton :id "ed_save" :label "Save" :gridx 3 :gridy 0 :weightx 0 :weighty 0 :fill 3 :primary TRUE :callback "LUA(cb_save)")',
        '  (grid :id "ed_form" :gridx 0 :gridy 1 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :defaultDistance 8 :borderWidth 0',
        '    (label :id "ed_title_lbl" :label "Title:" :gridx 0 :gridy 0 :weightx 0 :weighty 0 :fill 3)',
        '    (edit :id "ed_title" :label "Recipe title" :gridx 1 :gridy 0 :weightx 1 :weighty 0 :fill 3)',
        '    (label :id "ed_author_lbl" :label "Author:" :gridx 0 :gridy 1 :weightx 0 :weighty 0 :fill 3)',
        '    (edit :id "ed_author" :label "Author" :gridx 1 :gridy 1 :weightx 1 :weighty 0 :fill 3)',
        '    (label :id "ed_serv_lbl" :label "Servings:" :gridx 0 :gridy 2 :weightx 0 :weighty 0 :fill 3)',
        '    (WspinBox :id "ed_serv" :gridx 1 :gridy 2 :weightx 1 :weighty 0 :fill 3 :value 4 :min 1 :max 20)',
        '    (label :id "ed_time_lbl" :label "Minutes:" :gridx 2 :gridy 2 :weightx 0 :weighty 0 :fill 3)',
        '    (WspinBox :id "ed_time" :gridx 3 :gridy 2 :weightx 1 :weighty 0 :fill 3 :value 30 :min 1 :max 600)',
        '    (label :id "ed_cat_lbl" :label "Category:" :gridx 0 :gridy 3 :weightx 0 :weighty 0 :fill 3)',
        '    (WComboBox :id "ed_cat" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :placeholder "Main" :linesHeight 4 :callback "LUA(cb_category_changed)")',
        '  )',
        '  (label :id "ed_diet_lbl" :label "Dietary" :gridx 0 :gridy 2 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (grid :id "ed_veg_box" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3 :borderWidth 0',
        '    (label :label "Vegetarian" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)',
        '    (WToggleSwitch :id "ed_veg" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :checked FALSE))',
        '  (grid :id "ed_vegan_box" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :borderWidth 0',
        '    (label :label "Vegan" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)',
        '    (WToggleSwitch :id "ed_vegan" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :checked FALSE))',
        '  (grid :id "ed_gf_box" :gridx 2 :gridy 3 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :borderWidth 0',
        '    (label :label "Gluten-free" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)',
        '    (WToggleSwitch :id "ed_gf" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :checked FALSE))',
        '  (label :id "ed_ing_lbl" :label "Ingredients (amount, unit, name per line)" :gridx 0 :gridy 4 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (KaroEd :id "ed_ingredients" :gridx 0 :gridy 5 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :xftFont "Monospace-12" :grid_width 80 :grid_height 3 :height 80 :auto_resize 0 :callback "LUA(cb_noop)")',
        '  (label :id "ed_steps_lbl" :label "Steps" :gridx 0 :gridy 6 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :fontSize 14)',
        '  (KaroEd :id "ed_steps" :gridx 0 :gridy 7 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :xftFont "Monospace-12" :grid_width 80 :grid_height 3 :height 80 :auto_resize 0 :callback "LUA(cb_noop)")',
        ')',
    }, "\n")
end

return M
