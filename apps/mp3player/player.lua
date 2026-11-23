-- mpg123 -R remote controller for the mp3player demo.
--
-- The pty runner comes from commander/script_process.c; apps/mp3player/script.lua
-- is the minimal AdmPanel wrapper copy.  Commands are newline-terminated and
-- only lines starting with '@' are interpreted (the pty echoes input).

local script = require("script")

local M = {}
M.__index = M
M.engine_type = "mpg123"

local function trim(s)
    return (tostring(s or ""):match("^%s*(.-)%s*$"))
end

local function tonum(s)
    return tonumber(trim(s))
end

function M.new(cfg)
    cfg = cfg or {}
    local p = setmetatable({}, M)
    p.cfg = cfg
    p.job = nil
    p.state = "idle"          -- idle | loading | playing | paused | stopped | error
    p.position = 0
    p.duration = 0
    p.frame = 0
    p.frames_left = 0
    p.stream = ""
    p.title = nil
    p.path = nil
    p.tags = {}
    p.tag_block = nil
    p.last_error = nil
    p.on_update = nil
    p.volume = cfg.volume or 80
    p._started = false
    p._outputs = {}
    p._output_index = 1
    p._output_ok = false
    p._quitting = False
    p.eq = {}
    p:_init_outputs()
    return p
end

