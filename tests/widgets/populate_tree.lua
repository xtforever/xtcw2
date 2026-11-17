-- Populate the WTreeView probe with enough nodes to exercise scrolling.
-- 40 flat nodes are created; selecting row 20 scrolls it into view.
local nodes = {}
for i = 0, 39 do
    nodes[#nodes + 1] = '0\tRow ' .. i
end
gui.set_string_list('tree', 'nodes', nodes)
local w = gui.get_widget('tree')
if w then
    xtaction(w, 'select_node', 20)
end
