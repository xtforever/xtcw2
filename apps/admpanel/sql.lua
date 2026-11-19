-- SQL lookup helpers for the AdmPanel launcher.

local vars = require('vars')

local M = {
    query_fn = nil,
}

-- Test hook: override the C db_query binding.
function M.set_query_fn(fn)
    M.query_fn = fn
end

local function merge_scope(scope, row)
    local out = {}
    for k, v in pairs(scope or {}) do out[k] = v end
    for k, v in pairs(row or {}) do out[k] = v end
    return out
end

local function connection_opts(conn, field_sql)
    local opts = {}
    for k, v in pairs(conn or {}) do opts[k] = v end
    local keys = {
        'group', 'host', 'port', 'user', 'password', 'database',
        'socket', 'connect_timeout', 'read_timeout',
    }
    for _, key in ipairs(keys) do
        if field_sql[key] ~= nil then opts[key] = field_sql[key] end
    end
    return opts
end

-- Run a SQL field's query and build the combo items plus the display->key map.
-- Returns items, map, err.
function M.load_field(field, conn, scope)
    if not field or not field.sql then
        return nil, nil, 'not a SQL field'
    end

    local query, missing = vars.expand(field.sql.query, scope or {}, {})
    if #missing > 0 then
        return nil, nil, 'missing SQL variables: ' .. table.concat(missing, ', ')
    end

    local fn = M.query_fn or _G.db_query
    if type(fn) ~= 'function' then
        return nil, nil, 'db_query binding is not available'
    end

    local result, err = fn(connection_opts(conn, field.sql), query)
    if not result then
        return nil, nil, err or 'SQL query failed'
    end

    local items = {}
    local map = {}
    for i, row in ipairs(result.rows or {}) do
        local row_scope = merge_scope(scope, row)
        local display = vars.expand(field.sql.display, row_scope, {})
        local key = vars.expand(field.sql.key, row_scope, {})
        if display == '' then display = key end
        items[i] = display
        map[i] = { key = key, display = display, row = row }
    end

    return items, map, nil
end

return M
