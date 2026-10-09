-- cookbook/app/screens/recipe_list.lua
-- Screen 1: browse / search recipes.
--
-- A screen module exposes { id, title, fragment() } where fragment() returns an
-- S-expression grid that main.lua embeds in the shared window. The list is a
-- `list-view` (WlsMulti) bound to an MLS store; the handle is injected by
-- main.lua through the __STORE_RECIPES__ placeholder.

local M = {}

M.id = "screen_list"
M.title = "Recipes"

function M.fragment()
    return table.concat({
        '(grid :id "screen_list" :gridx 0 :gridy 3 :gridWidth 5 :weightx 1 :weighty 1 :fill 3',
        '  (label :id "list_title" :label "Recipe list" :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 15)',
        '  (edit :id "search_ed" :label "" :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)',
        '  (Wbutton :id "search_btn" :label "Search" :gridx 2 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_search)")',
        '  (Wlabel :id "recipe_list" :label "(recipe list)" :gridx 0 :gridy 2 :gridWidth 5 :weightx 1 :weighty 1 :fill 3 :fontSize 13)',
        '  (Wbutton :id "open_recipe_btn" :label "Open" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_open_recipe)")',
        '  (Wbutton :id "new_recipe_btn" :label "New recipe" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_nav_editor)")',
        '  (Wbutton :id "delete_recipe_btn" :label "Delete" :gridx 2 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_delete_recipe)")',
        ')',
    }, "\n")
end

return M
