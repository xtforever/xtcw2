-- Unit tests for apps/mp3player/analysis.lua (cache + C FFT glue).
local root = os.getenv("XTCW_ROOT") or "."
package.path = root .. "/apps/mp3player/?.lua;" .. package.path
package.cpath = root .. "/apps/mp3player/c/?.so;" .. package.cpath

local analysis = require("analysis")

local function fail(msg) io.stderr:write("FAIL: " .. msg .. "\n"); os.exit(1) end
local function ok(c, msg) if not c then fail(msg or "condition") end end

local dir = os.tmpname() .. "_analysis"
os.execute("mkdir -p '" .. dir .. "'")
local cfg = {
    analysis = { rate = 22050, cache_dir = dir, fft_size = 1024 },
    spectrum = { bands = 24, min_db = -50, max_db = 0 },
}

local rate, n = 22050, 1024
local cache = dir .. "/test.pcm"
local f = assert(io.open(cache, "wb"))
for i = 0, n - 1 do
    local v = math.floor(16000 * math.sin(2 * math.pi * 1000 * i / rate) + 0.5)
    f:write(string.pack("<hh", v, v))
end
f:close()

local h = { path = "dummy.mp3", cache = cache, rate = rate, channels = 2, ready = true }
local data, err = analysis.read_window(h, 0, 1024)
ok(data, "read_window: " .. tostring(err))
ok(#data == 4096, "window byte count")

local r, serr = analysis.spectrum(h, 0, cfg)
ok(r, "spectrum: " .. tostring(serr))
ok(#r.levels == 24, "spectrum bands")
local best, best_i = -1000, 0
for i, v in ipairs(r.levels) do
    if v > best then best, best_i = v, i end
end
ok(best_i >= 10 and best_i <= 18, "1 kHz peak band, got " .. best_i)
ok(r.rms[1] < 0, "rms present")

-- Cache path helper is stable and uses the .pcm extension.
local p1 = analysis.cache_path(cfg, "dummy.mp3")
local p2 = analysis.cache_path(cfg, "dummy.mp3")
ok(p1 == p2, "cache path stable")
ok(p1:match("%.pcm$") ~= nil, "cache extension")

os.execute("rm -rf '" .. dir .. "'")
print("test_mp3_analysis: OK")
