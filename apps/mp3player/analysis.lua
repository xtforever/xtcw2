-- PCM cache and analysis glue for the mp3player demo.
-- Tracks are decoded once to a small stereo s16le cache; the C FFT module
-- then analyses a window at the current playback position.

local script = require("script")
local fft = require("fft")

local M = {}

local function shquote(s)
    return "'" .. tostring(s):gsub("'", "'\\''") .. "'"
end

local function exists(path)
    local f = io.open(path, "rb")
    if f then f:close(); return true end
    return false
end

local function file_size(path)
    local f = io.open(path, "rb")
    if not f then return 0 end
    local size = f:seek("end") or 0
    f:close()
    return size
end

local function fnv1a(s)
    local h = 2166136261
    for i = 1, #s do
        h = (h ~ s:byte(i)) & 0xffffffff
        h = (h * 16777619) & 0xffffffff
    end
    return string.format("%08x", h)
end

local function cache_dir(cfg)
    local d = (cfg.analysis and cfg.analysis.cache_dir) or
              ((os.getenv("HOME") or ".") .. "/.cache/xtcw-mp3player")
    os.execute("mkdir -p " .. shquote(d))
    return d
end

function M.cache_path(cfg, path)
    local st = "?"
    local f = io.open(path, "rb")
    if f then
        local size = f:seek("end") or 0
        f:close()
        st = tostring(size)
    end
    local rate = (cfg.analysis and cfg.analysis.rate) or 22050
    return cache_dir(cfg) .. "/" .. fnv1a(path .. "|" .. st .. "|" .. rate) .. ".pcm"
end

function M.available()
    local p = io.popen("command -v ffmpeg 2>/dev/null", "r")
    if not p then return false end
    local line = p:read("*l")
    p:close()
    return line ~= nil and line ~= ""
end

function M.prepare(cfg, path, on_done)
    local rate = (cfg.analysis and cfg.analysis.rate) or 22050
    local handle = {
        path = path,
        cache = M.cache_path(cfg, path),
        rate = rate,
        channels = 2,
        ready = false,
        job = nil,
        error = nil,
    }

    if exists(handle.cache) and file_size(handle.cache) > 0 then
        handle.ready = true
        if on_done then on_done(handle) end
        return handle
    end

    if not M.available() then
        handle.error = "ffmpeg missing"
        if on_done then on_done(handle) end
        return handle
    end

    _G.mp3player_analysis_event = function(ev)
        if ev and ev.kind == "exit" then
            handle.job = nil
            if file_size(handle.cache) > 0 then
                handle.ready = true
            else
                handle.error = "ffmpeg decode failed"
            end
            if on_done then on_done(handle) end
        end
    end

    local argv = {
        "ffmpeg", "-v", "error", "-y", "-i", path,
        "-vn", "-ac", "2", "-ar", tostring(rate),
        "-f", "s16le", handle.cache,
    }
    local job, err = script.start_argv(argv, {
        on_event = "mp3player_analysis_event",
    })
    if not job then
        handle.error = tostring(err)
        if on_done then on_done(handle) end
        return handle
    end
    handle.job = job
    return handle
end

function M.stop(handle)
    if handle and handle.job and script.is_running(handle.job) then
        script.stop(handle.job)
    end
    if handle then handle.job = nil end
end

function M.read_window(handle, seconds, frames)
    if not handle or not handle.ready then return nil, "analysis not ready" end
    frames = frames or 1024
    local f = io.open(handle.cache, "rb")
    if not f then return nil, "cannot open PCM cache" end
    local offset = math.max(0, math.floor((tonumber(seconds) or 0) * handle.rate)) *
                   handle.channels * 2
    local want = frames * handle.channels * 2
    f:seek("set", offset)
    local data = f:read(want)
    f:close()
    if not data or #data < want then return nil, "short PCM window" end
    return data
end

function M.spectrum(handle, seconds, cfg)
    if not handle or not handle.ready then return nil, "analysis not ready" end
    if not handle.analyzer then
        local sc = (cfg and cfg.spectrum) or {}
        local a, err = fft.new{
            rate = handle.rate,
            size = (cfg and cfg.analysis and cfg.analysis.fft_size) or 1024,
            bands = sc.bands or 24,
            min_db = sc.min_db or -50,
            max_db = sc.max_db or 0,
        }
        if not a then return nil, err end
        handle.analyzer = a
    end
    local data, err = M.read_window(handle, seconds, 1024)
    if not data then return nil, err end
    return handle.analyzer:process_pcm_data(data, handle.channels)
end

return M
