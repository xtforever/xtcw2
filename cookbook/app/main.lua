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
local WModel = require('wmodel')
local persistence = require('store.persistence')

-- The data file is resolved from commander/'s working directory. Tests set
-- COOKBOOK_DATA to a temp file so they never mutate the developer's data.
local DATA_PATH = os.getenv('COOKBOOK_DATA') or '../cookbook/data/recipes.lua'
local selected_id = nil
local editing_id = nil
local pending_delete_id = nil

local loaded = persistence.load(DATA_PATH)
if loaded then model.set_recipes(loaded) end

local screens = {
    require('screens.recipe_list'),
    require('screens.recipe_detail'),
    require('screens.recipe_editor'),
    require('screens.shopping_list'),
    require('screens.widget_gallery'),
    require('screens.tree_view'),
}

-- Screen id -> WtabWidget page index (0-based).
local screen_index = {
    screen_list = 0,
    screen_detail = 1,
    screen_editor = 2,
    screen_shopping = 3,
    screen_gallery = 4,
    screen_tree = 5,
}
local tab_names = { "Recipes", "Detail", "Editor", "Shopping", "Gallery", "Tree" }

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

local function load_detail(id)
    local recipe = model.get_by_id(id)
    if not recipe then return end
    selected_id = recipe.id
    gui.set("det_title", "label", recipe.title or "")
    gui.set("det_meta", "label", string.format(
        "%s  ·  %s  ·  %d servings  ·  %d min",
        recipe.author or "", recipe.category or "",
        tonumber(recipe.servings) or 0, tonumber(recipe.minutes) or 0))
    local rows = {}
    for _, ing in ipairs(recipe.ingredients or {}) do
        rows[#rows + 1] = table.concat({
            ing[1] or "", ing[2] or "", ing[3] or "" }, "\t")
    end
    gui.set_string_list("det_ingredients", "rows", rows)
    gui.set("det_steps", "text", recipe.steps or "")
end

-- Rebuild the recipe table from the model and keep a stable row<->id mapping.
-- `select_id` selects that recipe after the refresh, if it is still present.
local function build_recipe_model()
    return WModel.new{
        columns = {
            { title = "Name",     width = 0 },
            { title = "Category", width = 120 },
            { title = "Servings", width = 110, align = 2 },
            { title = "Minutes",  width = 100, align = 2 },
        },
        rows = model.list_rows(),
        ids = model.list_ids(),
    }
end

local function refresh_recipe_list(select_id)
    gui.set_model("recipe_list", build_recipe_model(),
                  { sort_column = 0, sort_ascending = true })
    if select_id then
        local proxy = gui.get_model("recipe_list")
        local view = proxy and proxy:view_of_id(select_id)
        if view then gui.set("recipe_list", "selected", view - 1) end
    end
end

local function persist()
    return persistence.save(DATA_PATH, model.recipes)
end

local function show(id)
    local index = screen_index[id] or 0
    gui.set("tabs", "selected", index)
    gui.set("status", "message", "Screen: " .. (tab_names[index + 1] or id)
        .. "  (" .. tostring(model.count()) .. " recipes)")
end

-- Callbacks are looked up as Lua globals by the host's LUA(...) dispatch.
function cb_tab()
    local index = tonumber(gui.get("tabs", "selected")) or 0
    gui.set("status", "message", "Tab: " .. (tab_names[index + 1] or "?"))
end

function cb_nav_list() show("screen_list") end
function cb_nav_detail() show("screen_detail") end
function cb_nav_shopping() show("screen_shopping") end

-- Assigned after `categories` is defined below.
local load_editor, clear_editor

function cb_nav_editor()
    if selected_id and model.get_by_id(selected_id) then
        editing_id = selected_id
        load_editor(model.get_by_id(selected_id))
    else
        cb_new_recipe()
        return
    end
    show("screen_editor")
end

function cb_new_recipe()
    editing_id = nil
    clear_editor()
    show("screen_editor")
end

function cb_noop() end

function cb_search()
    local q = gui.get("search_ed", "label") or ""
    local n = gui.set_model_filter("recipe_list", q)
    gui.set("status", "message",
            string.format("Search: '%s' — %d recipe(s)", q, n))
end

function cb_open_recipe()
    local id = tonumber(gui.get("recipe_list", "selectedId"))
    if not id then
        local index = (tonumber(gui.get("recipe_list", "selected")) or 0) + 1
        local recipe = model.get(index)
        id = recipe and recipe.id
    end
    if not id then return end
    load_detail(id)
    show("screen_detail")
