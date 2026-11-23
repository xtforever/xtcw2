-- Integration test for the in-process DSP engine (null output).
package.cpath = (os.getenv("XTCW_ROOT") or ".") .. "/apps/mp3player/c/?.so;" .. package.cpath
local eng = require("mp3engine")

local function fail(msg) io.stderr:write("FAIL: " .. msg .. "\n"); os.exit(1) end
local function ok(c, msg) if not c then fail(msg or "condition") end end

ok(eng.available(), "available")

local file = os.getenv("MP3ENGINE_TEST_FILE")
if not file or file == "" then
    fail("MP3ENGINE_TEST_FILE is required")
end

local e, err = eng.new{
    file = file,
    sink = "null",
    bands = 24,
    volume = 70,
    eq = { 0, 2, 0, -1, 0 },
    freqs = { 100, 400, 1600, 5000, 10000 },
}
ok(e, "new: " .. tostring(err))
ok(e:play(), "play")

local function sleep(s) os.execute("sleep " .. tostring(s)) end
sleep(0.4)
local s = e:status()
ok(s.state == "playing", "state playing, got " .. tostring(s.state))
ok(s.position > 0.1, "position advanced")
ok(s.duration > 1.0, "duration")
ok(#s.levels == 24, "24 spectrum bands")
ok(type(s.rms[1]) == "number" and s.rms[1] == s.rms[1], "rms finite")

e:pause()
sleep(0.1)
s = e:status()
ok(s.state == "paused", "state paused, got " .. tostring(s.state))

e:resume()
e:seek(10)
sleep(0.3)
s = e:status()
ok(s.state == "playing", "state playing after seek")
ok(s.position >= 10.0, "seek position, got " .. tostring(s.position))

e:set_volume(40)
e:set_eq(2, 3.0)
sleep(0.1)
ok(e:is_running(), "is_running")

e:stop()
s = e:status()
ok(s.state == "stopped", "state stopped, got " .. tostring(s.state))
print("test_mp3engine_module: OK")
