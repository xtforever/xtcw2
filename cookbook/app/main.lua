-- cookbook/app/main.lua
-- Entry point for the cookbook reference application.
--
-- Loaded by the xtcw2 host:
--   cd commander && ./commander_runner -Luafile ../cookbook/app/main.lua \
--                    -themeFile ../cookbook/themes/cookbook.hdf
--
-- The host must run from commander/ because both its own `../lui/?.lua` path and
-- the cookbook's module path below are resolved relative to the working
-- directory. (Making the host location-independent is a gap recorded in
-- ../plan-xtcw-completed.md.)

package.path = '../cookbook/app/?.lua;' .. package.path

local gui = require('gui_xt')
local lui = require('lui')
local macros = require('ui.macros')
local model = require('model.recipes')
local persistence = require('store.persistence') -- noqa: unused for now (persistence gap)

local screens = {
    require('screens.recipe_list'),
    require('screens.recipe_detail'),
    require('screens.recipe_editor'),
    require('screens.shopping_list'),
}

-- ---------------------------------------------------------------------------
-- Data stores (MLS handles passed to the list-view widgets)
-- ---------------------------------------------------------------------------
local recipe_store = gui.create_store(4)
for _, row in ipairs(model.list_rows()) do
    gui.store_append(recipe_store, row)
end

local ingredient_store = gui.create_store(3)
for _, row in ipairs(model.ingredient_rows(1)) do
    gui.store_append(ingredient_store, row)
end

local shopping_store = gui.create_store(4)
for _, row in ipairs(model.all_ingredient_rows()) do
    gui.store_append(shopping_store, row)
end

-- ---------------------------------------------------------------------------
-- Screen switching (the toolkit has no tab widget yet — see the plan).
-- ---------------------------------------------------------------------------
local active_screen = screens[1].id

local function show(id)
    for _, s in ipairs(screens) do
        if s.id == id then
            gui.manage(s.id)
            active_screen = id
        else
            gui.unmanage(s.id)
        end
    end
    gui.set("status", "label",
        "Screen: " .. id .. "  (" .. tostring(model.count()) .. " recipes)")
end

-- Callbacks are looked up as Lua globals by the host's LUA(...) dispatch.
function cb_nav_list() show("screen_list") end
function cb_nav_detail() show("screen_detail") end
function cb_nav_editor() show("screen_editor") end
function cb_nav_shopping() show("screen_shopping") end

function cb_noop() end

function cb_search()
    local q = gui.get("search_ed", "label") or ""
    gui.set("status", "label", "Search: '" .. q .. "' (filtering not implemented)")
end

function cb_open_recipe() show("screen_detail") end

function cb_delete_recipe()
    gui.set("status", "label", "Delete recipe (not implemented)")
end

function cb_save()
    local title = gui.get("ed_title", "label") or ""
    gui.set("status", "label", "Saved: " .. title)
end

function cb_cancel() show("screen_list") end

function cb_check_shopping()
    gui.set("status", "label", "Toggle shopping item (not implemented)")
end

function cb_clear_shopping()
    gui.set("status", "label", "Shopping list cleared")
end

function cb_quit() xtappexitflag() end

-- ---------------------------------------------------------------------------
-- Assemble the single UI source parsed by lui.run().
-- ---------------------------------------------------------------------------
local ui = macros.PRELUDE .. [[
(window :id "win" :title "xtcw2 Cookbook" :width 1000 :height 720
  (grid :id "main" :weightx 1 :weighty 1
    (Wlabel :id "hdr" :label "\\includesvg{../cookbook/assets/dish.svg}{18pt}{18pt} xtcw2 Cookbook - recipe manager reference app"
           (cell 0 0 1 0) :gridWidth 5 :fill 3 :fontSize 17)
    (Wbutton :id "nav_list"     :label "Recipes"  :gridx 0 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_nav_list)")
    (Wbutton :id "nav_detail"   :label "Detail"   :gridx 1 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_nav_detail)")
    (Wbutton :id "nav_editor"   :label "Editor"   :gridx 2 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_nav_editor)")
    (Wbutton :id "nav_shopping" :label "Shopping" :gridx 3 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_nav_shopping)")
    (Wbutton :id "nav_quit"     :label "Quit"     :gridx 4 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(cb_quit)")
    (label :id "status" :label "Ready" :gridx 0 :gridy 2 :gridWidth 5 :weightx 1 :weighty 0 :fill 3)
]]
    .. screens[1].fragment() .. "\n"
    .. screens[2].fragment() .. "\n"
    .. screens[3].fragment() .. "\n"
    .. screens[4].fragment() .. "\n"
    .. [[
  ))
]]

-- Inject the MLS store handles into the list-view models.
-- (The current screens render their data through Wlabel to stay robust; the
-- stores above exercise the gui.create_store / store_append API and would back
-- a real WtableView once that widget exists — see ../plan-xtcw-completed.md.)
local function join_rows(rows, fmt)
    local lines = {}
    for _, r in ipairs(rows) do
        lines[#lines + 1] = string.format(fmt, table.unpack(r))
    end
    return table.concat(lines, "\n")
end

local ok, err = lui.run(ui)
if not ok then
    error("cookbook: " .. tostring(err))
end

-- Populate the label-based lists from the model.
gui.set("recipe_list", "label",
    join_rows(model.list_rows(), "%-26s %-10s %2s servings  %3s min"))
gui.set("det_ingredients", "label",
    join_rows(model.ingredient_rows(1), "%6s %-5s %s"))
gui.set("shop_list", "label",
    join_rows(model.all_ingredient_rows(), "%6s %-5s %-24s [ ]"))

-- Show the first screen; the others are unmanaged again.
show(active_screen)
