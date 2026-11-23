-- M3U / M3U8 playlist reader and writer.

local M = {}

local function dirname(path)
    return (path:match("^(.*)/[^/]*$")) or "."
end

local function is_absolute(path)
    return path:sub(1, 1) == "/"
end

local function join(base, path)
    if is_absolute(path) then return path end
    return base .. "/" .. path
end

local function parse_extinf(line)
    local rest = line:sub(9)
    local dur, title = rest:match("^%s*([%-%d%.]+)%s*,%s*(.*)$")
    if not dur then
        dur, title = rest:match("^%s*([%-%d%.]+)%s*$"), ""
    end
    return tonumber(dur), title or ""
end

function M.read(path)
    local f, err = io.open(path, "r")
    if not f then return nil, err end
    local text = f:read("*a")
    f:close()
    if text:sub(1, 3) == "\239\187\191" then text = text:sub(4) end

    local base = dirname(path)
    local entries = {}
    local pending = nil

    for raw in (text .. "\n"):gmatch("(.-)\r?\n") do
        local line = raw:match("^%s*(.-)%s*$")
        if line ~= "" then
            if line:sub(1, 7) == "#EXTM3U" then
                -- header
            elseif line:sub(1, 8) == "#EXTINF:" then
                local dur, title = parse_extinf(line)
                pending = { duration = dur, title = title }
            elseif line:sub(1, 1) == "#" then
                -- comment
            else
                local entry = pending or {}
                pending = nil
                entry.file = line
                entry.path = join(base, line)
                entry.line = line
                if not entry.title or entry.title == "" then
                    entry.title = line:match("([^/]+)$") or line
                end
                entries[#entries + 1] = entry
            end
        end
    end
    return entries
end

local function fmt_duration(d)
    d = tonumber(d)
    if not d or d < 0 then return "-1" end
    return string.format("%d", math.floor(d + 0.5))
end

function M.write(path, entries)
    local f, err = io.open(path, "w")
    if not f then return nil, err end
    f:write("#EXTM3U\n")
    for _, e in ipairs(entries or {}) do
        local file = e.file or e.path or ""
        local title = e.title or file:match("([^/]+)$") or file
        f:write("#EXTINF:", fmt_duration(e.duration), ",", title, "\n")
        f:write(file, "\n")
    end
    f:close()
    return true
end

function M.is_m3u(path)
    return tostring(path):lower():match("%.m3u8?$") ~= nil
end

return M
