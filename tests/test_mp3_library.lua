-- Unit tests for apps/mp3player/library.lua
package.path = (os.getenv("XTCW_ROOT") or ".") .. "/apps/mp3player/?.lua;" .. package.path
local library = require("library")
local m3u = require("m3u")

local function fail(msg) io.stderr:write("FAIL: " .. msg .. "\n"); os.exit(1) end
local function eq(a, b, msg)
    if a ~= b then fail((msg or "value") .. ": expected " .. tostring(b) .. ", got " .. tostring(a)) end
end
local function ok(c, msg) if not c then fail(msg or "condition") end end

local root = os.tmpname() .. "_lib"
os.execute("mkdir -p '" .. root .. "/albumA' '" .. root .. "/albumB' '" .. root .. "/cache'")

local function pad(s, n) return (s .. string.rep("\0", n)):sub(1, n) end
local function v1(title, artist, album, track)
    return "TAG" .. pad(title, 30) .. pad(artist, 30) .. pad(album, 30) ..
           "2020" .. pad("c", 28) .. "\0" .. string.char(track) .. string.char(17)
end
local function write(path, data)
    local f = assert(io.open(path, "wb")); f:write(data); f:close()
end

write(root .. "/albumA/02.mp3", v1("Second", "Artist A", "Album A", 2))
write(root .. "/albumA/01.mp3", v1("First", "Artist A", "Album A", 1))
write(root .. "/albumB/01.mp3", v1("Other", "Artist B", "Album B", 1))

local files = library.scan(root, { "mp3" })
eq(#files, 3, "scan count")

local cfg = {
    music_dir = root,
    extensions = { "mp3" },
    sort = "album",
    auto_playlist = "auto.m3u",
    analysis = { cache_dir = root .. "/cache" },
}

local entries, info = library.load(cfg)
ok(entries, "load: " .. tostring(info))
eq(#entries, 3, "load count")
eq(entries[1].title, "First", "album sort first")
eq(entries[2].title, "Second", "album sort second")
eq(entries[3].title, "Other", "album sort third")
ok(info.created, "auto playlist created")

local auto = root .. "/cache/auto.m3u"
local f = io.open(auto, "rb"); ok(f, "auto m3u exists"); f:close()

local groups = library.albums(entries)
eq(#groups, 2, "album groups")
eq(groups[1].album, "Album A", "group 1")
eq(groups[2].album, "Album B", "group 2")

eq(library.next_album(entries, 1), 3, "next album from first")
eq(library.next_album(entries, 3), 1, "next album wraps")
eq(library.prev_album(entries, 3), 1, "prev album from third")
eq(library.prev_album(entries, 1), 3, "prev album wraps")

-- Explicit m3u order wins with sort = "m3u".
local list = root .. "/custom.m3u"
assert(m3u.write(list, {
    { file = "albumB/01.mp3", title = "Other", duration = 10 },
    { file = "albumA/02.mp3", title = "Second", duration = 20 },
    { file = "albumA/01.mp3", title = "First", duration = 30 },
}))
cfg.playlist = list
cfg.sort = "m3u"
local ordered = assert(library.load(cfg))
eq(ordered[1].title, "Other", "m3u order 1")
eq(ordered[2].title, "Second", "m3u order 2")
eq(ordered[3].title, "First", "m3u order 3")

os.execute("rm -rf '" .. root .. "'")
print("test_mp3_library: OK")
