-- Lua facade for the in-process libmpg123 + libpulse-simple DSP engine.
-- Keeps the same surface as player.lua so main.lua can switch engines.

local ok, engine_mod = pcall(require, "mp3engine")

local M = {}
M.__index = M
M.available = ok and type(engine_mod) == "table" and engine_mod.available()
M.engine_type = "dsp"

function M.new(cfg)
    if not M.available then return nil, "mp3engine.so unavailable" end
    local p = setmetatable({}, M)
    p.cfg = cfg or {}
    p.engine = nil
    p.path = nil
    p.state = "idle"
    p.position = 0
    p.duration = 0
    p.levels = nil
    p.rms = { -120, -120 }
    p.peak = { -120, -120 }
    p.last_error = nil
    p.volume = (cfg and cfg.volume) or 80
    p.eq_db = {}
    for i = 1, 5 do
        p.eq_db[i] = ((cfg and cfg.eq_gain and cfg.eq_gain[i]) or 0)
    end
    return p
end

function M:set_on_update(fn) end

function M:start()
    return true
end

function M:is_running()
    return self.engine ~= nil and self.engine:is_running()
end

function M:load(path)
    self:stop()
    self.path = path
    local cfg = self.cfg
    local sink = (cfg.player and cfg.player.sink) or ""
    local bands = (cfg.spectrum and cfg.spectrum.bands) or 24
    local eq = {}
    for i = 1, 5 do eq[i] = self.eq_db[i] or 0 end
    local freqs = {}
    local cfg_bands = (cfg.eq_bands or { 60, 250, 1000, 4000, 12000 })
    for i = 1, 5 do freqs[i] = tonumber(cfg_bands[i]) or 0 end

    local e, err = engine_mod.new{
        file = path,
        sink = sink,
        bands = bands,
        volume = self.volume,
        eq = eq,
        freqs = freqs,
    }
    if not e then
        self.state = "error"
        self.last_error = tostring(err)
        return nil, err
    end
    self.engine = e
    local pok, perr = e:play()
    if not pok then
        self.state = "error"
        self.last_error = tostring(perr)
        return nil, perr
    end
    self.state = "playing"
    self.position = 0
    self.duration = e:duration()
    return true
end

function M:toggle()
    if not self.engine then
        if self.path then return self:load(self.path) end
        return nil, "no track loaded"
    end
    if self.state == "playing" then
        self.engine:pause()
        self.state = "paused"
    elseif self.state == "paused" then
        self.engine:resume()
        self.state = "playing"
    else
        return self:load(self.path)
    end
    return true
end

function M:play()
    if self.state == "paused" then return self:toggle() end
    if self.state == "playing" then return true end
    if self.path then return self:load(self.path) end
    return nil, "no track loaded"
end

function M:pause()
    if self.state == "playing" and self.engine then
        self.engine:pause()
        self.state = "paused"
    end
end

function M:stop()
    if self.engine then
        self.engine:stop()
        self.engine = nil
    end
    self.state = "stopped"
    self.levels = nil
    self.rms = { -120, -120 }
    self.peak = { -120, -120 }
end

function M:seek(seconds)
    seconds = tonumber(seconds) or 0
    if seconds < 0 then seconds = 0 end
    if self.engine then
        self.engine:seek(seconds)
        self.position = seconds
    end
end

function M:set_volume(v)
    v = tonumber(v) or self.volume
    if v < 0 then v = 0 end
    if v > 100 then v = 100 end
    self.volume = v
    if self.engine then self.engine:set_volume(v) end
end

function M:set_eq_band(i, db)
    i = tonumber(i) or 0
    db = tonumber(db) or 0
    if i < 1 or i > 5 then return nil, "band out of range" end
    self.eq_db[i] = db
    if self.engine then self.engine:set_eq(i - 1, db) end
end

-- Compatibility with player.lua's subband API; ignored by the DSP engine.
function M:set_eq(band, mult)
    return true
end

function M:update()
    if not self.engine then return end
    local s = self.engine:status()
    self.state = s.state or self.state
    self.position = s.position or 0
    self.duration = s.duration or self.duration or 0
    self.rms = s.rms or self.rms
    self.peak = s.peak or self.peak
    self.levels = s.levels or self.levels
    if s.error and s.error ~= "" then self.last_error = s.error end
end

function M:quit()
    self:stop()
end

return M
