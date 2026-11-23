-- Headless probe: drive mpg123 -R through apps/mp3player/player.lua.
package.path = (os.getenv("MP3PLAYER_ROOT") or ".") .. "/apps/mp3player/?.lua;" .. package.path

local out_path = os.getenv("MP3PLAYER_TEST_OUT")
local mp3 = os.getenv("MP3PLAYER_TEST_MP3")
if not out_path or not mp3 then error("MP3PLAYER_TEST_OUT/MP3 are required") end

local output = os.getenv("MP3PLAYER_TEST_OUTPUT") or "dummy"
local player = require("player")
local p = player.new({
    player = {
        args = { "mpg123", "-R", "-o", output },
    },
    volume = 80,
})

local seen_playing = false
local seen_paused = false
local seen_stopped = false
local toggled = false
local saw_f = false
local max_pos = 0
local polls = 0

p:set_on_update(function(self)
    if self.state == "playing" then seen_playing = true end
    if self.state == "paused" then seen_paused = true end
    if self.state == "stopped" then seen_stopped = true end
    if self.position and self.position > max_pos then max_pos = self.position end
    if self.duration and self.duration > 0 then saw_f = true end
end)

local job, err = p:start()
if not job then error("player start: " .. tostring(err)) end
p:load(mp3)

local function finish()
    local f = assert(io.open(out_path, "w"))
    f:write("output=" .. tostring(output) .. "\n")
    f:write("playing=" .. tostring(seen_playing) .. "\n")
    f:write("paused=" .. tostring(seen_paused) .. "\n")
    f:write("stopped=" .. tostring(seen_stopped) .. "\n")
    f:write("saw_f=" .. tostring(saw_f) .. "\n")
    f:write("max_pos=" .. tostring(max_pos) .. "\n")
    f:write("polls=" .. tostring(polls) .. "\n")
    f:close()
    p:quit()
    xtappexitflag()
end

local function poll()
    polls = polls + 1
    if seen_stopped then finish(); return end
    if polls > 200 then finish(); return end

    if seen_playing and not toggled and polls > 50 then
        toggled = true
        p:toggle()
    elseif toggled and seen_paused and not seen_stopped then
        p:stop()
    end
    xtapptimeout(20, poll)
end

xtapptimeout(20, poll)
