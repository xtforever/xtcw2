-- Recursive music scan, playlist loading and ID3-based sorting.

local id3 = require("id3")
local m3u = require("m3u")

local M = {}

local function shquote(s)
    return "'" .. tostring(s):gsub("'", "'\\''") .. "'"
end

local function exists(path)
    local f = io.open(path, "rb")
    if f then f:close(); return true end
    return false
end

local function dirname(path)
    return (path:match("^(.*)/[^/]*$")) or "."
end

local function basename(path)
    return (path:match("([^/]+)$")) or path
end

local function find_files(dir, args)
    local cmd = "find " .. shquote(dir) .. " " .. args .. " 2>/dev/null"
    local p = io.popen(cmd, "r")
    if not p then return {} end
    local out = {}
    for line in p:lines() do
        if line ~= "" then out[#out + 1] = line end
    end
    p:close()
    table.sort(out)
    return out
end

function M.scan(dir, extensions)
    if not dir or dir == "" then return {} end
    extensions = extensions or { "mp3" }
    local pats = {}
    for _, ext in ipairs(extensions) do
        pats[#pats + 1] = "-iname " .. shquote("*." .. ext)
    end
    local args = "-type f \\( " .. table.concat(pats, " -o ") .. " \\) -print"
    return find_files(dir, args)
end

function M.find_m3u(dir)
    if not dir or dir == "" then return nil end
    local files = find_files(dir,
        "-maxdepth 2 -type f \\( -iname '*.m3u' -o -iname '*.m3u8' \\) -print")
    return files[1]
end

local function entry_from_path(path, tags)
    tags = tags or {}
    local t = {
        path = path,
        file = basename(path),
        title = tags.title or basename(path),
        artist = tags.artist or "Unknown Artist",
        album = tags.album or dirname(path):match("([^/]+)$") or "",
        albumartist = tags.albumartist or tags.artist or "Unknown Artist",
        track = tags.track or "",
        disc = tags.disc or "",
        year = tags.year or "",
        genre = tags.genre or "",
        duration = tags.duration,
        tags = tags,
    }
    return t
end

function M.read_tags(path)
    local ok, tags = pcall(id3.read, path)
    if ok and tags then return tags end
    return {}
end

local function cmp_num(a, b)
    local na = tonumber(a) or 0
    local nb = tonumber(b) or 0
    return na < nb
end

local function cmp_entry(a, b, mode)
    local function s(v) return tostring(v or ""):lower() end
    if mode == "album" then
        if s(a.albumartist) ~= s(b.albumartist) then return s(a.albumartist) < s(b.albumartist) end
        if s(a.album) ~= s(b.album) then return s(a.album) < s(b.album) end
        local da, db = id3.split_pair(a.disc), id3.split_pair(b.disc)
        if da ~= db then return (da or 0) < (db or 0) end
        local ta, tb = id3.split_pair(a.track), id3.split_pair(b.track)
        if ta ~= tb then return (ta or 0) < (tb or 0) end
        return s(a.title) < s(b.title)
    elseif mode == "artist" then
        if s(a.artist) ~= s(b.artist) then return s(a.artist) < s(b.artist) end
        if s(a.album) ~= s(b.album) then return s(a.album) < s(b.album) end
        local ta, tb = id3.split_pair(a.track), id3.split_pair(b.track)
        if ta ~= tb then return (ta or 0) < (tb or 0) end
        return s(a.title) < s(b.title)
    elseif mode == "title" then
        return s(a.title) < s(b.title)
    elseif mode == "filename" then
        return s(a.file) < s(b.file)
    end
    return false
end

function M.sort(entries, mode)
    if mode == "m3u" then return entries end
    table.sort(entries, function(a, b) return cmp_entry(a, b, mode or "album") end)
    return entries
end

function M.albums(entries)
    local groups = {}
    local last_key
    for i, e in ipairs(entries) do
        local key = tostring(e.albumartist or "") .. "\0" .. tostring(e.album or "")
        if key ~= last_key then
            groups[#groups + 1] = { key = key, album = e.album, artist = e.albumartist, first = i, last = i }
            last_key = key
        else
            groups[#groups].last = i
        end
    end
    return groups
end

function M.auto_path(cfg)
    local cache = (cfg.analysis and cfg.analysis.cache_dir) or
                  ((os.getenv("HOME") or ".") .. "/.cache/xtcw-mp3player")
    os.execute("mkdir -p " .. shquote(cache))
    return cache .. "/" .. (cfg.auto_playlist or "auto.m3u")
end

function M.load(cfg)
    local playlist_path = cfg.playlist
    if playlist_path == "auto" then playlist_path = nil end
    if not playlist_path then
        playlist_path = M.find_m3u(cfg.music_dir)
    end

    local entries = {}
    local created = false
    if playlist_path and exists(playlist_path) then
        local list, err = m3u.read(playlist_path)
        if not list then return nil, err end
        for _, e in ipairs(list) do
            if exists(e.path) then
                local tags = M.read_tags(e.path)
                local entry = entry_from_path(e.path, tags)
                entry.title = e.title ~= "" and e.title or entry.title
                entry.duration = e.duration or entry.duration
                entries[#entries + 1] = entry
            end
        end
    else
        local files = M.scan(cfg.music_dir, cfg.extensions)
        for _, path in ipairs(files) do
            entries[#entries + 1] = entry_from_path(path, M.read_tags(path))
        end
        created = true
    end

    if #entries == 0 then
        return nil, "no playable tracks in " .. tostring(cfg.music_dir)
    end

    M.sort(entries, cfg.sort or "album")

    if created then
        local auto = M.auto_path(cfg)
        local out = {}
        for _, e in ipairs(entries) do
            out[#out + 1] = {
                file = e.path,
                title = e.title,
                duration = e.duration,
            }
        end
        pcall(m3u.write, auto, out)
    end

    return entries, {
        playlist = playlist_path,
        created = created,
        albums = M.albums(entries),
    }
end

function M.next_album(entries, index)
    local groups = M.albums(entries)
    for _, g in ipairs(groups) do
        if g.last >= index then
            if g.first > index then return g.first end
            for _, h in ipairs(groups) do
                if h.first > g.last then return h.first end
            end
            return groups[1].first
        end
    end
    return 1
end

function M.prev_album(entries, index)
    local groups = M.albums(entries)
    local prev = groups[#groups].first
    for _, g in ipairs(groups) do
        if g.first >= index then return prev end
        prev = g.first
    end
    return prev
end

return M
