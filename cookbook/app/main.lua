-- cookbook/app/main.lua
-- Entry point for the cookbook reference application.
--
-- Loaded by the xtcw2 host:
--   cd commander && ./commander_runner -Luafile ../cookbook/app/main.lua \
--                    -themeFile ../cookbook/themes/cookbook.hdf
--
-- The layout follows style-guide.md: a header/toolbar row, a tabbed content
-- area that fills the window, and a status bar at the bottom.  The screens use
-- the modern themed widgets (WListView, WToggleSwitch, Wbutton, Wedit,
-- WspinBox, KaroEd), not Xaw.

package.path = '../cookbook/app/?.lua;' .. package.path

local gui = require('gui_xt')
local lui = require('lui')
local macros = require('ui.macros')
local model = require('model.recipes')

local screens = {
    require('screens.recipe_list'),
    require('screens.recipe_detail'),
    require('screens.recipe_editor'),
    require('screens.shopping_list'),
}

-- Screen id -> WtabWidget page index (0-based).
local screen_index = {
    screen_list = 0,
    screen_detail = 1,
    screen_editor = 2,
    screen_shopping = 3,
}
local tab_names = { "Recipes", "Detail", "Editor", "Shopping" }

-- Shopping rows are mutable (the Done column toggles); the WListView is
-- refreshed from this table.
local shopping_rows = {}
for _, row in ipairs(model.all_ingredient_rows()) do
    shopping_rows[#shopping_rows + 1] = row
end

local function shopping_strings()
    local out = {}
    for _, row in ipairs(shopping_rows) do
        out[#out + 1] = table.concat(row, "\t")
    end
    return out
end

local function load_detail(index)
    local recipe = model.get(index)
    if not recipe then return end
    gui.set("det_title", "label", recipe.title)
    gui.set("det_meta", "label", string.format(
        "%s  ·  %s  ·  %d servings  ·  %d min",
        recipe.author, recipe.category, recipe.servings, recipe.minutes))
    local rows = {}
    for _, ing in ipairs(recipe.ingredients) do
        rows[#rows + 1] = table.concat(ing, "\t")
    end
    gui.set_string_list("det_ingredients", "rows", rows)
    gui.set("det_steps", "text", recipe.steps)
end

local function show(id)
    local index = screen_index[id] or 0
    gui.set("tabs", "selected", index)
    gui.set("status", "label", "Screen: " .. (tab_names[index + 1] or id)
        .. "  (" .. tostring(model.count()) .. " recipes)")
end

-- Callbacks are looked up as Lua globals by the host's LUA(...) dispatch.
function cb_tab()
    local index = tonumber(gui.get("tabs", "selected")) or 0
    gui.set("status", "label", "Tab: " .. (tab_names[index + 1] or "?"))
end

function cb_nav_list() show("screen_list") end
function cb_nav_detail() show("screen_detail") end
function cb_nav_editor() show("screen_editor") end
function cb_nav_shopping() show("screen_shopping") end

function cb_noop() end

function cb_search()
    local q = gui.get("search_ed", "label") or ""
    gui.set("status", "label", "Search: '" .. q .. "' (filtering not implemented)")
end

function cb_open_recipe()
    local index = (tonumber(gui.get("recipe_list", "selected")) or 0) + 1
    load_detail(index)
    show("screen_detail")
end

function cb_delete_recipe()
    gui.set("status", "label", "Delete recipe (not implemented)")
end

local categories = { "Starter", "Main", "Dessert", "Breakfast" }
local category_index = 2

function cb_cycle_category()
    category_index = (category_index % #categories) + 1
    gui.set("ed_cat", "label", categories[category_index])
    gui.set("status", "label", "Category: " .. categories[category_index])
end

function cb_save()
    local title = gui.get("ed_title", "label") or ""
    local category = gui.get("ed_cat", "label") or ""
    gui.set("status", "label", "Saved: " .. title .. "  (" .. category .. ")")
end

function cb_cancel() show("screen_list") end

function cb_check_shopping()
    local index = tonumber(gui.get("shop_list", "selected")) or -1
    if index < 0 or not shopping_rows[index + 1] then return end
    local row = shopping_rows[index + 1]
    row[4] = (row[4] == "yes") and "no" or "yes"
    gui.set_string_list("shop_list", "rows", shopping_strings())
    gui.set("status", "label", "Shopping item " .. (index + 1) .. ": " .. row[4])
end

function cb_clear_shopping()
    for _, row in ipairs(shopping_rows) do
        row[4] = "no"
    end
    gui.set_string_list("shop_list", "rows", shopping_strings())
    gui.set("status", "label", "Shopping list cleared")
end

function cb_quit() xtappexitflag() end

-- ---------------------------------------------------------------------------
-- Assemble the single UI source parsed by lui.run().
-- ---------------------------------------------------------------------------
local ui = macros.PRELUDE .. [[
(window :id "win" :title "xtcw2 Cookbook" :width 1000 :height 760
  (grid :id "main" :weightx 1 :weighty 1 :defaultDistance 12
    (Wlabel :id "hdr"
       :label "\\includesvg{../cookbook/assets/dish.svg}{18pt}{18pt} xtcw2 Cookbook"
       :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :fontSize 17)
    (Wbutton :id "nav_quit" :label "Quit"
       :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :callback "LUA(cb_quit)")
    (tabs :id "tabs" :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1 :fill 3
       :callback "LUA(cb_tab)"
]]
    .. screens[1].fragment() .. "\n"
    .. screens[2].fragment() .. "\n"
    .. screens[3].fragment() .. "\n"
    .. screens[4].fragment() .. "\n"
    .. [[
    )
    (label :id "status" :label "Ready" :gridx 0 :gridy 2 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
  ))
]]

local ok, err = lui.run(ui)
if not ok then
    error("cookbook: " .. tostring(err))
end

-- Populate the list views and the initial detail page.
local recipe_rows = {}
for _, row in ipairs(model.list_rows()) do
    recipe_rows[#recipe_rows + 1] = table.concat(row, "\t")
end
gui.set_string_list("recipe_list", "rows", recipe_rows)
gui.set_string_list("shop_list", "rows", shopping_strings())
load_detail(1)
show("screen_list")
