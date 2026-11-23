-- Minimal HDF (Human Data Forms) reader for the generated widget docs.
--
-- Supports the core language plus the structural `seq` array convention and
-- ignores `rem` comment forms.  wbuild emits plain HDF (no def/for/fold), so
-- the optional macro/fold layers are intentionally not implemented here.
--
-- API:
--   local doc = hdf.load(path)       -- AST
--   local data = hdf.to_data(doc)    -- Lua tables (seq -> array, list -> object)
--   local data = hdf.parse_text(text)

local M = {}

local function is_space(c)
    return c == " " or c == "\t" or c == "\n" or c == "\r"
end

local function is_kw_start(c)
    return c:match("[A-Za-z_]") ~= nil
end

local function is_kw_char(c)
    return c:match("[A-Za-z0-9_%./:%-]") ~= nil
end

local function new_parser(text)
    local p = 1
    local n = #text

    local function skip()
        while p <= n do
            local c = text:sub(p, p)
            if is_space(c) then
                p = p + 1
            elseif c == ";" then
                while p <= n and text:sub(p, p) ~= "\n" do p = p + 1 end
            else
                break
            end
        end
    end

    local function read_raw()
        local start = p
        p = p + 1
        local eq = 0
        while p <= n and text:sub(p, p) == "=" do
            eq = eq + 1
            p = p + 1
        end
        local double = text:sub(p, p) == "["
        if double then p = p + 1 end
        local close = "]" .. string.rep("=", eq) .. "]"
        local e = text:find(close, p, true)
        if not e then error("hdf: unterminated raw string at " .. start) end
        local body = text:sub(p, e - 1)
        p = e + #close
        return { kind = "raw", text = body }
    end

    local function read_quoted()
        local start = p
        p = p + 1
        local out = {}
        while true do
            local c = text:sub(p, p)
            if c == "" then error("hdf: unterminated string at " .. start) end
            if c == '"' then p = p + 1; break end
            if c == "\n" then error("hdf: newline in quoted string at " .. start) end
            if c ~= "\\" then
                out[#out + 1] = c
                p = p + 1
            else
                p = p + 1
                local e = text:sub(p, p)
                if e == '"' or e == "\\" then out[#out + 1] = e; p = p + 1
                elseif e == "n" then out[#out + 1] = "\n"; p = p + 1
                elseif e == "r" then out[#out + 1] = "\r"; p = p + 1
                elseif e == "t" then out[#out + 1] = "\t"; p = p + 1
                elseif e == "0" then out[#out + 1] = "\0"; p = p + 1
                elseif e == "x" then
                    local h = text:sub(p + 1, p + 2)
                    if not h:match("^%x%x$") then error("hdf: bad \\x escape") end
                    out[#out + 1] = string.char(tonumber(h, 16)); p = p + 3
                elseif e == "u" then
                    local h = text:sub(p + 1, p + 4)
                    if not h:match("^%x%x%x%x$") then error("hdf: bad \\u escape") end
                    out[#out + 1] = utf8.char(tonumber(h, 16)); p = p + 5
                elseif e == "U" then
                    local h = text:sub(p + 1, p + 8)
                    if not h:match("^%x%x%x%x%x%x%x%x$") then error("hdf: bad \\U escape") end
                    out[#out + 1] = utf8.char(tonumber(h, 16)); p = p + 9
                else
                    error("hdf: unknown escape \\" .. e)
                end
            end
        end
        return { kind = "str", text = table.concat(out) }
    end

    local function read_number()
        local start = p
        if text:sub(p, p) == "-" then p = p + 1 end
        while p <= n and text:sub(p, p):match("%d") do p = p + 1 end
        if text:sub(p, p) == "." then
            p = p + 1
            while p <= n and text:sub(p, p):match("%d") do p = p + 1 end
        end
        local e = text:sub(p, p)
        if e == "e" or e == "E" then
            p = p + 1
            local sg = text:sub(p, p)
            if sg == "+" or sg == "-" then p = p + 1 end
            while p <= n and text:sub(p, p):match("%d") do p = p + 1 end
        end
        local s = text:sub(start, p - 1)
        return { kind = "num", value = tonumber(s), text = s }
    end

    local function read_atom()
        local start = p
        if not is_kw_start(text:sub(p, p)) then
            error("hdf: unexpected character '" .. text:sub(p, p) .. "' at " .. p)
        end
        while p <= n and is_kw_char(text:sub(p, p)) do p = p + 1 end
        local s = text:sub(start, p - 1)
        if s == "true" then return { kind = "bool", value = true, text = "true" } end
        if s == "false" then return { kind = "bool", value = false, text = "false" } end
        if s == "null" then return { kind = "null", value = nil, text = "null" } end
        local num = tonumber(s)
        if num and (s:match("^-?%d+$") or s:match("^-?%d+%.%d+") or
                    s:match("^-?%d+[eE][+-]?%d+")) then
            return { kind = "num", value = num, text = s }
        end
        return { kind = "kw", text = s }
    end

    local parse_value

    local function read_list()
        p = p + 1
        local items = {}
        while true do
            skip()
            local c = text:sub(p, p)
            if c == "" then error("hdf: unterminated list") end
            if c == ")" then p = p + 1; break end
            items[#items + 1] = parse_value()
        end
        return { kind = "list", items = items }
    end

    parse_value = function()
        skip()
        local c = text:sub(p, p)
        if c == "(" then return read_list() end
        if c == "[" then return read_raw() end
        if c == '"' then return read_quoted() end
        if c == "%" then p = p + 1; return { kind = "empty", text = "" } end
        if c:match("%d") or (c == "-" and text:sub(p + 1, p + 1):match("%d")) then
            return read_number()
        end
        return read_atom()
    end

    local doc = {}
    while true do
        skip()
        if p > n then break end
        doc[#doc + 1] = parse_value()
    end
    return doc
end

local function is_rem(node)
    return node and node.kind == "list" and node.items[1] and
           node.items[1].kind == "kw" and node.items[1].text == "rem"
end

local function to_value(node)
    if node.kind == "list" then
        if is_rem(node) then return nil end
        local items = node.items
        if items[1] and items[1].kind == "kw" and items[1].text == "seq" then
            local arr = {}
            for i = 2, #items do
                local v = to_value(items[i])
                if v ~= nil then arr[#arr + 1] = v end
            end
            return arr
        end
        local obj = {}
        local i = 1
        while i <= #items do
            local k = items[i]
            local v = items[i + 1]
            if not k or (k.kind ~= "kw" and k.kind ~= "str") then
                error("hdf: object key must be a keyword or string")
            end
            obj[k.text] = to_value(v)
            i = i + 2
        end
        return obj
    end
    if node.kind == "bool" then return node.value end
    if node.kind == "null" then return nil end
    if node.kind == "num" then return node.value end
    return node.text
end

function M.parse_text(text)
    return new_parser(text)
end

function M.to_data(doc)
    local values = {}
    for i = 1, #doc do
        if not is_rem(doc[i]) then values[#values + 1] = doc[i] end
    end
    if #values == 1 then return to_value(values[1]) end
    if #values == 0 then return nil end
    local obj = {}
    local i = 1
    while i <= #values do
        local k = values[i]
        local v = values[i + 1]
        if not k or (k.kind ~= "kw" and k.kind ~= "str") then
            error("hdf: root key must be a keyword or string")
        end
        obj[k.text] = to_value(v)
        i = i + 2
    end
    return obj
end

function M.load(path)
    local f = assert(io.open(path, "r"))
    local text = f:read("*a")
    f:close()
    return M.to_data(M.parse_text(text))
end

return M
