-- Headless probe that the mandatory db_query binding is registered and reports
-- connection errors without crashing.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_db.txt'
local result, err = db_query({
    group = 'definitely_missing_adminpanel_group',
    host = '127.0.0.1',
    port = 1,
    user = 'nobody',
    password = 'nobody',
    database = 'nodb',
    connect_timeout = 1,
}, 'select 1')

local f = assert(io.open(out_path, 'w'))
f:write('result=' .. tostring(result) .. '\n')
f:write('err=' .. tostring(err) .. '\n')
f:write('has_error=' .. tostring(err ~= nil and #tostring(err) > 0) .. '\n')
f:close()
xtappexitflag()
