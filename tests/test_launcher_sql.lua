-- Unit tests for apps/admpanel/sql.lua (query binding injected).
package.path = 'apps/admpanel/?.lua;' .. package.path
local sql = require('sql')

local function fail(msg)
    io.stderr:write('FAIL: ' .. msg .. '\n')
    os.exit(1)
end
local function eq(actual, expected, msg)
    if actual ~= expected then
        fail((msg or 'value') .. ': expected ' .. tostring(expected) ..
             ', got ' .. tostring(actual))
    end
end
local function ok(cond, msg)
    if not cond then fail(msg or 'condition failed') end
end

local seen_opts, seen_query
sql.set_query_fn(function(opts, query)
    seen_opts = opts
    seen_query = query
    return {
        columns = { 'custid', 'dbservid' },
        rows = {
            { custid = 'jens', dbservid = 'db1' },
            { custid = 'anna', dbservid = 'db2' },
        },
    }
end)

local field = {
    variable = 'kk',
    sql = {
        query = 'select * from cust where srv="$srv"',
        display = '$custid ($dbservid)',
        key = '$custid',
    },
}
local items, map, err = sql.load_field(field, { group = 'adminpanel' }, { srv = 'sandbox7' })
ok(items, err)
eq(seen_opts.group, 'adminpanel', 'connection group')
eq(seen_query, 'select * from cust where srv="sandbox7"', 'query expansion')
eq(#items, 2, 'item count')
eq(items[1], 'jens (db1)', 'display 1')
eq(items[2], 'anna (db2)', 'display 2')
eq(map[1].key, 'jens', 'key 1')
eq(map[2].key, 'anna', 'key 2')
eq(map[1].row.dbservid, 'db1', 'row retained')

-- Field-level connection overrides
sql.set_query_fn(function(opts, query)
    seen_opts = opts
    return { rows = {} }
end)
items, map, err = sql.load_field({
    variable = 'k',
    sql = { query = 'select 1', display = '$x', key = '$x', database = 'other' },
}, { group = 'adminpanel', database = 'copybox' }, {})
ok(items, err)
eq(seen_opts.database, 'other', 'field database override')
eq(seen_opts.group, 'adminpanel', 'group inherited')

-- Empty display falls back to key
sql.set_query_fn(function()
    return { rows = { { x = 'abc' } } }
end)
items, map, err = sql.load_field({
    variable = 'k',
    sql = { query = 'select 1', display = '', key = '$x' },
}, {}, {})
ok(items, err)
eq(items[1], 'abc', 'display fallback')

-- Query failure is propagated
sql.set_query_fn(function() return nil, 'access denied' end)
items, map, err = sql.load_field(field, {}, { srv = 's' })
ok(not items, 'query failure rejected')
ok(err and err:find('access denied'), 'query error text')

-- Missing query variable is reported
sql.set_query_fn(function() error('should not run') end)
items, map, err = sql.load_field({
    variable = 'k',
    sql = { query = 'select $missing', display = '$x', key = '$x' },
}, {}, {})
ok(not items, 'missing query var rejected')
ok(err and err:find('missing'), 'missing query var text')

-- Missing binding is reported
sql.set_query_fn(nil)
_G.db_query = nil
items, map, err = sql.load_field({
    variable = 'k',
    sql = { query = 'select 1', display = '$x', key = '$x' },
}, {}, {})
ok(not items, 'missing binding rejected')
ok(err and err:find('db_query'), 'missing binding text')

print('test_launcher_sql: OK')
