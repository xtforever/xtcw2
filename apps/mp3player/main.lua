-- mp3player demo bootstrap: fixed chassis, library, mpg123 controller.
local root = os.getenv("MP3PLAYER_ROOT")
if not root or root == "" then root = "." end

package.path = root .. "/apps/mp3player/?.lua;" .. package.path
package.cpath = root .. "/apps/mp3player/c/?.so;" .. package.cpath

local lui = require("lui")
local gui = require("gui_xt")
local library = require("library")
local playlist_library = require("playlist_library")
local script = require("script")
local analysis = require("analysis")

local cfg = require("config")
local cfg_path = os.getenv("MP3PLAYER_CONFIG")
if cfg_path and cfg_path ~= "" then
    local chunk, err = loadfile(cfg_path)
    if not chunk then
        error("mp3player: cannot load config " .. cfg_path .. ": " .. tostring(err))
    end
    local user = chunk()
    if type(user) == "table" then
        for k, v in pairs(user) do cfg[k] = v end
    end
end

local use_dsp = not (cfg.player and cfg.player.engine == "mpg123")
local dsp_ok, dsp_mod = pcall(require, "player_dsp")
local player_mod
if use_dsp and dsp_ok and dsp_mod.available then
    player_mod = dsp_mod
else
    player_mod = require("player")
end

local cw = (cfg.chassis and cfg.chassis.width) or 1160
local ch = (cfg.chassis and cfg.chassis.height) or 770

local function load_queue()
    local entries, imported, summary = playlist_library.ensure(cfg)
    if entries and #entries > 0 then
        return playlist_library.queue(cfg, entries), {
            entries = entries,
            imported = imported,
            summary = summary,
        }
    end
    return library.load(cfg)
end

local tracks, lib_info = load_queue()
if not tracks then
    tracks = {}
    lib_info = { error = lib_info }
end

