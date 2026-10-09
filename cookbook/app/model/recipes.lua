-- cookbook/app/model/recipes.lua
-- In-memory recipe model for the cookbook reference app.
--
-- This is deliberately a plain Lua module (no database, no JSON yet). It holds a
-- small sample dataset and a few helpers used by the screens. Persistence is a
-- separate concern — see app/store/persistence.lua.

local M = {}

-- A recipe is a table:
--   { title, author, category, servings, minutes, tags = {..},
--     ingredients = { { amount, unit, name }, ... }, steps = "text" }
M.recipes = {
    {
        title = "Spaghetti Carbonara",
        author = "Demo Chef",
        category = "Main",
        servings = 4,
        minutes = 25,
        tags = { "pasta", "quick" },
        ingredients = {
            { "200", "g",  "Spaghetti" },
            { "100", "g",  "Pancetta" },
            { "2",   "",   "Eggs" },
            { "50",  "g",  "Parmesan" },
            { "1",   "tsp","Black pepper" },
        },
        steps = "1. Boil the pasta until al dente.\n"
             .. "2. Fry the pancetta until crisp.\n"
             .. "3. Whisk eggs with grated parmesan and pepper.\n"
             .. "4. Combine off the heat and serve immediately.",
    },
    {
        title = "Tomato Soup",
        author = "Demo Chef",
        category = "Starter",
        servings = 2,
        minutes = 30,
        tags = { "vegetarian", "soup" },
        ingredients = {
            { "500", "g", "Tomatoes" },
            { "1",   "",  "Onion" },
            { "2",   "",  "Garlic cloves" },
            { "300", "ml","Vegetable stock" },
        },
        steps = "1. Chop onion and garlic.\n"
             .. "2. Fry gently, add tomatoes and stock.\n"
             .. "3. Simmer 20 minutes and blend.",
    },
    {
        title = "Apple Crumble",
        author = "Demo Chef",
        category = "Dessert",
        servings = 6,
        minutes = 45,
        tags = { "sweet", "baking" },
        ingredients = {
            { "800", "g",  "Apples" },
            { "150", "g",  "Flour" },
            { "100", "g",  "Butter" },
            { "80",  "g",  "Sugar" },
        },
        steps = "1. Peel and slice the apples into a dish.\n"
             .. "2. Rub flour, butter and sugar into crumbs.\n"
             .. "3. Cover the apples and bake at 180C for 30 min.",
    },
    {
        title = "Greek Salad",
        author = "Demo Chef",
        category = "Starter",
        servings = 2,
        minutes = 10,
        tags = { "vegetarian", "quick", "salad" },
        ingredients = {
            { "2",   "",  "Tomatoes" },
            { "1",   "",  "Cucumber" },
            { "100", "g", "Feta" },
            { "50",  "g", "Olives" },
            { "2",   "tbsp", "Olive oil" },
        },
        steps = "1. Chop tomatoes and cucumber.\n"
             .. "2. Add feta and olives.\n"
             .. "3. Dress with olive oil.",
    },
    {
        title = "Pancakes",
        author = "Demo Chef",
        category = "Breakfast",
        servings = 4,
        minutes = 20,
        tags = { "sweet", "quick" },
        ingredients = {
            { "200", "g",  "Flour" },
            { "2",   "",   "Eggs" },
            { "300", "ml", "Milk" },
            { "1",   "tbsp","Sugar" },
            { "1",   "pinch", "Salt" },
        },
        steps = "1. Whisk flour, eggs, milk, sugar and salt.\n"
             .. "2. Rest the batter 10 minutes.\n"
             .. "3. Fry small pancakes until golden.",
    },
}

-- Return all recipes as rows of { title, category, servings, minutes }.
function M.list_rows()
    local rows = {}
    for _, r in ipairs(M.recipes) do
        rows[#rows + 1] = { r.title, r.category, tostring(r.servings), tostring(r.minutes) }
    end
    return rows
end

-- Return the ingredient rows of recipe `index` as { amount, unit, name }.
function M.ingredient_rows(index)
    local r = M.recipes[index]
    if not r then return {} end
    local rows = {}
    for _, ing in ipairs(r.ingredients) do
        rows[#rows + 1] = { ing[1], ing[2], ing[3] }
    end
    return rows
end

function M.count()
    return #M.recipes
end

-- Return one recipe by 1-based index (nil if out of range).
function M.get(index)
    return M.recipes[index]
end

-- Aggregate the ingredient rows of every recipe, for the shopping list.
function M.all_ingredient_rows()
    local rows = {}
    for _, r in ipairs(M.recipes) do
        for _, ing in ipairs(r.ingredients) do
            rows[#rows + 1] = { ing[1], ing[2], ing[3], "no" }
        end
    end
    return rows
end

return M
