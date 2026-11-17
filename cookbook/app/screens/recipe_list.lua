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
        '  (WLineEdit :id "search_ed" :label "" :placeholder "Search recipes" :gridx 1 :gridy 1 :weightx 1 :weighty 0 :fill 3)',
        '  (Wbutton :id "search_btn" :label "Search" :gridx 2 :gridy 1 :weightx 0 :weighty 0 :fill 3 :callback "LUA(cb_search)")',
        '  (table-view :id "recipe_list" :gridx 0 :gridy 2 :gridWidth 3 :weightx 1 :weighty 1 :fill 3',
        '     :columnCount 4',
        '     :columnTitles "Name\\tCategory\\tServings\\tMinutes"',
        '     :columnWidths "0,120,110,100"',
        '     :columnAligns "0,0,2,2"',
        '     :columnKinds "0,0,0,0"',
        '     :showHeader 1 :gridLines 1 :alternateRows 1 :visibleRows 6 :selected 0 :sortColumn 0 :sortAscending TRUE',
        '     :tooltip "Double-click a row to open the recipe"',
        '     :on-row-activated "LUA(cb_open_recipe)" :on-activate "LUA(cb_open_recipe)"',
        '     :on-header "LUA(cb_sort_recipe_list)")',
        '  (Wbutton :id "delete_recipe_btn" :label "Delete" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_delete_recipe)")',
        '  (Wbutton :id "new_recipe_btn" :label "New recipe" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_new_recipe)")',
        '  (Wbutton :id "open_recipe_btn" :label "Open" :gridx 2 :gridy 3 :weightx 1 :weighty 0 :fill 3 :primary TRUE :callback "LUA(cb_open_recipe)")',
        '  (WDialog :id "confirm_delete" :gridx 0 :gridy 4 :gridWidth 3 :weightx 1 :weighty 0 :fill 3',
        '           :title "Delete recipe" :message "Delete this recipe?"',
        '           :okLabel "Delete" :cancelLabel "Cancel"',
        '           :callback "LUA(cb_delete_result)")',
        ')',
    }, "\n")
end

return M