end

local confirm_delete = false

function cb_toggle_confirm()
    confirm_delete = not confirm_delete
    gui.set("status", "message", "Delete confirmation: "
        .. (confirm_delete and "on" or "off"))
end

function cb_delete_recipe()
    local id = tonumber(gui.get("recipe_list", "selectedId"))
    if not id then
        local index = tonumber(gui.get("recipe_list", "selected")) or -1
        local recipe = model.get(index + 1)
        id = recipe and recipe.id
    end
    if not id then
        gui.set("status", "message", "Delete: no recipe selected")
        return
    end

    local recipe = model.get_by_id(id)
    pending_delete_id = id
    local dlg = gui.get_widget("confirm_delete")
    if dlg then
        gui.set("confirm_delete", "message",
            "Delete \"" .. (recipe and recipe.title or "recipe") .. "\"?")
        xtaction(dlg, "show")
    else
        cb_delete_result(1)
    end
end

function cb_delete_result(ok)
    ok = tonumber(ok)
    if not ok then ok = tonumber(gui.get("confirm_delete", "result")) end
    if ok ~= 1 or not pending_delete_id then
        pending_delete_id = nil
        gui.set("status", "message", "Delete cancelled")
        return
    end

    local id = pending_delete_id
    pending_delete_id = nil
    local recipe = model.get_by_id(id)
    model.remove(id)
    local saved_ok, save_err = persist()
    if selected_id == id then selected_id = nil end

    local first = model.recipes[1]
    refresh_recipe_list(first and first.id or nil)
    if first then
        load_detail(first.id)
    else
        gui.set("det_title", "label", "Recipe detail")
        gui.set("det_meta", "label", "No recipes.")
        gui.set_string_list("det_ingredients", "rows", {})
        gui.set("det_steps", "text", "")
    end
    if saved_ok then
        gui.set("status", "message",
            "Deleted: " .. (recipe and recipe.title or tostring(id)))
    else
        gui.set("status", "message", "Delete failed to persist: " .. tostring(save_err))
    end
end

local categories = { "Starter", "Main", "Dessert", "Breakfast" }
local category_index = 2

local function set_toggle(id, on)
    gui.set(id, "checked", on and true or false)
end

local function ingredients_text(recipe)
    local lines = {}
    for _, ing in ipairs(recipe.ingredients or {}) do
        lines[#lines + 1] = table.concat({
            ing[1] or "", ing[2] or "", ing[3] or "" }, "\t")
    end
    return table.concat(lines, "\n")
end