function M:_init_outputs()
    local pc = self.cfg.player or {}
    self._fixed_argv = nil
    if type(pc.args) == "table" and #pc.args > 0 then
        self._fixed_argv = {}
        for i, v in ipairs(pc.args) do self._fixed_argv[i] = tostring(v) end
        self._outputs = { "custom" }
        self._output_index = 1
        return
    end

    local outs, seen = {}, {}
    local function add(o)
        o = tostring(o or "")
        if o ~= "" and not seen[o] then
            seen[o] = true
            outs[#outs + 1] = o
        end
    end
    add(pc.output or "pulse")
    if type(pc.fallback_outputs) == "table" then
        for _, o in ipairs(pc.fallback_outputs) do add(o) end
    end
    add(pc.fallback_output)
    add("dummy")
    self._outputs = outs
    self._output_index = 1
end

function M:set_on_update(fn)
    self.on_update = fn
end

function M:_notify()
    if self.on_update then
        local ok, err = pcall(self.on_update, self)
        if not ok then
            io.stderr:write("mp3player player update error: " .. tostring(err) .. "\n")
        end
    end
end

function M:_argv()
    if self._fixed_argv then
        local a = {}
        for i, v in ipairs(self._fixed_argv) do a[i] = v end
        return a
    end
    local pc = self.cfg.player or {}
    local cmd = pc.command or "mpg123"
    local output = self._outputs[self._output_index] or "dummy"
    self.output_name = output
    return { cmd, "-R", "-o", output }
end

function M:start()
    if self.job and script.is_running(self.job) then
        return self.job
    end

    _G.mp3player_player_event = function(ev)
        self:_on_event(ev)
    end

    local job, err = script.start_argv(self:_argv(), {
        on_event = "mp3player_player_event",
        cwd = self.cfg.player and self.cfg.player.cwd or nil,
    })
    if not job then
        self.state = "error"
        self.last_error = tostring(err)
        self:_notify()
        return nil, err
    end

    self.job = job
    self._started = true
    self._output_ok = false
    self.state = "idle"
    self:_notify()

    -- If the output module hangs or fails before the first protocol line,
    -- try the next configured output.  The timer is harmless if the player
    -- is already responding.
    self._start_timeout = xtapptimeout(1500, function()
        if not self._quitting and not self._output_ok and self.job then
            self:_fallback("no response from output " .. tostring(self.output_name))
        end
    end)
    return job
end

function M:_fallback(reason)
    if self._quitting then return end
    self.last_error = tostring(reason or "output failed")
    if self._output_index >= #self._outputs then
        self.state = "error"
        self:_notify()
        return
    end

    local old = self.job
    self.job = nil
    if old and script.is_running(old) then script.stop(old) end

    self._output_index = self._output_index + 1
    self.state = "idle"
    local job, err = self:start()
    if not job then
        self.state = "error"
        self.last_error = tostring(err)
        self:_notify()
        return
    end
    self:set_volume(self.volume)
    for band, mult in pairs(self.eq) do self:set_eq(band, mult) end
    if self.path then self:load(self.path) end
end

function M:is_running()
    return self.job ~= nil and script.is_running(self.job)
end

function M:_write(cmd)
    if not self.job then return nil, "player not started" end
    local ok, err = script.write(self.job, cmd .. "\n")
    if not ok then
        self.last_error = tostring(err)
    end
    return ok, err
end

function M:load(path)
    self.path = path
    self.title = nil
    self.tags = {}
    self.tag_block = nil
    self.position = 0
    self.duration = 0
    self.state = "loading"
    self:_notify()
    return self:_write("L " .. path)
end

function M:play()
    if self.state == "paused" then
        return self:_write("P")
    end
    if self.path then
        return self:load(self.path)
    end
    return nil, "no track loaded"
end

function M:pause()
    if self.state == "playing" then
        return self:_write("P")
    end
    return nil, "not playing"
end

function M:toggle()
    if self.state == "playing" or self.state == "paused" or self.state == "loading" then
        return self:_write("P")
    end
    if self.path then return self:load(self.path) end
    return nil, "no track loaded"
end

function M:stop()
    return self:_write("S")
end

function M:seek(seconds)
    seconds = tonumber(seconds) or 0
    if seconds < 0 then seconds = 0 end
    return self:_write(string.format("J %.3fs", seconds))
end

function M:set_volume(v)
    v = tonumber(v) or self.volume
    if v < 0 then v = 0 end
    if v > 100 then v = 100 end
    self.volume = v
    return self:_write(string.format("V %d", math.floor(v + 0.5)))
end

function M:set_eq(band, multiplier)
    band = math.floor(tonumber(band) or 0)
    multiplier = tonumber(multiplier) or 1.0
    if band < 0 or band > 31 then return nil, "band out of range" end
    if multiplier < 0.1 then multiplier = 0.1 end
    if multiplier > 4.0 then multiplier = 4.0 end
    self.eq[band] = multiplier
    return self:_write(string.format("E 3 %d %.4f", band, multiplier))
end

function M:quit()
    self._quitting = True
    local job = self.job
    if not job then return true end
    self:_write("Q")
    self.job = nil
    self.state = "stopped"
    self:_notify()
    -- Give mpg123 a moment to exit, then make sure the pty is gone.
    xtapptimeout(50, function()
        if script.is_running(job) then script.stop(job) end
    end)
    return true
end

function M:_on_event(ev)
    if not ev then return end
    if ev.kind == "exit" then
        self.job = nil
        if not self._quitting and not self._output_ok and self.state ~= "stopped" then
            self:_fallback("output " .. tostring(self.output_name) .. " failed")
        else
            if self.state ~= "stopped" then self.state = "stopped" end
            self:_notify()
        end
        return
    end
    if ev.kind ~= "line" then return end

    local line = trim(ev.text)
    if line == "" then return end

    -- pty echo and mpg123's own banner are not protocol lines
    if line:sub(1, 1) ~= "@" then
        if line:lower():match("error") then self.last_error = line end
        return
    end

    -- Any protocol response means the output module is alive.
    self._output_ok = true

    if self.tag_block then
        if line == "@T }" then
            self.tag_block = nil
        else
            local key, value = line:match("^@T%s+([^:]+):%s*(.*)$")
            if key then
                self.tags[trim(key)] = value
            elseif line:sub(1, 5) == "@T = " or line:sub(1, 4) == "@T =" then
                -- continuation of the previous text frame
                local last = self.tag_block
                if last then
                    self.tags[last] = (self.tags[last] or "") .. trim(line:sub(4))
                end
            end
        end
        return
    end

    local cmd, rest = line:match("^@(%S+)%s*(.*)$")
    if not cmd then return end

    if cmd == "P" then
        local n = tonum(rest)
        if n == 2 then self.state = "playing"
        elseif n == 1 then self.state = "paused"
        elseif n == 0 then self.state = "stopped"
        end
        self:_notify()
    elseif cmd == "F" then
        local frame, left, sec, sec_left = rest:match("^(%-?%d+)%s+(%-?%d+)%s+([%d%.]+)%s+([%d%.]+)")
        if frame then
            self.frame = tonumber(frame)
            self.frames_left = tonumber(left)
            self.position = tonumber(sec) or 0
            self.duration = self.position + (tonumber(sec_left) or 0)
            self:_notify()
        end
    elseif cmd == "I" then
        -- @I <filename> or @I ID3:artist album year comment genre
        if rest ~= "" and rest:sub(1, 4) ~= "ID3:" then
            self.title = rest
        end
    elseif cmd == "S" then
        self.stream = rest
    elseif cmd == "T" then
        if rest == "{" then
            self.tag_block = nil
        else
            local key, value = rest:match("^([^:]+):%s*(.*)$")
            if key then
                self.tags[trim(key)] = value
            end
        end
    elseif cmd == "R" then
        self.version = rest
    end
end

return M
