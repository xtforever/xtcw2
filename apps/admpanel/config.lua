-- Lua launcher config loader and normalizer.

local M = {}

local function trim(s)
    return (tostring(s or ''):match('^%s*(.-)%s*$'))
end

local function split_csv(s)
    local out = {}
    for part in tostring(s or ''):gmatch('[^,]+') do
        out[#out + 1] = trim(part)
    end
    return out
end

local function valid_variable(name)
    return type(name) == 'string' and name:match('^[%a_][%w_]*$') ~= nil
end

local function normalize_type(t)
    t = tostring(t or 'input'):lower()
    if t == 'kuerzel' or t == 'query' or t == 'sql-query' then return 'sql' end
    if t == 'string' or t == 'text' or t == 'input' then return 'input' end
    if t == 'option' or t == 'enum' or t == 'dropdown' then return 'option' end
    if t == 'switch' or t == 'checkbox' or t == 'bool' or t == 'boolean' then return 'switch' end
    if t == 'sql' then return 'sql' end
    return nil
end

local function default_widget(typ)
    if typ == 'option' then return 'dropdown' end
    if typ == 'switch' then return 'checkbox' end
    if typ == 'sql' then return 'sql-query' end
    return 'text'
end

local function normalize_widget_type(w, typ)
    w = tostring(w or default_widget(typ)):lower()
    if w == 'lookup/dropdown' or w == 'lookup' or w == 'sql-query' then
        return 'sql-query'
    end
    if w == 'text' or w == 'entry' or w == 'input' then return 'text' end
    if w == 'checkbox' or w == 'switch' then return 'checkbox' end
    if w == 'dropdown' or w == 'combo' or w == 'combo-box' then return 'dropdown' end
    if w == 'none' or w == 'hidden' then return 'none' end
    return w
end

local function pick(src, extra, field, ...)
    local keys = { ... }
    for _, key in ipairs(keys) do
        if src and src[key] ~= nil then return src[key] end
    end
    for _, key in ipairs(keys) do
        if extra and extra[key] ~= nil then return extra[key] end
    end
    for _, key in ipairs(keys) do
        if field and field[key] ~= nil then return field[key] end
    end
    return nil
end

local function normalize_field_sql(field, variable, default_sql)
    local src = {}
    if type(default_sql) == 'table' then
        for k, v in pairs(default_sql) do src[k] = v end
    end
    if type(field.sql) == 'table' then
        for k, v in pairs(field.sql) do src[k] = v end
    end
    local extra = type(field.extra_vars) == 'table' and field.extra_vars or {}

    local query = pick(src, extra, field, 'query', 'sql_query_str')
    local display = pick(src, extra, field, 'display', 'sql_disp_field')
    local key = pick(src, extra, field, 'key', 'sql_key_field', 'sql_return_string')
    if not query or not display or not key then
        return nil, 'sql field "' .. tostring(variable) ..
            '" requires query, display and key'
    end

    local out = {
        query = tostring(query),
        display = tostring(display),
        key = tostring(key),
    }
    out.host = pick(src, extra, field, 'host', 'sql_host')
    out.port = pick(src, extra, field, 'port', 'sql_port')
    out.user = pick(src, extra, field, 'user', 'sql_user')
    out.password = pick(src, extra, field, 'password', 'sql_pwd', 'sql_password')
    out.database = pick(src, extra, field, 'database', 'sql_db')
    out.socket = pick(src, extra, field, 'socket', 'sql_socket')
    out.group = pick(src, extra, field, 'group', 'sql_group')
    out.connect_timeout = pick(src, extra, field, 'connect_timeout', 'sql_connect_timeout')
    return out
end

local function normalize_field(field, index, defaults)
    if type(field) ~= 'table' then
        return nil, 'field ' .. index .. ' must be a table'
    end

    local variable = field.variable
    if not valid_variable(variable) then
        return nil, 'field ' .. index .. ' has an invalid or missing variable name'
    end

    local typ = normalize_type(field.type)
    if not typ then
        return nil, 'field "' .. variable .. '" has an unknown type: ' ..
            tostring(field.type)
    end

    local nf = {
        index = index,
        type = typ,
        variable = variable,
        title = tostring(field.title or field.widget_label or
                         field['widget-label'] or variable),
        widget_type = normalize_widget_type(
            field.widget_type or field['widget-type'], typ),
        mandatory = field.mandatory == true or field.optional == false,
        arg = field.arg,
        short = field.short,
        long = field.long,
        flag = field.flag,
    }
    nf.hidden = nf.widget_type == 'none'
    if nf.hidden then nf.mandatory = true end

    if typ == 'input' then
        nf.default = field.default ~= nil and tostring(field.default) or ''
    elseif typ == 'option' then
        local values = field.values
        if values == nil and field.option ~= nil then values = field.option end
        if type(values) == 'string' then values = split_csv(values) end
        if type(values) ~= 'table' or #values == 0 then
            return nil, 'option field "' .. variable .. '" requires values'
        end
        nf.values = {}
        for _, v in ipairs(values) do
            nf.values[#nf.values + 1] = tostring(v)
        end
        nf.default = field.default ~= nil and tostring(field.default) or nf.values[1]
        local found = false
        for _, v in ipairs(nf.values) do
            if v == nf.default then found = true; break end
        end
        if not found then
            return nil, 'option field "' .. variable ..
                '" default is not one of its values'
        end
    elseif typ == 'switch' then
        local on, off
        if field.switch ~= nil then
            local parts = split_csv(field.switch)
            on = parts[1]
            off = parts[2] or ''
        end
        if field.on ~= nil then on = field.on end
        if field['on'] ~= nil then on = field['on'] end
        if field.flag ~= nil then on = field.flag end
        if on == nil then
            return nil, 'switch field "' .. variable .. '" requires on/flag'
        end
        nf.on = tostring(on)
        nf.off = field.off ~= nil and tostring(field.off) or (off or '')
        nf.flag = tostring(field.flag or nf.on)
        nf.default = field.default ~= nil and tostring(field.default) or nf.off
    elseif typ == 'sql' then
        local sql, err = normalize_field_sql(field, variable,
                                            defaults and defaults.sql)
        if not sql then return nil, err end
        nf.sql = sql
        nf.default = field.default ~= nil and tostring(field.default) or ''
    end

    if nf.hidden and (nf.default == nil or nf.default == '') then
        return nil, 'hidden field "' .. variable .. '" requires a default'
    end

    return nf
end

local function normalize_connection(cfg)
    local out = {}
    if type(cfg.sql) == 'table' then
        for k, v in pairs(cfg.sql) do out[k] = v end
    end
    out.default = nil
    out.group = out.group or cfg.sql_group or 'adminpanel'
    return out
end

function M.normalize(cfg)
    if type(cfg) ~= 'table' then
        return nil, 'config must return a table'
    end
    if type(cfg.script) ~= 'string' or cfg.script == '' then
        return nil, 'config.script is required'
    end

    local defaults = {
        sql = (type(cfg.sql) == 'table' and cfg.sql.default) or cfg.sql_default,
    }

    local out = {
        title = tostring(cfg.title or 'Launcher'),
        script = cfg.script,
        shell = cfg.shell == true,
        command_mode = cfg.command_mode or 'template',
        cwd = cfg.cwd,
        init = cfg.init,
        sql = normalize_connection(cfg),
        fields = {},
    }

    local fields = cfg.fields or {}
    if type(fields) ~= 'table' then
        return nil, 'config.fields must be an array'
    end
    for i, field in ipairs(fields) do
        local nf, err = normalize_field(field, i, defaults)
        if not nf then return nil, err end
        out.fields[#out.fields + 1] = nf
    end

    return out
end

function M.load(path)
    if type(path) ~= 'string' or path == '' then
        return nil, 'config path is required'
    end

    if path:match('%.json$') then
        local ok, njson = pcall(require, 'compat.njson')
        if not ok or type(njson) ~= 'table' or type(njson.parse_file) ~= 'function' then
            return nil, 'legacy .json configs are not supported here; convert to Lua: ' .. path
        end
        local data, err = njson.parse_file(path)
        if not data then return nil, err end
        return M.normalize(data)
    end

    local chunk, err = loadfile(path)
    if not chunk then return nil, err end
    local ok, data = pcall(chunk)
    if not ok then return nil, data end
    if type(data) ~= 'table' then
        return nil, 'config must return a table: ' .. path
    end
    return M.normalize(data)
end

return M
