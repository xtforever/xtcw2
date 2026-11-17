-- Example --after hook for the filtered table-view probe.
-- Builds a WModel, attaches it to the table and applies a filter.
local WModel = require('wmodel')

local m = WModel.new{
    columns = {
        { title = 'Name', width = 0 },
        { title = 'Qty', width = 80, align = 2 },
    },
    rows = {
        { 'Cherry', '3' },
        { 'Apple',  '1' },
        { 'Banana', '2' },
        { 'Date',   '4' },
    },
    ids = { 'cherry', 'apple', 'banana', 'date' },
}

gui.set_model('table', m, { sort_column = 0, sort_ascending = true })
gui.set_model_filter('table', 'a')
