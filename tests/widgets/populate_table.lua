-- Example --after hook for tests/widgets/wtableview.lui.
gui.set_string_list('table', 'rows', {
    'Cherry\t3', 'Apple\t1', 'Banana\t2', 'Date\t4',
})
local w = gui.get_widget('table')
if w then xtaction(w, 'sort') end
