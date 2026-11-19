-- Unit tests for apps/admpanel/config.lua
package.path = 'apps/admpanel/?.lua;' .. package.path
local config = require('config')

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

local n, err = config.normalize({
    title = 'Kundenauswahl',
    script = 'auswahl.sh $kk',
    fields = {
        { type = 'kuerzel', title = 'Kuerzel', variable = 'kk',
          extra_vars = {
              sql_query_str = 'select * from custdb',
              sql_disp_field = '$custid ($dbservid)',
              sql_key_field = '$custid',
          } },
        { type = 'option', title = 'sandbox', variable = 'sandbox',
          values = { 1, 2, 3, 4, 5, 6, 7 } },
        { type = 'switch', title = 'Force', variable = 'force', on = '-f' },
        { type = 'input', title = 'Domain', variable = 'domain' },
    },
})
ok(n, err)
eq(n.title, 'Kundenauswahl', 'title')
eq(n.script, 'auswahl.sh $kk', 'script')
eq(#n.fields, 4, 'field count')
eq(n.fields[1].type, 'sql', 'legacy kuerzel becomes sql')
eq(n.fields[1].sql.query, 'select * from custdb', 'sql query')
eq(n.fields[1].sql.display, '$custid ($dbservid)', 'sql display')
eq(n.fields[1].sql.key, '$custid', 'sql key')
eq(n.fields[2].type, 'option', 'option type')
eq(n.fields[2].values[1], '1', 'option value normalized to string')
eq(n.fields[2].default, '1', 'option default is first value')
eq(n.fields[3].type, 'switch', 'switch type')
eq(n.fields[3].on, '-f', 'switch on')
eq(n.fields[3].off, '', 'switch off default')
eq(n.fields[4].type, 'input', 'input type')
eq(n.fields[4].default, '', 'input default')
eq(n.sql.group, 'adminpanel', 'default my.cnf group')

-- Legacy option string
n, err = config.normalize({
    script = 'x',
    fields = { { type = 'option', variable = 'o', values = 'a, b, c' } },
})
ok(n, err)
eq(#n.fields[1].values, 3, 'csv option count')
eq(n.fields[1].values[2], 'b', 'csv option trimmed')

-- Hidden mandatory field needs a default
local bad, berr = config.normalize({
    script = 'x',
    fields = { { type = 'input', variable = 'hidden', widget_type = 'none' } },
})
ok(not bad, 'hidden without default rejected')
ok(berr and berr:find('default'), 'hidden error mentions default')

-- Hidden field with default is accepted
n, err = config.normalize({
    script = 'x',
    fields = { { type = 'input', variable = 'hidden',
                 widget_type = 'none', default = '/tmp/log' } },
})
ok(n, err)
eq(n.fields[1].hidden, true, 'hidden flag')
eq(n.fields[1].default, '/tmp/log', 'hidden default')

-- Missing script
bad, berr = config.normalize({ fields = {} })
ok(not bad, 'missing script rejected')
ok(berr and berr:find('script'), 'missing script error')

-- Invalid variable name
bad, berr = config.normalize({
    script = 'x',
    fields = { { type = 'input', variable = 'bad-name' } },
})
ok(not bad, 'invalid variable rejected')
ok(berr and berr:find('variable'), 'invalid variable error')

-- SQL field without query/display/key
bad, berr = config.normalize({
    script = 'x',
    fields = { { type = 'sql', variable = 'k' } },
})
ok(not bad, 'incomplete sql field rejected')
ok(berr and berr:find('sql'), 'incomplete sql error')

-- Load a Lua config file
n, err = config.load('apps/admpanel/examples/kunde-auswahl.lua')
ok(n, err)
eq(n.title, 'Kundenauswahl', 'loaded title')
eq(#n.fields, 1, 'loaded field count')
eq(n.fields[1].sql.key, '$custid', 'loaded sql key')

-- Load freigabe example
n, err = config.load('apps/admpanel/examples/freigabe.lua')
ok(n, err)
eq(n.title, 'freigabe erstellen', 'freigabe title')
eq(#n.fields, 4, 'freigabe field count')

print('test_launcher_config: OK')