local function split_ingredients(text)
    local rows = {}
    for line in (text or ""):gmatch("[^\n]+") do
        local amount, unit, name = line:match("^([^\t]*)\t([^\t]*)\t(.*)$")
        if not amount then
            local a, u, n = line:match("^(%S+)%s+(%S+)%s+(.+)$")
            amount, unit, name = a, u, n
        end
        if not amount then
            amount, unit, name = "", "", line
        end
        rows[#rows + 1] = { amount or "", unit or "", name or "" }
    end
    return rows
end

local function select_category(name)
    local idx = 1
    for i, cat in ipairs(categories) do
        if cat == name then idx = i; break end
    end
    category_index = idx
    gui.set("ed_cat", "selected", idx - 1)
    local w = gui.get_widget("ed_cat")
    if w then xtaction(w, "refresh") end
end

load_editor = function(recipe)
    recipe = recipe or {}
    gui.set("ed_title", "label", recipe.title or "")
    gui.set("ed_author", "label", recipe.author or "")
    gui.set("ed_serv", "value", tonumber(recipe.servings) or 1)
    gui.set("ed_time", "value", tonumber(recipe.minutes) or 1)
    select_category(recipe.category or "Main")

    local tags = {}
    for _, tag in ipairs(recipe.tags or {}) do tags[tag] = true end
    set_toggle("ed_veg", tags["vegetarian"])
    set_toggle("ed_vegan", tags["vegan"])
    set_toggle("ed_gf", tags["gluten-free"])

    local ing = gui.get_widget("ed_ingredients")
    if ing then xtaction(ing, "set_text", ingredients_text(recipe)) end
    local steps = gui.get_widget("ed_steps")
    if steps then xtaction(steps, "set_text", recipe.steps or "") end
end

clear_editor = function()
    gui.set("ed_title", "label", "")
    gui.set("ed_author", "label", "")
    gui.set("ed_serv", "value", 4)
    gui.set("ed_time", "value", 30)
    select_category("Main")
    set_toggle("ed_veg", false)
    set_toggle("ed_vegan", false)
    set_toggle("ed_gf", false)
    local ing = gui.get_widget("ed_ingredients")
    if ing then xtaction(ing, "set_text", "") end
    local steps = gui.get_widget("ed_steps")
    if steps then xtaction(steps, "set_text", "") end
end

function cb_show_dialog()
    local dlg = gui.get_widget("gal_dialog")
    if dlg then xtaction(dlg, "show") end
end

function cb_dialog_result(ok)
    ok = tonumber(ok)
    if not ok then ok = tonumber(gui.get("gal_dialog", "result")) end
    gui.set("status", "message", "Dialog: " .. (ok == 1 and "OK" or "Cancel"))
end

function cb_sort_recipes()
    gui.sort_model("recipe_list", 0, true)
    gui.set("status", "message", "Recipes sorted by name")
end

function cb_sort_recipe_list(col)
    col = tonumber(col) or 0
    local cur = tonumber(gui.get("recipe_list", "sortColumn")) or -1
    local asc = true
    if cur == col then
        asc = not (gui.get("recipe_list", "sortAscending") and true or false)
    end
    gui.sort_model("recipe_list", col, asc)
    gui.set("status", "message",
            string.format("Sorted by column %d (%s)", col + 1,
                          asc and "ascending" or "descending"))
end

function cb_category_changed()
    local idx = tonumber(gui.get("ed_cat", "selected")) or -1
    category_index = idx + 1
    local cat = categories[category_index] or "?"
    gui.set("status", "message", "Category: " .. cat)
end

function cb_save()
    local title = gui.get("ed_title", "label") or ""
    if title == "" then
        gui.set("status", "message", "Save failed: title is required")
        return
    end

    local tags = {}
    if gui.get("ed_veg", "checked") then tags[#tags + 1] = "vegetarian" end
    if gui.get("ed_vegan", "checked") then tags[#tags + 1] = "vegan" end
    if gui.get("ed_gf", "checked") then tags[#tags + 1] = "gluten-free" end

    local ing_w = gui.get_widget("ed_ingredients")
    local step_w = gui.get_widget("ed_steps")
    local ing_text = ing_w and xtkaroed_get_text(ing_w) or ""
    local steps = step_w and xtkaroed_get_text(step_w) or ""

    local recipe = {
        title = title,
        author = gui.get("ed_author", "label") or "",
        category = gui.get("ed_cat", "label") or "Main",
        servings = tonumber(gui.get("ed_serv", "value")) or 1,
        minutes = tonumber(gui.get("ed_time", "value")) or 1,
        tags = tags,
        ingredients = split_ingredients(ing_text),
        steps = steps,
    }

    local id
    if editing_id and model.get_by_id(editing_id) then
        model.update(editing_id, recipe)
        id = editing_id
    else
        id = model.add(recipe)
    end
    editing_id = id

    local saved_ok, save_err = persist()
    refresh_recipe_list(id)
    load_detail(id)
    show("screen_detail")
    if saved_ok then
        gui.set("status", "message", "Saved: " .. title)
    else
        gui.set("status", "message", "Save failed: " .. tostring(save_err))
    end
end

function cb_cancel() show("screen_list") end

function cb_check_shopping()
    local index = tonumber(gui.get("shop_list", "selected")) or -1
    if index < 0 or not shopping_rows[index + 1] then return end
    local row = shopping_rows[index + 1]
    row[4] = (row[4] == "yes") and "no" or "yes"
    gui.set_string_list("shop_list", "rows", shopping_strings())
    gui.set("status", "message", "Shopping item " .. (index + 1) .. ": " .. row[4])
end

function cb_clear_shopping()
    for _, row in ipairs(shopping_rows) do
        row[4] = "no"
    end
    gui.set_string_list("shop_list", "rows", shopping_strings())
    gui.set("status", "message", "Shopping list cleared")
end

function cb_about()
    gui.set("status", "message", "xtcw2 Cookbook — improved widget set testing ground")
end

function cb_quit() xtappexitflag() end

-- ---------------------------------------------------------------------------
-- Assemble the single UI source parsed by lui.run().
-- ---------------------------------------------------------------------------
local ui = macros.PRELUDE .. [[
(window :id "win" :title "xtcw2 Cookbook" :width 1000 :height 680
  (app-window :id "main" :windowMargin 12 :rowGap 8
                  :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3
    (WMenuBar :id "menubar" :slot "menu" :barHeight 28
      (WmenuButton :id "menu_file" :label "File"
                   :buttonWidth 64 :buttonHeight 28
        (WPopupMenu :id "menu_file_popup"
          (Wbutton :id "mi_open" :label "Open recipe" :callback "LUA(cb_open_recipe)")
          (WAction :id "mi_confirm" :label "Confirm delete"
                   :checkable TRUE :checked FALSE :shortcut "Ctrl+D"
                   :callback "LUA(cb_toggle_confirm)")
          (Wbutton :id "mi_delete" :label "Delete recipe" :callback "LUA(cb_delete_recipe)")
          (Wbutton :id "mi_quit" :label "Quit" :callback "LUA(cb_quit)")))
      (WmenuButton :id "menu_view" :label "View"
                   :buttonWidth 64 :buttonHeight 28
        (WPopupMenu :id "menu_view_popup"
          (Wbutton :id "mi_recipes" :label "Recipes" :callback "LUA(cb_nav_list)")
          (Wbutton :id "mi_detail" :label "Detail" :callback "LUA(cb_nav_detail)")
          (Wbutton :id "mi_editor" :label "Editor" :callback "LUA(cb_nav_editor)")
          (Wbutton :id "mi_shopping" :label "Shopping" :callback "LUA(cb_nav_shopping)")
          (Wbutton :id "mi_sort" :label "Sort recipes" :tooltip "Sort by name"
                    :callback "LUA(cb_sort_recipes)")))
      (WmenuButton :id "menu_help" :label "Help"
                   :buttonWidth 64 :buttonHeight 28
        (WPopupMenu :id "menu_help_popup"
          (Wbutton :id "mi_about" :label "About" :callback "LUA(cb_about)"))))
    (grid :id "toolbar" :slot "toolbar" :weightx 1 :weighty 0 :fill 3 :defaultDistance 8
      (Wlabel :id "hdr"
         :label "\\includesvg{../cookbook/assets/dish.svg}{18pt}{18pt} xtcw2 Cookbook"
         :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :fontSize 17)
      (WToolBar :id "toolbar_actions"
                :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3
                :toolbarHeight 36 :itemGap 8 :toolbarPadding 4
        (Wbutton :id "nav_new" :label "New" :tooltip "Create a new recipe"
                 :callback "LUA(cb_new_recipe)")
        (Wbutton :id "nav_quit" :label "Quit" :tooltip "Quit the cookbook"
                 :callback "LUA(cb_quit)")))
    (tabs :id "tabs" :slot "content" :weightx 1 :weighty 1 :fill 3
       :callback "LUA(cb_tab)"
]]
    .. screens[1].fragment() .. "\n"
    .. screens[2].fragment() .. "\n"
    .. screens[3].fragment() .. "\n"
    .. screens[4].fragment() .. "\n"
    .. screens[5].fragment() .. "\n"
    .. screens[6].fragment() .. "\n"
    .. [[
    )
    (WStatusBar :id "status" :slot "status" :message "Ready"
                :itemGap 8 :barPadding 6)
  ))
]]

local ok, err = lui.run(ui)
if not ok then
    error("cookbook: " .. tostring(err))
end

-- Populate the list views and the initial detail page.
local first_recipe = model.recipes[1]
refresh_recipe_list(first_recipe and first_recipe.id or nil)
gui.set_string_list("shop_list", "rows", shopping_strings())
gui.set_string_list("ed_cat", "items", categories)
gui.set("ed_cat", "selected", category_index - 1)
local cat_widget = gui.get_widget("ed_cat")
if cat_widget then xtaction(cat_widget, "refresh") end

gui.set_string_list("tree_view", "nodes", {
    "0\tRecipes", "1\tItalian", "2\tSpaghetti",
    "1\tDessert", "2\tCake", "0\tShopping", "1\tGroceries",
})
gui.set_string_list("gal_combo", "items", { "Small", "Medium", "Large" })
gui.set("gal_combo", "selected", 1)
local gal_combo = gui.get_widget("gal_combo")
if gal_combo then xtaction(gal_combo, "refresh") end
load_detail(first_recipe and first_recipe.id or nil)
show("screen_list")
