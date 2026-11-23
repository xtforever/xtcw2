-- Standalone music-library GUI.
--
-- Album = M3U playlist; the alias is the displayed album name.  This module
-- does not load the audio engine and can run independently of the player.

local root = os.getenv("MP3PLAYER_ROOT")
if not root or root == "" then root = "." end

package.path = root .. "/apps/mp3player/?.lua;" .. package.path

local lui = require("lui")
local gui = require("gui_xt")
local plib = require("playlist_library")

local cfg = require("config")
local cfg_path = os.getenv("MP3PLAYER_CONFIG")
if cfg_path and cfg_path ~= "" then
    local chunk, err = loadfile(cfg_path)
    if not chunk then
        error("library: cannot load config " .. cfg_path .. ": " .. tostring(err))
    end
    local user = chunk()
    if type(user) == "table" then
        for k, v in pairs(user) do cfg[k] = v end
    end
end

local entries = {}
local selected = nil

local function status(msg)
    gui.set("status", "label", tostring(msg or ""))
end

local function entry_at(i)
    return entries[i]
end

local function refresh_tracks()
    local rows = {}
    if selected then
        local tracks, warnings = plib.tracks(cfg, selected)
        for i, t in ipairs(tracks) do
            rows[#rows + 1] = string.format("%d\t%s\t%s\t%s\t%s",
                i, t.title or "", t.artist or "", t.album or "", t.path or "")
        end
        gui.set_string_list("tracks", "rows", rows)
        if #warnings > 0 then
            status(string.format("%d track(s), %d warning(s): %s",
                #tracks, #warnings, warnings[1]))
        else
            status(string.format("%d track(s) in %s", #tracks, selected.alias))
        end
        gui.set("alias", "label", selected.alias or "")
    else
        gui.set_string_list("tracks", "rows", {})
        status("Select an album or add a directory.")
    end
end

local function refresh(keep_selection)
    entries = plib.entries(cfg)
    local rows = {}
    for i, e in ipairs(entries) do
        rows[#rows + 1] = string.format("%s\t%d\t%s\t%s",
            e.alias or "", e.track_count or 0, e.source_dir or "",
            e.generated and "generated" or "m3u")
    end
    gui.set_string_list("albums", "rows", rows)

    local idx = 1
    if keep_selection and selected then
        for i, e in ipairs(entries) do
            if e.playlist == selected.playlist then idx = i; break end
        end
    end
    if #entries == 0 then
        selected = nil
        gui.set("albums", "selected", -1)
    else
        selected = entries[idx]
        gui.set("albums", "selected", idx - 1)
    end
    refresh_tracks()
end

function on_album_select()
    local idx = tonumber(gui.get("albums", "selected")) or -1
    selected = entry_at(idx + 1)
    refresh_tracks()
end

function on_add()
    local path = tostring(gui.get("path", "label") or "")
    if path == "" then
        status("Enter a music directory first.")
        return
    end
    status("Scanning " .. path .. " …")
    local summary, err = plib.add_directory(cfg, path)
    if not summary then
        status("Add failed: " .. tostring(err))
        return
    end
    refresh(false)
    status(string.format("Added %d, updated %d, generated %d, warnings %d",
        summary.added, summary.updated, summary.generated, #summary.warnings))
end

function on_rescan()
    status("Rescanning library …")
    local ok, err = plib.rescan(cfg)
    if not ok then status("Rescan failed: " .. tostring(err)); return end
    refresh(true)
    status("Rescan complete.")
end

function on_rename()
    if not selected then status("Select an album first."); return end
    local alias = tostring(gui.get("alias", "label") or "")
    if alias == "" then status("Alias must not be empty."); return end
    local ok, result = plib.rename(cfg, selected.playlist, alias)
    if not ok then status("Rename failed: " .. tostring(result)); return end
    refresh(true)
    status("Alias: " .. tostring(result))
end

function on_remove()
    if not selected then status("Select an album first."); return end
    local alias = selected.alias
    local playlist = selected.playlist
    local generated = selected.generated
    local ok, err = plib.remove(cfg, playlist, generated)
    if not ok then status("Remove failed: " .. tostring(err)); return end
    selected = nil
    refresh(false)
    status("Removed " .. tostring(alias))
end

function on_close()
    xtappexitflag()
end

local ui = [[
(window :id "win" :title "XTCW Music Library" :width 1000 :height 680
  (grid :id "main" :weightx 1 :weighty 1
    (Wlabel :id "head" :label "MUSIC LIBRARY  ·  album = M3U playlist"
            :gridx 0 :gridy 0 :gridWidth 4 :weightx 1 :weighty 0 :fill 3
            :fontSize 18 :fontFace "DejaVu Sans" :fg_norm "#8ff7ff" :bg_norm "#141210")
    (WListView :id "albums"
       :gridx 0 :gridy 1 :gridWidth 4 :weightx 1 :weighty 1 :fill 3
       :columnCount 4
       :columnTitles "Album / Alias\tTracks\tSource\tType"
       :columnWidths "0,70,420,100"
       :columnAligns "0,2,0,1"
       :showHeader 1 :gridLines 1 :alternateRows 1 :visibleRows 8
       :on-row-activated "LUA(on_album_select)")
    (WListView :id "tracks"
       :gridx 0 :gridy 2 :gridWidth 4 :weightx 1 :weighty 1 :fill 3
       :columnCount 5
       :columnTitles "#\tTitle\tArtist\tAlbum\tPath"
       :columnWidths "40,260,160,160,0"
       :columnAligns "2,0,0,0,0"
       :showHeader 1 :gridLines 1 :alternateRows 1 :visibleRows 8)
    (WLineEdit :id "path" :label "" :placeholder "Music directory to add…"
       :gridx 0 :gridy 3 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (Wbutton :id "add" :label "ADD DIRECTORY"
       :gridx 2 :gridy 3 :weightx 0 :weighty 0 :fill 1 :callback "LUA(on_add)")
    (Wbutton :id "rescan" :label "RESCAN"
       :gridx 3 :gridy 3 :weightx 0 :weighty 0 :fill 1 :callback "LUA(on_rescan)")
    (WLineEdit :id "alias" :label "" :placeholder "Album alias…"
       :gridx 0 :gridy 4 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (Wbutton :id "rename" :label "RENAME"
       :gridx 2 :gridy 4 :weightx 0 :weighty 0 :fill 1 :callback "LUA(on_rename)")
    (Wbutton :id "remove" :label "REMOVE"
       :gridx 3 :gridy 4 :weightx 0 :weighty 0 :fill 1 :callback "LUA(on_remove)")
    (Wlabel :id "status" :label "Add a directory or select an album."
            :gridx 0 :gridy 5 :gridWidth 3 :weightx 1 :weighty 0 :fill 3
            :fontSize 12 :fontFace "DejaVu Sans Mono" :fg_norm "#d8b06a" :bg_norm "#141210")
    (Wbutton :id "close" :label "CLOSE"
       :gridx 3 :gridy 5 :weightx 0 :weighty 0 :fill 1 :callback "LUA(on_close)")
  ))
]]

local ok, err = lui.run(ui)
if not ok then error("library_ui: " .. tostring(err)) end
refresh(false)

local autoclose = tonumber(os.getenv("MP3PLAYER_AUTOCLOSE") or "0") or 0
if autoclose > 0 then
    xtapptimeout(autoclose, function() xtappexitflag() end)
end
