-- Parse a machine-readable <launcher-begin> block from a shell script.

local config = require('config')
local vars = require('vars')

local M = {}

local function trim(s)
    return (tostring(s or ''):match('^%s*(.-)%s*$'))
end

local function unquote(s)
    s = trim(s)
    local first = s:sub(1, 1)
    if (first == '"' or first == "'") and s:sub(-1) == first then
        s = s:sub(2, -2)
        s = s:gsub('\\n', '\n')
        s = s:gsub('\\t', '\t')
        s = s:gsub('\\r', '\r')
        s = s:gsub('\\"', '"')
        s = s:gsub("\\'", "'")
        s = s:gsub('\\\\', '\\')
    end
    return s
end

local function split_lines(text)
    local lines = {}
    for line in (tostring(text or '') .. '\n'):gmatch('(.-)\n') do
        lines[#lines + 1] = line
    end
    return lines
end

local function strip_comment(line)
    return line:gsub('^%s*#%s?', '')
end

function M.extract(text)
    local lines = split_lines(text)
    local start_at, end_at
    for i, line in ipairs(lines) do
        local s = trim(line)
        if s:match('^#%s*<launcher%-begin>') then start_at = i end
        if s:match('^#%s*<launcher%-end>') then end_at = i end
    end
    if not start_at or not end_at or end_at <= start_at then
        return nil, 'missing <launcher-begin>/<launcher-end> block'
    end
    local out = {}
    for i = start_at + 1, end_at - 1 do
        out[#out + 1] = strip_comment(lines[i])
    end
    return table.concat(out, '\n')
end

local function parse_arg(lines, start_index)
    local line = trim(lines[start_index])
    local tokens = {}
    for token in line:gmatch('%S+') do tokens[#tokens + 1] = token end
    if #tokens == 0 or tokens[#tokens] ~= '(' then
        return nil, start_index, 'argument line must end with "(": ' .. line
    end

    local field = {
        type = 'input',
        mandatory = false,
        widget_type = 'text',
        default = '',
    }
    for i = 1, #tokens - 1 do
        local token = tokens[i]
        if token:match('^%-%-') then
            field.long = token
        elseif token:match('^%-') then
            field.short = token
        elseif token:match('^%$') then
            field.variable = token:sub(2)
        end
    end
    if not field.variable then
        local base = field.long or field.short
        if not base then
            return nil, start_index, 'argument has no option or variable: ' .. line
        end
        field.variable = base:gsub('^%-+', ''):gsub('%-', '_')
    end
    field.arg = field.long or field.short
    field.flag = field.long or field.short

    local i = start_index + 1
    while i <= #lines do
        local s = trim(lines[i])
        if s == ')' then
            i = i + 1
            break
        end
        local key, value = s:match('^([%w_%-]+)%s*:%s*(.*)$')
        if key then
            if key == 'type' then
                local typ, requirement = value:match('^(%S+)%s*(%S*)')
                field.type = typ or 'string'
                if requirement == 'mandatory' then
                    field.mandatory = true
                elseif requirement == 'optional' then
                    field.mandatory = false
                end
            elseif key == 'widget-type' or key == 'widget_type' then
                field.widget_type = value:match('^(%S+)') or value
            elseif key == 'default' then
                field.default = unquote(value)
            elseif key == 'widget-label' or key == 'widget_label' then
                field.title = unquote(value)
            elseif key == 'sql_query_str' then
                field.sql_query_str = unquote(value)
            elseif key == 'sql_disp_field' then
                field.sql_disp_field = unquote(value)
            elseif key == 'sql_return_string' or key == 'sql_key_field' then
                field.sql_return_string = unquote(value)
            elseif key == 'sql_host' or key == 'sql_port' or key == 'sql_user' or
                   key == 'sql_pwd' or key == 'sql_db' or key == 'sql_group' then
                field[key] = unquote(value)
            else
                field.extra = field.extra or {}
                field.extra[key] = unquote(value)
            end
        end
        i = i + 1
    end

    if field.type == 'string' then
        field.type = 'input'
    elseif field.type == 'switch' then
        field.on = field.flag
        field.off = ''
        field.default = field.default or ''
        if field.widget_type == 'text' then field.widget_type = 'checkbox' end
    elseif field.type ~= 'input' then
        return nil, i, 'unsupported argument type: ' .. tostring(field.type)
    end

    if field.widget_type == 'sql-query' then
        field.type = 'sql'
    end
    if field.widget_type == 'none' and (field.default == nil or field.default == '') then
        return nil, i, 'hidden argument "' .. field.variable .. '" requires a default'
    end
    if not field.title then field.title = field.variable end

    return field, i
end

function M.parse_block(block, script_path)
    local cfg = {
        title = 'Launcher',
        script = script_path or '',
        command_mode = 'args',
        fields = {},
    }
    local lines = split_lines(block)
    local i = 1
    while i <= #lines do
        local line = trim(lines[i])
        if line == '' then
            i = i + 1
        elseif line:match('^title%s*:') then
            cfg.title = unquote(line:match('^title%s*:%s*(.*)$'))
            i = i + 1
        elseif line:match('^version%s*:') then
            cfg.version = unquote(line:match('^version%s*:%s*(.*)$'))
            i = i + 1
        elseif line:match('^args%s*:') then
            i = i + 1
        elseif line:match('^%-') then
            local field, next_i, err = parse_arg(lines, i)
            if not field then return nil, err end
            cfg.fields[#cfg.fields + 1] = field
            i = next_i
        else
            i = i + 1
        end
    end
    if #cfg.fields == 0 then
        return nil, 'no arguments found in launcher block'
    end
    return cfg
end

function M.load(path)
    local f, err = io.open(path, 'r')
    if not f then return nil, err end
    local text = f:read('*a')
    f:close()
    local block, berr = M.extract(text)
    if not block then return nil, berr end
    local cfg, perr = M.parse_block(block, path)
    if not cfg then return nil, perr end
    return config.normalize(cfg)
end

function M.build_command(cfg, values)
    local parts = { vars.shell_quote(cfg.script) }
    for _, field in ipairs(cfg.fields) do
        local flag = field.long or field.short or field.flag
        if field.type == 'switch' then
            local v = values[field.variable]
            if v == field.on or v == true or v == 'true' or v == '1' then
                parts[#parts + 1] = vars.shell_quote(flag)
            end
        else
            local v = values[field.variable]
            if v == nil or v == '' then v = field.default end
            if v ~= nil and v ~= '' then
                parts[#parts + 1] = vars.shell_quote(flag)
                parts[#parts + 1] = vars.shell_quote(v)
            end
        end
    end
    return table.concat(parts, ' ')
end

return M
