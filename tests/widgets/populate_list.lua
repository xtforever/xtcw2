-- Example --after hook for tests/widgets/wlistview.lui.
-- Run with:
--   ./run_widget.sh --after tests/widgets/populate_list.lua tests/widgets/wlistview.lui

local rows = {}
for i = 1, 60 do
    rows[#rows + 1] = string.format("row_%03d\t%d KB\tfile", i, i * 3)
end
gui.set_string_list("lv", "rows", rows)