local index = math.max(1, math.min(tonumber(cfg.start_track) or 1, #tracks))
local player = player_mod.new(cfg)
local status_text = lib_info and lib_info.error or "ready"
local current_analysis = nil
local last_vfd = {}
local last_spec = nil
local last_vu = {}
local last_play_icon = nil

local function set_vfd(field, value)
    value = tostring(value or "")
    if last_vfd[field] ~= value then
        last_vfd[field] = value
        gui.set("vfd", field, value)
    end
end

local function shorten(s, n)
    s = tostring(s or "")
    if #s <= n then return s end
    return s:sub(1, n - 1) .. "…"
end

local function fmt_time(sec)
    sec = math.max(0, math.floor(tonumber(sec) or 0))
    return string.format("%02d:%02d", math.floor(sec / 60), sec % 60)
end

local function track_text()
    local t = tracks[index]
    if not t then return "NO TRACKS" end
    return shorten(string.format("%s  —  %s", t.title or t.file, t.artist or ""), 44)
end

local function status_update()
    local t = tracks[index]
    if not t then
        set_vfd("album", "NO ALBUM")
        set_vfd("title", "NO TRACKS")
        set_vfd("playState", status_text or "IDLE")
        return
    end
    local dur = player.duration > 0 and player.duration or (t.duration or 0)
    local pos = player.position or 0
    local rem = math.max(0, dur - pos)
    set_vfd("album", string.format("%s  ·  %s", t.album or "", t.artist or ""))
    set_vfd("title", t.title or t.file or "")
    set_vfd("elapsed", fmt_time(pos))
    set_vfd("remaining", "-" .. fmt_time(rem))
    set_vfd("playState", string.upper(player.state or "idle"))
    local play_icon = (player.state == "playing") and "pause" or "play"
    if play_icon ~= last_play_icon then
        last_play_icon = play_icon
        gui.set("playbtn", "icon", play_icon)
    end
    gui.set("seek", "positionMs", math.floor(pos * 1000))
    gui.set("seek", "durationMs", math.floor(dur * 1000))
end

local eq_bands = cfg.eq_bands or { 60, 250, 1000, 4000, 12000 }

local function eq_label(freq)
    freq = tonumber(freq) or 0
    if freq >= 1000 then
        return string.format("%g kHz", freq / 1000)
    end
    return string.format("%g Hz", freq)
end

-- Approximate mpg123 fallback mapping: MPEG subband width is ~689 Hz at
-- 44.1 kHz.  The DSP engine uses the exact frequencies from cfg.eq_bands.
local function eq_subbands(freq)
    local center = math.floor((tonumber(freq) or 0) / 689.0625)
    if center < 0 then center = 0 end
    if center > 31 then center = 31 end
    local out = {}
    for b = math.max(0, center - 1), math.min(31, center + 1) do
        out[#out + 1] = b
    end
    return out
end

local EQ_MAP = {}
for i = 1, 5 do EQ_MAP[i] = eq_subbands(eq_bands[i]) end
local eq_values = {}
for i = 1, 5 do
    eq_values[i] = math.floor(((cfg.eq_gain and cfg.eq_gain[i]) or 0) * 100)
end

local function ensure_player()
    if not player:is_running() then
        local job, err = player:start()
        if not job then
            status_text = "mpg123: " .. tostring(err)
            return false
        end
        player:set_volume(cfg.volume or 80)
        if player.set_eq_band then
            for i = 1, 5 do
                player:set_eq_band(i, (eq_values[i] or 0) / 100.0)
            end
        else
            for i = 1, 5 do
                local mult = 10 ^ ((eq_values[i] or 0) / 2000.0)
                for _, band in ipairs(EQ_MAP[i] or {}) do
                    player:set_eq(band, mult)
                end
            end
        end
    end
    return true
end

local function play_index(i)
    if #tracks == 0 then return end
    index = ((i - 1) % #tracks) + 1
    if not ensure_player() then status_update(); return end
    player:load(tracks[index].path)
    if player.engine_type == "dsp" then
        analysis.stop(current_analysis)
        current_analysis = nil
        for i = 1, 5 do
            player:set_eq_band(i, (eq_values[i] or 0) / 100.0)
        end
    else
        analysis.stop(current_analysis)
        current_analysis = analysis.prepare(cfg, tracks[index].path, function(h)
            if h.error then status_text = "analysis: " .. h.error end
        end)
    end
    status_update()
end

function on_play()
    if #tracks == 0 then status_update(); return end
    if not ensure_player() then status_update(); return end
    if player.state == "playing" or player.state == "paused" then
        player:toggle()
    else
        play_index(index)
    end
    status_update()
end

function on_stop()
    player:stop()
    status_update()
end

function on_next()
    play_index(index + 1)
end

function on_prev()
    play_index(index - 1)
end

function on_prev_album()
    play_index(playlist_library.prev_album(tracks, index))
end

function on_next_album()
    play_index(playlist_library.next_album(tracks, index))
end

function on_seek()
    local ms = tonumber(gui.get("seek", "positionMs")) or 0
    player:seek(ms / 1000)
end

function on_volume()
    local v = tonumber(gui.get("vol", "value")) or 80
    player:set_volume(v)
end

local function apply_eq(i)
    local centibel = tonumber(gui.get("eq" .. i, "value")) or 0
    eq_values[i] = centibel
    local mult = 10 ^ (centibel / 2000.0)
    if mult < 0.1 then mult = 0.1 end
    if mult > 4.0 then mult = 4.0 end
    if player.set_eq_band then
        player:set_eq_band(i, centibel / 100.0)
    else
        for _, band in ipairs(EQ_MAP[i] or {}) do
            if player:is_running() then player:set_eq(band, mult) end
        end
    end
end

function on_eq_1() apply_eq(1) end
function on_eq_2() apply_eq(2) end
function on_eq_3() apply_eq(3) end
function on_eq_4() apply_eq(4) end
function on_eq_5() apply_eq(5) end

local library_job = nil

local function reload_library()
    local old_path = tracks[index] and tracks[index].path
    local new_tracks, info = load_queue()
    tracks = new_tracks or {}
    lib_info = info
    if old_path then
        for i, t in ipairs(tracks) do
            if t.path == old_path then index = i; break end
        end
    end
    if #tracks == 0 then index = 1 else
        index = math.max(1, math.min(index, #tracks))
    end
    status_update()
end

function on_library()
    if library_job and script.is_running(library_job) then return end
    _G.on_library_event = function(ev)
        if ev and ev.kind == "exit" then
            library_job = nil
            reload_library()
        end
    end
    local argv = { "bash", root .. "/apps/mp3player/run_library.sh" }
    local cp = os.getenv("MP3PLAYER_CONFIG")
    if cp and cp ~= "" then argv[#argv + 1] = cp end
    library_job = script.start_argv(argv, { on_event = "on_library_event" })
    if not library_job then
        status_text = "library: start failed"
        status_update()
    end
end

function on_quit()
    if library_job and script.is_running(library_job) then script.stop(library_job) end
    player:quit()
    xtappexitflag()
end

local ui = string.format([[
(WAudioChassis :id "chassis" :title "XTCW MP3 Player" :width %d :height %d :chassisWidth %d :chassisHeight %d
    (WVfdDisplay :id "vfd" :x 36 :y 36 :width 1088 :height 178
            :album "NO ALBUM" :title "NO TRACKS" :playState "IDLE")

    (WModulePanel :id "vupanel_l" :x 36 :y 218 :width 508 :height 96 :showScrews 0
        (WVuMeter :id "vu_l" :label "LEFT" :x 18 :y 16 :width 472 :height 64
                :xftFont "DejaVu Sans Condensed-9" :transparent 1))
    (WModulePanel :id "vupanel_r" :x 576 :y 218 :width 488 :height 96 :showScrews 0
        (WVuMeter :id "vu_r" :label "RIGHT" :x 18 :y 16 :width 452 :height 64
                :xftFont "DejaVu Sans Condensed-9" :transparent 1))

    (WModulePanel :id "volpanel" :x 1072 :y 218 :width 80 :height 196
            :showScrews 0
        (EqFader :id "vol" :label "VOL" :x 8 :y 8 :width 64 :height 180
             :value %d :minValue 0 :maxValue 100 :tickStep 20 :showDb 0 :showTicks 0 :hideEndLabels 1
             :scaleFont "DejaVu Sans-8" :xftFont "DejaVu Sans-10" :background "#141210"
             :labelColor "#d8b06a" :tickColor "#141210" :transparent 1
             :thumbColor "#ff8c1a" :thumbBorder "#111111" :callback "LUA(on_volume)"))

    (Wbutton :id "library" :label "LIBRARY" :x 1072 :y 420 :width 80 :height 24
             :fontSize 9 :bg_norm "#141210" :fg_norm "#d8b06a" :callback "LUA(on_library)")

    (WTransportButton :id "prevalbum" :icon "prev_album" :label "PREV ALBUM"
             :x 44 :y 330 :width 84 :height 72 :panelColor "#141210" :hoverColor "#24272a" :callback "LUA(on_prev_album)")
    (WTransportButton :id "prev" :icon "prev" :label "PREV TRACK"
             :x 136 :y 330 :width 84 :height 72 :panelColor "#141210" :hoverColor "#24272a" :callback "LUA(on_prev)")
    (WTransportButton :id "playbtn" :icon "play" :label "PLAY/PAUSE"
             :x 228 :y 330 :width 84 :height 72 :panelColor "#141210" :hoverColor "#24272a" :callback "LUA(on_play)")
    (WTransportButton :id "stop" :icon "stop" :label "STOP"
             :x 320 :y 330 :width 84 :height 72 :panelColor "#141210" :hoverColor "#24272a" :callback "LUA(on_stop)")
    (WTransportButton :id "next" :icon "next" :label "NEXT TRACK"
             :x 412 :y 330 :width 84 :height 72 :panelColor "#141210" :hoverColor "#24272a" :callback "LUA(on_next)")
    (WTransportButton :id "nextalbum" :icon "next_album" :label "NEXT ALBUM"
             :x 504 :y 330 :width 84 :height 72 :panelColor "#141210" :hoverColor "#24272a" :callback "LUA(on_next_album)")

    (WModulePanel :id "seekpanel" :x 596 :y 324 :width 472 :height 84
            :showScrews 0
        (WSeekBar :id "seek" :x 8 :y 6 :width 456 :height 72
            :xftFont "DejaVu Sans-10" :showBorder 0 :callback "LUA(on_seek)"))

    (WModulePanel :id "eqpanel" :x 36 :y 448 :width 708 :height 282 :showScrews 0
        (EqFader :id "eq1" :label "%s" :x 20 :y 18 :width 110 :height 250
                 :value 0 :tickStep 300 :scaleFont "DejaVu Sans Condensed-7"
                 :xftFont "DejaVu Sans Condensed-9" :showTicks 0 :labelOnTop 1 :transparent 1
                 :background "#141210" :labelColor "#d8b06a" :tickColor "#6b5a3a"
                 :thumbColor "#ff8c1a" :thumbBorder "#111111" :callback "LUA(on_eq_1)")
        (EqFader :id "eq2" :label "%s" :x 160 :y 18 :width 110 :height 250
                 :value 0 :tickStep 300 :scaleFont "DejaVu Sans Condensed-7"
                 :xftFont "DejaVu Sans Condensed-9" :showTicks 0 :labelOnTop 1 :transparent 1
                 :background "#141210" :labelColor "#d8b06a" :tickColor "#6b5a3a"
                 :thumbColor "#ff8c1a" :thumbBorder "#111111" :callback "LUA(on_eq_2)")
        (EqFader :id "eq3" :label "%s" :x 300 :y 18 :width 110 :height 250
                 :value 0 :tickStep 300 :scaleFont "DejaVu Sans Condensed-7"
                 :xftFont "DejaVu Sans Condensed-9" :showTicks 0 :labelOnTop 1 :transparent 1
                 :background "#141210" :labelColor "#d8b06a" :tickColor "#6b5a3a"
                 :thumbColor "#ff8c1a" :thumbBorder "#111111" :callback "LUA(on_eq_3)")
        (EqFader :id "eq4" :label "%s" :x 440 :y 18 :width 110 :height 250
                 :value 0 :tickStep 300 :scaleFont "DejaVu Sans Condensed-7"
                 :xftFont "DejaVu Sans Condensed-9" :showTicks 0 :labelOnTop 1 :transparent 1
                 :background "#141210" :labelColor "#d8b06a" :tickColor "#6b5a3a"
                 :thumbColor "#ff8c1a" :thumbBorder "#111111" :callback "LUA(on_eq_4)")
        (EqFader :id "eq5" :label "%s" :x 580 :y 18 :width 110 :height 250
                 :value 0 :tickStep 300 :scaleFont "DejaVu Sans Condensed-7"
                 :xftFont "DejaVu Sans Condensed-9" :showTicks 0 :labelOnTop 1 :transparent 1
                 :background "#141210" :labelColor "#d8b06a" :tickColor "#6b5a3a"
                 :thumbColor "#ff8c1a" :thumbBorder "#111111" :callback "LUA(on_eq_5)")
    )

    (WSpectrum :id "spec" :x 820 :y 448 :width 300 :height 282
            :bands 24 :xftFont "DejaVu Sans Condensed-9"
            :levels "-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50")

)
]], cw, ch, cw, ch, cfg.volume or 80,
    eq_label(eq_bands[1]), eq_label(eq_bands[2]), eq_label(eq_bands[3]),
    eq_label(eq_bands[4]), eq_label(eq_bands[5]))

local ok, err = lui.run(ui)
if not ok then error("mp3player: " .. tostring(err)) end

status_update()

local function set_meters(levels, rms, peak)
    if not levels then return end
    local parts = {}
    for i, v in ipairs(levels) do
        parts[i] = string.format("%.0f", v)
    end
    local spec = table.concat(parts, ",")
    if spec ~= last_spec then
        last_spec = spec
        gui.set("spec", "levels", spec)
    end
    local function set_vu(id, field, value)
        local key = id .. "." .. field
        if last_vu[key] ~= value then
            last_vu[key] = value
            gui.set(id, field, value)
        end
    end
    -- The bar follows the peak envelope.  RMS alone sits far below the
    -- peak marker on quiet material and looks like a half-filled meter.
    local l_l = math.max((rms and rms[1]) or -120, (peak and peak[1]) or -120)
    local l_r = math.max((rms and rms[2]) or -120, (peak and peak[2]) or -120)
    set_vu("vu_l", "level", math.floor(l_l * 10))
    set_vu("vu_r", "level", math.floor(l_r * 10))
    set_vu("vu_l", "peak", math.floor(((peak and peak[1]) or -60) * 10))
    set_vu("vu_r", "peak", math.floor(((peak and peak[2]) or -60) * 10))
end

local function tick()
    if player.update then player:update() end
    status_update()
    if player.levels then
        set_meters(player.levels, player.rms, player.peak)
    elseif current_analysis and current_analysis.ready and
       (player.state == "playing" or player.state == "paused") then
        local r = analysis.spectrum(current_analysis, player.position, cfg)
        if r then set_meters(r.levels, r.rms, r.peak) end
    end
    xtapptimeout(100, tick)
end
xtapptimeout(200, tick)

if os.getenv("MP3PLAYER_AUTOPLAY") == "1" then
    xtapptimeout(200, on_play)
end

local autoclose = tonumber(os.getenv("MP3PLAYER_AUTOCLOSE") or "0") or 0
if autoclose > 0 then
    xtapptimeout(autoclose, function()
        player:quit()
        xtappexitflag()
    end)
end
