-- ID3v1 / ID3v2 tag reader for the mp3player demo.
-- Pure Lua 5.3; malformed tags fall back to filename/directory defaults.

local M = {}

local function trim_nul(s)
    s = tostring(s or "")
    s = s:gsub("%z+$", "")
    return (s:match("^%s*(.-)%s*$"))
end

local function latin1_to_utf8(s)
    local out = {}
    for i = 1, #s do
        out[#out + 1] = utf8.char(s:byte(i))
    end
    return table.concat(out)
end

local function utf16_to_utf8(s, default_be)
    local out = {}
    local i = 1
    local be = default_be and true or false
    if s:sub(1, 2) == "\255\254" then
        be = false; i = 3
    elseif s:sub(1, 2) == "\254\255" then
        be = true; i = 3
    end
    while i + 1 <= #s do
        local b1, b2 = s:byte(i, i + 1)
        local cp = be and (b1 * 256 + b2) or (b2 * 256 + b1)
        i = i + 2
        if cp >= 0xD800 and cp <= 0xDBFF and i + 1 <= #s then
            local c1, c2 = s:byte(i, i + 1)
            local lo = be and (c1 * 256 + c2) or (c2 * 256 + c1)
            if lo >= 0xDC00 and lo <= 0xDFFF then
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00)
                i = i + 2
            end
        end
        if cp ~= 0 then out[#out + 1] = utf8.char(cp) end
    end
    return table.concat(out)
end

local function decode_text(raw)
    if raw == nil or raw == "" then return "" end
    local enc = raw:byte(1) or 0
    local body = raw:sub(2)
    if enc == 0 then
        return trim_nul(latin1_to_utf8(body))
    elseif enc == 1 then
        return trim_nul(utf16_to_utf8(body, false))
    elseif enc == 2 then
        return trim_nul(utf16_to_utf8(body, true))
    elseif enc == 3 then
        return trim_nul(body)
    end
    return trim_nul(latin1_to_utf8(raw))
end

local function syncsafe(s, pos)
    local a, b, c, d = s:byte(pos, pos + 3)
    if not d then return 0 end
    return a * 0x200000 + b * 0x4000 + c * 0x80 + d
end

local function u32be(s, pos)
    local a, b, c, d = s:byte(pos, pos + 3)
    if not d then return 0 end
    return ((a * 256 + b) * 256 + c) * 256 + d
end

local function u24be(s, pos)
    local a, b, c = s:byte(pos, pos + 2)
    if not c then return 0 end
    return (a * 256 + b) * 256 + c
end

local function deunsync(s)
    local out = {}
    local i = 1
    while i <= #s do
        local c = s:byte(i)
        out[#out + 1] = string.char(c)
        if c == 0xFF and s:byte(i + 1) == 0x00 then
            i = i + 2
        else
            i = i + 1
        end
    end
    return table.concat(out)
end

local V22 = {
    TT2 = "title", TP1 = "artist", TP2 = "albumartist", TAL = "album",
    TRK = "track", TYE = "year", TCO = "genre", TLE = "duration",
}
local V23 = {
    TIT2 = "title", TPE1 = "artist", TPE2 = "albumartist", TALB = "album",
    TRCK = "track", TYER = "year", TDRC = "year", TCON = "genre",
    TLEN = "duration",
}

local function parse_v2(data, version)
    local tags = {}
    local pos = 1
    local frame_map = (version == 2) and V22 or V23

    while pos + 6 <= #data do
        local id, size, flags, header
        if version == 2 then
            id = data:sub(pos, pos + 2)
            size = u24be(data, pos + 3)
            header = 6
            flags = 0
        else
            id = data:sub(pos, pos + 3)
            if version == 4 then
                size = syncsafe(data, pos + 4)
            else
                size = u32be(data, pos + 4)
            end
            flags = data:byte(pos + 8) or 0
            header = 10
        end
        if not id or id:match("^%z+$") or size <= 0 then break end
        local start = pos + header
        if start + size - 1 > #data then break end
        local body = data:sub(start, start + size - 1)

        -- v2.3/2.4 per-frame unsynchronisation (0x02) and data-length (0x01)
        if version ~= 2 and (flags & 0x02) ~= 0 then
            body = deunsync(body)
        end

        local key = frame_map[id]
        if key and body ~= "" then
            if key == "duration" then
                local ms = tonumber(decode_text(body))
                if ms then tags.duration = ms / 1000 end
            else
                tags[key] = decode_text(body)
            end
        end
        pos = start + size
    end
    return tags
end

local function read_v2(f)
    local head = f:read(10)
    if not head or head:sub(1, 3) ~= "ID3" then return nil end
    local version = head:byte(4) or 0
    local flags = head:byte(6) or 0
    local size = syncsafe(head, 7)
    if size <= 0 or size > 16 * 1024 * 1024 then return nil end
    local data = f:read(size)
    if not data or #data < size then return nil end
    if (flags & 0x80) ~= 0 then data = deunsync(data) end

    -- extended header
    if (flags & 0x40) ~= 0 and #data >= 4 then
        local eh
        if version == 4 then eh = syncsafe(data, 1) else eh = u32be(data, 1) + 4 end
        if eh > 0 and eh < #data then data = data:sub(eh + 1) end
    end

    local tags = parse_v2(data, version)
    tags.version = version
    return tags
end

local function read_v1(f)
    local ok = f:seek("end", -128)
    if not ok then return nil end
    local data = f:read(128)
    if not data or #data < 128 or data:sub(1, 3) ~= "TAG" then return nil end

    local function field(a, b) return trim_nul(latin1_to_utf8(data:sub(a, b))) end
    local tags = {
        title = field(4, 33),
        artist = field(34, 63),
        album = field(64, 93),
        year = field(94, 97),
        genre = string.format("%d", data:byte(128) or 255),
    }
    local comment = data:sub(98, 127)
    if comment:byte(29) == 0 and (comment:byte(30) or 0) ~= 0 then
        tags.track = string.format("%d", comment:byte(30))
        tags.comment = trim_nul(latin1_to_utf8(comment:sub(1, 28)))
    else
        tags.comment = trim_nul(latin1_to_utf8(comment))
    end
    return tags
end

local function fallback(path)
    local base = path:match("([^/]+)$") or path
    local title = base:gsub("%.[Mm][Pp]3$", "")
    local dir = path:match("^(.*)/[^/]+$") or ""
    local album = dir:match("([^/]+)$") or ""
    return {
        title = title,
        artist = "Unknown Artist",
        album = album,
        track = "",
    }
end

function M.read(path)
    local f, err = io.open(path, "rb")
    if not f then return nil, err end
    local tags = read_v2(f) or {}
    local v1 = read_v1(f)
    f:close()

    if v1 then
        for k, v in pairs(v1) do
            if tags[k] == nil or tags[k] == "" then tags[k] = v end
        end
    end

    local fb = fallback(path)
    for k, v in pairs(fb) do
        if tags[k] == nil or tags[k] == "" then tags[k] = v end
    end

    tags.path = path
    tags.track = tags.track or ""
    tags.disc = tags.disc or ""
    return tags
end

-- "3/12" -> 3, 12
function M.split_pair(s)
    local a, b = tostring(s or ""):match("^%s*(%d+)%s*/%s*(%d+)")
    if a then return tonumber(a), tonumber(b) end
    return tonumber(tostring(s or ""):match("^%s*(%d+)")), nil
end

return M
