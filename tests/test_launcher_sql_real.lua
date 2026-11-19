-- Real MySQL integration test using credentials parsed from ~/.my.cnf.
package.path = 'tests/?.lua;../tests/?.lua;tests/support/?.lua;../tests/support/?.lua;'
    .. (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local mycnf = require('support.mycnf')
local sql = require('sql')

local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_sql_real.txt'
local group_name = os.getenv('ADMPANEL_TEST_GROUP') or 'adminpanel'
local db_name = os.getenv('ADMPANEL_TEST_DB') or 'copybox'
local table_name = os.getenv('ADMPANEL_TEST_TABLE') or 'cust'
local cnf_path = os.getenv('ADMPANEL_MYCNF') or (os.getenv('HOME') or '') .. '/.my.cnf'

local function write_result(lines)
    local f = assert(io.open(out_path, 'w'))
    for _, line in ipairs(lines) do f:write(line .. '\n') end
    f:close()
end

local groups, perr = mycnf.parse(cnf_path)
if not groups then
    write_result({ 'skip=1', 'reason=' .. tostring(perr) })
    xtappexitflag()
    return
end

local g = groups[group_name]
if not g then
    write_result({ 'skip=1', 'reason=group [' .. group_name .. '] not found' })
    xtappexitflag()
    return
end

local opts = {
    host = g.host,
    port = g.port and tonumber(g.port) or nil,
    user = g.user,
    password = g.password,
    database = db_name,
    connect_timeout = 3,
}

if not table_name:match('^[%a_][%w_]*$') then
    write_result({ 'skip=0', 'ok=false', 'err=invalid table name' })
    xtappexitflag()
    return
end
local query = 'SELECT custid, dbservid FROM ' .. table_name ..
    ' ORDER BY custid LIMIT 3'

local result, err = db_query(opts, query)
if not result then
    write_result({ 'skip=0', 'ok=false', 'err=' .. tostring(err) })
    xtappexitflag()
    return
end

local field = {
    variable = 'kk',
    sql = {
        query = query,
        display = '$custid ($dbservid)',
        key = '$custid',
    },
}
local items, map, serr = sql.load_field(field, opts, {})

-- Also exercise the app's real path: MYSQL_READ_DEFAULT_GROUP from ~/.my.cnf.
local group_result, group_err = db_query({
    group = group_name,
    database = db_name,
    connect_timeout = 3,
}, 'SELECT custid FROM ' .. table_name .. ' ORDER BY custid LIMIT 1')

local group_items, group_map, group_sql_err = sql.load_field(field, {
    group = group_name,
    database = db_name,
    connect_timeout = 3,
}, {})

local lines = {
    'skip=0',
    'ok=true',
    'columns=' .. table.concat(result.columns or {}, ','),
    'rows=' .. tostring(#(result.rows or {})),
    'first_custid=' .. tostring(result.rows and result.rows[1] and result.rows[1].custid or ''),
    'first_dbservid=' .. tostring(result.rows and result.rows[1] and result.rows[1].dbservid or ''),
    'group_ok=' .. tostring(group_result ~= nil),
    'group_err=' .. tostring(group_err),
    'group_items=' .. tostring(group_items and #group_items or 0),
    'group_first_key=' .. tostring(group_map and group_map[1] and group_map[1].key or ''),
    'group_sql_err=' .. tostring(group_sql_err),
}
if items then
    lines[#lines + 1] = 'items=' .. tostring(#items)
    lines[#lines + 1] = 'first_item=' .. tostring(items[1] or '')
    lines[#lines + 1] = 'first_key=' .. tostring(map and map[1] and map[1].key or '')
else
    lines[#lines + 1] = 'sql_error=' .. tostring(serr)
end
write_result(lines)
xtappexitflag()
