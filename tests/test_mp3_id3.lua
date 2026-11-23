-- Unit tests for apps/mp3player/id3.lua
package.path = (os.getenv("XTCW_ROOT") or ".") .. "/apps/mp3player/?.lua;" .. package.path
local id3 = require("id3")

local function fail(msg)
    io.stderr:write("FAIL: " .. msg .. "\n")
    os.exit(1)
end
local function eq(a, b, msg)
    if a ~= b then fail((msg or "value") .. ": expected " .. tostring(b) .. ", got " .. tostring(a)) end
end
local function ok(c, msg) if not c then fail(msg or "condition") end end

local function syncsafe(n)
    return string.char(
        math.floor(n / 0x200000) % 128,
        math.floor(n / 0x4000) % 128,
        math.floor(n / 0x80) % 128,
        n % 128)
end

local function u32be(n)
    return string.char(
        math.floor(n / 0x1000000) % 256,
        math.floor(n / 0x10000) % 256,
        math.floor(n / 0x100) % 256,
        n % 256)
end

local function u24be(n)
    return string.char(
        math.floor(n / 0x10000) % 256,
        math.floor(n / 0x100) % 256,
        n % 256)
end

local function frame23(id, body)
    return id .. u32be(#body) .. "\0\0" .. body
end
local function frame24(id, body)
    return id .. syncsafe(#body) .. "\0\0" .. body
end
local function frame22(id, body)
    return id .. u24be(#body) .. body
end

local function utf16le(s)
    local out = { "\255\254" }
    for _, cp in utf8.codes(s) do
        out[#out + 1] = string.pack("<I2", cp)
    end
    return table.concat(out)
end

local function write(path, data)
    local f = assert(io.open(path, "wb"))
    f:write(data)
    f:close()
end

local tmp = os.tmpname()

-- ID3v2.3, UTF-8 text frames
local body23 = table.concat({
    frame23("TIT2", string.char(3) .. "Midnight Drive"),
    frame23("TPE1", string.char(3) .. "The Testers"),
    frame23("TALB", string.char(3) .. "Echoes of Youth"),
    frame23("TRCK", string.char(3) .. "3/12"),
    frame23("TYER", string.char(3) .. "2024"),
    frame23("TCON", string.char(3) .. "Synthwave"),
})
write(tmp, "ID3" .. string.char(3, 0, 0) .. syncsafe(#body23) .. body23 .. "AUDIO")
local t, err = id3.read(tmp)
ok(t, "read v2.3: " .. tostring(err))
eq(t.title, "Midnight Drive", "v2.3 title")
eq(t.artist, "The Testers", "v2.3 artist")
eq(t.album, "Echoes of Youth", "v2.3 album")
eq(t.track, "3/12", "v2.3 track")
eq(t.year, "2024", "v2.3 year")
eq(t.version, 3, "v2.3 version")
eq(id3.split_pair(t.track), 3, "split_pair track")

-- ID3v2.4 syncsafe frame sizes
local body24 = table.concat({
    frame24("TIT2", string.char(3) .. "UTF8 Title"),
    frame24("TPE1", string.char(3) .. "Artist 4"),
})
write(tmp, "ID3" .. string.char(4, 0, 0) .. syncsafe(#body24) .. body24)
t = assert(id3.read(tmp))
eq(t.title, "UTF8 Title", "v2.4 title")
eq(t.artist, "Artist 4", "v2.4 artist")
eq(t.version, 4, "v2.4 version")

-- ID3v2.3 UTF-16 with BOM
local body16 = frame23("TIT2", string.char(1) .. utf16le("Müller"))
write(tmp, "ID3" .. string.char(3, 0, 0) .. syncsafe(#body16) .. body16)
t = assert(id3.read(tmp))
eq(t.title, "Müller", "UTF-16 title")

-- ID3v2.2
local body22 = table.concat({
    frame22("TT2", string.char(0) .. "Old Title"),
    frame22("TP1", string.char(0) .. "Old Artist"),
    frame22("TAL", string.char(0) .. "Old Album"),
})
write(tmp, "ID3" .. string.char(2, 0, 0) .. syncsafe(#body22) .. body22)
t = assert(id3.read(tmp))
eq(t.title, "Old Title", "v2.2 title")
eq(t.artist, "Old Artist", "v2.2 artist")
eq(t.version, 2, "v2.2 version")

-- ID3v1 only
local function pad(s, n) return (s .. string.rep("\0", n)):sub(1, n) end
local v1 = "TAG" .. pad("V1 Title", 30) .. pad("V1 Artist", 30) ..
           pad("V1 Album", 30) .. "1999" .. pad("comment", 28) .. "\0\3" .. string.char(17)
write(tmp, string.rep("A", 100) .. v1)
t = assert(id3.read(tmp))
eq(t.title, "V1 Title", "v1 title")
eq(t.artist, "V1 Artist", "v1 artist")
eq(t.album, "V1 Album", "v1 album")
eq(t.year, "1999", "v1 year")
eq(t.track, "3", "v1 track")

-- No tags: filename/directory fallback
local path = os.tmpname() .. "_NoTag.mp3"
write(path, "not really an mp3")
t = assert(id3.read(path))
eq(t.title, path:match("([^/]+)$"):gsub("%.mp3$", ""), "fallback title")
ok(t.artist == "Unknown Artist", "fallback artist")
os.remove(path)

-- Malformed tag must not crash
write(tmp, "ID3" .. string.char(3, 0, 0) .. syncsafe(200) .. "short")
t = assert(id3.read(tmp))
ok(type(t) == "table", "malformed tag returns table")

os.remove(tmp)
print("test_mp3_id3: OK")
