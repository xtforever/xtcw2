-- Named variable store helpers and shell-safe expansion.
--
-- Values are plain strings/numbers/booleans. Arrays are 0-based for legacy
-- compatibility: $arr[0] reads table index 1, $arr[1] reads table index 2.

local M = {}

local function tostr(v)
    if v == nil then return nil end
    local t = type(v)
    if t == 'string' then return v end
    if t == 'number' then return tostring(v) end
    if t == 'boolean' then return v and 'true' or 'false' end
    return nil
end

local function lookup(scope, name, index)
    if type(scope) ~= 'table' then return nil end
    local v = scope[name]
    if v == nil then return nil end
    if index ~= nil then
        if type(v) ~= 'table' then return nil end
        return tostr(v[index + 1])
    end
    if type(v) == 'table' then return nil end
    return tostr(v)
end

-- Expand $name, ${name}, $name[index] and $$.
-- Returns text, missing_names. Unknown names expand to "" and are reported.
-- With opts.quote_values, substituted values are shell-quoted.
function M.expand(template, scope, opts)
    template = tostring(template or '')
    opts = opts or {}
    local out = {}
    local missing = {}
    local i = 1
    local n = #template

    while i <= n do
        local c = template:sub(i, i)
        if c ~= '$' then
            out[#out + 1] = c
            i = i + 1
        else
            local nextc = template:sub(i + 1, i + 1)
            if nextc == '$' then
                out[#out + 1] = '$'
                i = i + 2
            elseif nextc == '{' then
                local close = template:find('}', i + 2, true)
                if close then
                    local name = template:sub(i + 2, close - 1)
                    local val = lookup(scope, name, nil)
                    if val == nil then
                        missing[#missing + 1] = name
                        val = ''
                    end
                    if opts.quote_values then val = M.shell_quote(val) end
                    out[#out + 1] = val
                    i = close + 1
                else
                    out[#out + 1] = c
                    i = i + 1
                end
            else
                local name, after = template:match('^([%a_][%w_]*)()', i + 1)
                local idx
                if name and template:sub(after, after) == '[' then
                    local digits, after_bracket =
                        template:match('^%[([%d]+)%]()', after)
                    if digits then
                        idx = tonumber(digits)
                        after = after_bracket
                    end
                end
                if name then
                    local val = lookup(scope, name, idx)
                    if val == nil then
                        missing[#missing + 1] =
                            idx and (name .. '[' .. tostring(idx) .. ']') or name
                        val = ''
                    end
                    if opts.quote_values then val = M.shell_quote(val) end
                    out[#out + 1] = val
                    i = after
                else
                    out[#out + 1] = c
                    i = i + 1
                end
            end
        end
    end

    return table.concat(out), missing
end

function M.expand_strict(template, scope, opts)
    local text, missing = M.expand(template, scope, opts)
    if #missing > 0 then
        return nil, 'missing variables: ' .. table.concat(missing, ', ')
    end
    return text
end

-- POSIX-shell single-quote a value.
function M.shell_quote(value)
    local s = tostring(value or '')
    if s == '' then return "''" end
    if s:match('^[%w_@%%%+=:,./-]+$') then return s end
    return "'" .. s:gsub("'", "'\\''") .. "'"
end

-- Split a command line into execvp-style argv. Handles single/double quotes
-- and backslash escapes. Returns argv or nil, error.
function M.tokenize(command)
    command = tostring(command or '')
    local args = {}
    local cur = {}
    local quote = nil
    local i = 1
    local n = #command

    local function push()
        if #cur > 0 then
            args[#args + 1] = table.concat(cur)
            cur = {}
        end
    end

    while i <= n do
        local c = command:sub(i, i)
        if quote == "'" then
            if c == "'" then quote = nil else cur[#cur + 1] = c end
            i = i + 1
        elseif quote == '"' then
            if c == '"' then
                quote = nil
                i = i + 1
            elseif c == '\\' then
                local nx = command:sub(i + 1, i + 1)
                if nx ~= '' then cur[#cur + 1] = nx; i = i + 1 end
                i = i + 1
            else
                cur[#cur + 1] = c
                i = i + 1
            end
        else
            if c == "'" or c == '"' then
                quote = c
                i = i + 1
            elseif c == '\\' then
                local nx = command:sub(i + 1, i + 1)
                if nx ~= '' then cur[#cur + 1] = nx; i = i + 1 end
                i = i + 1
            elseif c:match('%s') then
                push()
                i = i + 1
            else
                cur[#cur + 1] = c
                i = i + 1
            end
        end
    end

    if quote then return nil, 'unterminated quote' end
    push()
    return args
end

-- Build an execvp argv from a template and variable scope.
function M.build_argv(template, scope)
    local text, missing = M.expand(template, scope, { quote_values = true })
    if #missing > 0 then
        return nil, 'missing variables: ' .. table.concat(missing, ', ')
    end
    return M.tokenize(text)
end

return M
