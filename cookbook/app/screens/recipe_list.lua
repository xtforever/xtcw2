-- cookbook/app/screens/recipe_list.lua
-- Screen 1: browse / search recipes with a real WListView.
--
-- The page is a child of the WtabWidget in main.lua; it only carries the
-- `:tab-title` constraint.  Rows are injected after lui.run() with
-- gui.set_string_list().

local M = {}

M.id = "screen_list"
M.title = "Recipes"

function M.fragment()
    return table.concat({
        '(grid :id "screen_list" :tab-title "Recipes" :defaultDistance 8',
        '  (label :id "list_title" :label "Recipes" :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (label :id "search_lbl" :label "Search:" :gridx 0 :gridy 1 :weightx 0 :weighty 0 :fill 3)',
        '  (edit :id "search_ed" :label "Search recipes" :gridx 1 :gridy 1 :weightx 1 :weighty 0 :fill 3)',
        '  (Wbutton :id "search_btn" :label "Search" :gridx 2 :gridy 1 :weightx 0 :weighty 0 :fill 3 :callback "LUA(cb_search)")',
        '  (WListView :id "recipe_list" :gridx 0 :gridy 2 :gridWidth 3 :weightx 1 :weighty 1 :fill 3',
        '     :columnCount 4',
        '     :columnTitles "Name\\tCategory\\tServings\\tMinutes"',
        '     :columnWidths "0,120,110,100"',
        '     :columnAligns "0,0,2,2"',
        '     :columnKinds "0,0,0,0"',
        '     :showHeader 1 :gridLines 1 :alternateRows 1 :visibleRows 6 :selected 0',
        '     :on-row-activated "LUA(cb_open_recipe)" :on-activate "LUA(cb_open_recipe)")',
        '  (Wbutton :id "delete_recipe_btn" :label "Delete" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_delete_recipe)")',
        '  (Wbutton :id "new_recipe_btn" :label "New recipe" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_nav_editor)")',
        '  (Wbutton :id "open_recipe_btn" :label "Open" :gridx 2 :gridy 3 :weightx 1 :weighty 0 :fill 3 :primary TRUE :callback "LUA(cb_open_recipe)")',
        ')',
    }, "\n")
end

return M
