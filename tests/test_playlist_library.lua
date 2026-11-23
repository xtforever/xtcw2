-- Unit tests for apps/mp3player/playlist_library.lua
package.path = (os.getenv("XTCW_ROOT") or ".") .. "/apps/mp3player/?.lua;" .. package.path
local lib = require("playlist_library")
local m3u = require("m3u")

local function fail(msg) io.stderr:write("FAIL: " .. msg .. "\n"); os.exit(1) end
local function eq(a, b, msg)
    if a ~= b then fail((msg or "value") .. ": expected " .. tostring(b) .. ", got " .. tostring(a)) end
end
local function ok(c, msg) if not c then fail(msg or "condition") end end

local root = os.tmpname() .. "_pllib"
os.execute("mkdir -p '" .. root .. "/music/a' '" .. root .. "/music/b' '" .. root .. "/music/c' '" .. root .. "/data'")

local function pad(s, n) return (s .. string.rep("\0", n)):sub(1, n) end
local function v1(title, artist, album, track)
    return "TAG" .. pad(title, 30) .. pad(artist, 30) .. pad(album, 30) ..
           "2020" .. pad("c", 28) .. "\0" .. string.char(track) .. string.char(17)
end
local function write(path, data)
    local f = assert(io.open(path, "wb")); f:write(data); f:close()
end

write(root .. "/music/a/01.mp3", v1("A1", "Artist A", "Album A", 1))
write(root .. "/music/a/02.mp3", v1("A2", "Artist A", "Album A", 2))
write(root .. "/music/b/01.mp3", v1("B1", "Artist B", "Album B", 1))
write(root .. "/music/c/01.mp3", v1("C1", "Artist C", "Album C", 1))

-- Explicit playlist with mixed albums and a deliberate order.
assert(m3u.write(root .. "/music/single.m3u", {
    { file = "a/01.mp3", title = "a01.mp3", duration = 10 },
    { file = "a/02.mp3", title = "a02.mp3", duration = 20 },
}))

assert(m3u.write(root .. "/music/mix.m3u", {
    { file = "b/01.mp3", title = "b01.mp3", duration = 10 },
    { file = "a/02.mp3", title = "a02.mp3", duration = 20 },
    { file = "c/01.mp3", title = "c01.mp3", duration = 30 },
}))

local cfg = {
    extensions = { "mp3" },
    library_file = root .. "/data/library.lua",
    library_playlist_dir = root .. "/data/playlists",
}

local summary, err = lib.add_directory(cfg, root .. "/music")
ok(summary, "add_directory: " .. tostring(err))
ok(summary.added >= 1, "added entries")

local entries = lib.entries(cfg)
local mix
for _, e in ipairs(entries) do
    if e.playlist:match("mix%.m3u$") then mix = e end
end
ok(mix, "mix playlist entry")
eq(mix.alias, "mix", "mixed playlist alias")
local single
for _, e in ipairs(entries) do
    if e.playlist:match("single%.m3u$") then single = e end
end
ok(single, "single playlist entry")
eq(single.alias, "Album A", "single-album playlist uses ID3 album")
eq(mix.generated, false, "mix generated flag")

local tracks = lib.tracks(cfg, mix)
eq(#tracks, 3, "mix track count")
eq(tracks[1].title, "B1", "ID3 title preferred 1")
eq(tracks[2].title, "A2", "ID3 title preferred 2")
eq(tracks[3].title, "C1", "ID3 title preferred 3")
eq(tracks[1].album, "mix", "album is alias")

local queue = lib.queue(cfg)
ok(#queue >= 3, "queue built")
for _, t in ipairs(queue) do
    if t.title == "B1" then eq(t.album_alias, "mix", "queue album alias") end
end

-- Add twice: no duplicates.
local before = #entries
lib.add_directory(cfg, root .. "/music")
entries = lib.entries(cfg)
eq(#entries, before, "no duplicate entries")

-- Rename.
local okr, newalias = lib.rename(cfg, mix.playlist, "Mixed Album")
ok(okr, "rename")
eq(newalias, "Mixed Album", "renamed alias")
entries = lib.entries(cfg)
for _, e in ipairs(entries) do
    if e.playlist == mix.playlist then eq(e.alias, "Mixed Album", "persisted alias") end
end

-- Generated playlists for a directory without m3u.
os.execute("mkdir -p '" .. root .. "/music/auto'")
write(root .. "/music/auto/02.mp3", v1("Auto A2", "Artist A", "Auto Album A", 2))
write(root .. "/music/auto/01.mp3", v1("Auto A1", "Artist A", "Auto Album A", 1))
write(root .. "/music/auto/11.mp3", v1("Auto B1", "Artist B", "Auto Album B", 1))

local s2 = lib.add_directory(cfg, root .. "/music/auto")
ok(s2, "add auto dir")
ok(s2.generated >= 2, "generated at least two playlists")
entries = lib.entries(cfg)
local gen_a, gen_b
for _, e in ipairs(entries) do
    if e.generated and e.alias == "Auto Album A" then gen_a = e end
    if e.generated and e.alias == "Auto Album B" then gen_b = e end
end
ok(gen_a, "generated album A")
ok(gen_b, "generated album B")
local at = lib.tracks(cfg, gen_a)
eq(#at, 2, "generated A track count")
eq(at[1].title, "Auto A1", "generated A order 1")
eq(at[2].title, "Auto A2", "generated A order 2")

-- Rescan detects a new file.
write(root .. "/music/auto/03.mp3", v1("Auto A3", "Artist A", "Auto Album A", 3))
assert(lib.rescan(cfg))
entries = lib.entries(cfg)
for _, e in ipairs(entries) do
    if e.generated and e.alias == "Auto Album A" then
        eq(e.track_count, 3, "rescan track count")
    end
end

-- Remove generated entry and its managed playlist.
local removed_playlist = gen_b.playlist
assert(lib.remove(cfg, removed_playlist, true))
ok(not io.open(removed_playlist, "rb"), "generated playlist removed")
entries = lib.entries(cfg)
for _, e in ipairs(entries) do
    ok(e.playlist ~= removed_playlist, "entry removed")
end

-- ensure(): first run imports music_dir and persists library.lua
local cfg2 = {
    music_dir = root .. "/music",
    extensions = { "mp3" },
    library_file = root .. "/data/library2.lua",
    library_playlist_dir = root .. "/data/playlists2",
}
local ens, imported = lib.ensure(cfg2)
ok(ens and #ens > 0, "ensure imported entries")
ok(imported, "ensure reports first import")
local f = io.open(cfg2.library_file, "rb")
ok(f, "ensure persisted library file")
f:close()
local n1 = #ens
local ens2, imported2 = lib.ensure(cfg2)
eq(#ens2, n1, "ensure does not duplicate")
ok(not imported2, "second ensure does not reimport")

os.execute("rm -rf '" .. root .. "'")
print("test_playlist_library: OK")
