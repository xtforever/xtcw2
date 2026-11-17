-- tests/test_model.lua
-- Pure-Lua tests for lui/wmodel.lua (no X server required).
local ROOT = (os.getenv('XTCW_ROOT') or '.')
package.path = ROOT .. '/lui/?.lua;' .. package.path

local WModel = require('wmodel')

local pass, fail = 0, 0
local function ok(name)
    pass = pass + 1
    print('ok - ' .. name)
end
local function bad(name, detail)
    fail = fail + 1
    print('FAIL - ' .. name .. (detail and (': ' .. tostring(detail)) or ''))
end
local function eq(name, got, want)
    if got == want then ok(name) else bad(name, 'got ' .. tostring(got) ..
                                            ', want ' .. tostring(want)) end
end

-- Construction, get/set, ids -------------------------------------------------
local m = WModel.new{
    columns = { { title = 'Name' }, { title = 'Qty', align = 2 } },
    rows = {
        { 'Cherry', '3' },
        { 'Apple',  '1' },
        { 'Banana', '2' },
        { 'date',   '10' },
    },
    ids = { 10, 20, 30, 40 },
}
eq('count', m:count(), 4)
eq('column_count', m:column_count(), 2)
eq('get(2,1)', m:get(2, 1), 'Apple')
eq('get(4,2)', m:get(4, 2), '10')
eq('id_at(3)', m:id_at(3), 30)
eq('index_of_id(20)', m:index_of_id(20), 2)
eq('index_of_id(missing)', m:index_of_id(99), nil)
eq('to_rows', m:to_rows()[2], 'Apple\t1')

m:set(1, 2, '7')
eq('set()', m:get(1, 2), '7')

-- Generated ids --------------------------------------------------------------
local g = WModel.new{ rows = { { 'a' }, { 'b' } } }
eq('generated id 1', g:id_at(1), 1)
eq('generated id 2', g:id_at(2), 2)
g:add({ 'c' })
eq('add() generated id', g:id_at(3), 3)
g:remove(2)
eq('remove() count', g:count(), 2)
eq('remove() keeps ids', g:id_at(2), 3)

-- Sorting --------------------------------------------------------------------
m:sort(1, true)
eq('sort asc text (case-insensitive)', m:to_rows()[1], 'Apple\t1')
eq('sort asc ids follow', m:id_at(1), 20)
eq('sort asc last', m:to_rows()[4], 'date\t10')
eq('sort asc last id', m:id_at(4), 40)

m:sort(1, false)
eq('sort desc first', m:to_rows()[1], 'date\t10')

m:sort(2, true)
eq('sort numeric asc', m:to_rows()[1], 'Apple\t1')
eq('sort numeric asc last', m:to_rows()[4], 'date\t10')

m:sort(2, false)
eq('sort numeric desc', m:to_rows()[1], 'date\t10')

-- Stable sort: equal keys keep the original order.
local s = WModel.new{
    rows = { { 'b', '1' }, { 'a', '1' }, { 'c', '1' } },
    ids = { 'b', 'a', 'c' },
}
s:sort(2, true)
eq('stable sort first', s:id_at(1), 'b')
eq('stable sort last', s:id_at(3), 'c')

-- Filtering ------------------------------------------------------------------
local f = WModel.new{
    columns = { { title = 'Name' }, { title = 'Category' } },
    rows = {
        { 'Spaghetti', 'Main' },
        { 'Tomato Soup', 'Starter' },
        { 'Apple Crumble', 'Dessert' },
        { 'Greek Salad', 'Starter' },
    },
    ids = { 1, 2, 3, 4 },
}
eq('match all empty filter', #f:match(''), 4)
eq('match all columns', #f:match('s'), 4)
eq('match column 1', #f:match('a', { 1 }), 4)
eq('match column 2', #f:match('starter', { 2 }), 2)
eq('match case-insensitive', #f:match('APPLE'), 1)
eq('match no hit', #f:match('zzz'), 0)
eq('match tab-safe', #f:match('\t'), 0)

-- Proxy ----------------------------------------------------------------------
local p = WModel.proxy(f)
eq('proxy initial count', p:count(), 4)
p:set_filter('starter')
eq('proxy filter count', p:count(), 2)
eq('proxy filtered ids', p:ids()[1], 2)
eq('proxy view_of_id visible', p:view_of_id(4), 2)
eq('proxy view_of_id hidden', p:view_of_id(1), nil)

p:set_sort(1, true)
eq('proxy sorted first', p:rows()[1], 'Greek Salad\tStarter')
eq('proxy source_index maps to source', f:get(p:source_index(1), 1), 'Greek Salad')

p:set_filter('')
eq('proxy filter cleared', p:count(), 4)
eq('proxy still sorted', p:rows()[1], 'Apple Crumble\tDessert')

p:set_filter('tomato')
eq('proxy narrow filter', p:count(), 1)
eq('proxy narrow row', p:rows()[1], 'Tomato Soup\tStarter')

-- Source replacement keeps filter/sort state.
local other = WModel.new{
    columns = { { title = 'Name' }, { title = 'Category' } },
    rows = { { 'Tomato Toast', 'Breakfast' }, { 'Soup', 'Starter' } },
    ids = { 7, 8 },
}
p:set_source(other)
eq('proxy source swap count', p:count(), 1)
eq('proxy source swap id', p:ids()[1], 7)

print(string.format('%d passed, %d failed', pass, fail))
os.exit(fail == 0 and 0 or 1)
