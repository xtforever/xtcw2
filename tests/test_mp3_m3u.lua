-- Unit tests for apps/mp3player/m3u.lua
package.path = (os.getenv("XTCW_ROOT") or ".") .. "/apps/mp3player/?.lua;" .. package.path
local m3u = require("m3u")

local function fail(msg) io.stderr:write("FAIL: " .. msg .. "\n"); os.exit(1) end
local function eq(a, b, msg)
    if a ~= b then fail((msg or "value") .. ": expected " .. tostring(b) .. ", got " .. tostring(a)) end
end
local function ok(c, msg) if not c then fail(msg or "condition") end end

local dir = os.tmpname() .. "_m3u"
os.execute("mkdir -p '" .. dir .. "/sub'")
local path = dir .. "/list.m3u"
local f = assert(io.open(path, "w"))
f:write("\239\187\191#EXTM3U\r\n")
f:write("# comment\r\n")
f:write("#EXTINF:123,First Track\r\n")
f:write("sub/a.mp3\r\n")
f:write("#EXTINF:-1,Second Track\r\n")
f:write("/abs/b.mp3\r\n")
f:write("plain.mp3\r\n")
f:close()

local entries, err = m3u.read(path)
ok(entries, "read: " .. tostring(err))
eq(#entries, 3, "entry count")
eq(entries[1].title, "First Track", "title 1")
eq(entries[1].duration, 123, "duration 1")
eq(entries[1].path, dir .. "/sub/a.mp3", "relative path")
eq(entries[2].path, "/abs/b.mp3", "absolute path")
eq(entries[3].path, dir .. "/plain.mp3", "plain path")
eq(entries[3].title, "plain.mp3", "plain title")

local out = dir .. "/out.m3u"
assert(m3u.write(out, entries))
local again = assert(m3u.read(out))
eq(#again, 3, "roundtrip count")
eq(again[1].title, "First Track", "roundtrip title")
eq(again[1].duration, 123, "roundtrip duration")
eq(again[1].path, dir .. "/sub/a.mp3", "roundtrip path")

ok(m3u.is_m3u("x.m3u"), "m3u extension")
ok(m3u.is_m3u("x.M3U8"), "m3u8 extension")
ok(not m3u.is_m3u("x.mp3"), "mp3 is not m3u")

os.execute("rm -rf '" .. dir .. "'")
print("test_mp3_m3u: OK")
