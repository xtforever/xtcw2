-- Unit test for apps/mp3player/c/mp3fft.so.
-- Runs with plain lua5.3; no X11 needed.

local root = os.getenv("XTCW_ROOT") or "."
package.cpath = root .. "/apps/mp3player/c/?.so;" .. package.cpath

local fft = require("mp3fft")

local function fail(msg)
    io.stderr:write("FAIL: " .. msg .. "\n")
    os.exit(1)
end

local function ok(cond, msg)
    if not cond then fail(msg or "condition failed") end
end

local rate = 22050
local n = 1024
local amp = 16000
local freq = 1000

local function pcm_sine(freq, amp, frames, channels)
    local out = {}
    for i = 0, frames - 1 do
        local v = math.floor(amp * math.sin(2 * math.pi * freq * i / rate) + 0.5)
        out[#out + 1] = string.pack("<h", v)
        if channels == 2 then out[#out + 1] = string.pack("<h", v) end
    end
    return table.concat(out)
end

ok(fft.version() == "1.0", "version")
local a = fft.new{ rate = rate, size = n, bands = 24, min_db = -50, max_db = 0 }

local data = pcm_sine(freq, amp, n, 2)
local r, err = a:process_pcm_data(data, 2)
ok(r, "process_pcm_data: " .. tostring(err))
ok(#r.levels == 24, "band count")
ok(r.bytes == n * 2 * 2, "byte count")

local best, best_i = -1000, 0
for i, v in ipairs(r.levels) do
    ok(type(v) == "number" and v == v, "finite band " .. i)
    if v > best then best, best_i = v, i end
end
ok(best_i >= 10 and best_i <= 18, "1 kHz peak band, got " .. best_i)
ok(math.abs(r.rms[1] + 9.23) < 1.0, "left RMS " .. tostring(r.rms[1]))
ok(math.abs(r.peak[1] + 6.23) < 1.0, "left peak " .. tostring(r.peak[1]))

local short, serr = a:process_pcm_data(string.rep("\0", 16), 2)
ok(short == nil and serr, "short data rejected")

-- File API and offset handling: first window sine, second window silence.
local path = os.tmpname()
local f = assert(io.open(path, "wb"))
f:write(pcm_sine(freq, amp, n, 2))
f:write(string.rep("\0", n * 2 * 2))
f:close()

local rf, ferr = a:process_pcm_file(path, 0, 2)
ok(rf, "process_pcm_file: " .. tostring(ferr))
local sil, silerr = a:process_pcm_file(path, n * 2 * 2, 2)
ok(sil, "process_pcm_file offset: " .. tostring(silerr))
local max_sil = -1000
for _, v in ipairs(sil.levels) do if v > max_sil then max_sil = v end end
ok(max_sil <= -49.0, "silence window stays at floor, got " .. max_sil)

os.remove(path)
print("test_mp3_fft: OK")
