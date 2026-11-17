-- Example --after hook for tests/widgets/wtreeview.lui.
gui.set_string_list('tree', 'nodes', {
    '0\tRecipes', '1\tItalian', '2\tSpaghetti', '1\tDessert', '2\tCake',
    '0\tShopping', '1\tGroceries',
})
local w = gui.get_widget('tree')
if w then
    xtaction(w, 'select_node', 0)
    xtaction(w, 'expand')
end
