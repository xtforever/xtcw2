-- mp3player default configuration.  This file is ordinary Lua code; a user
-- config can override it via MP3PLAYER_CONFIG.

local home = os.getenv("HOME") or "."

return {
    music_dir = home .. "/Musik",
    library_file = home .. "/.local/share/xtcw-mp3player/library.lua",
    library_playlist_dir = home .. "/.local/share/xtcw-mp3player/playlists",
    playlist = nil,
    auto_playlist = "auto.m3u",
    extensions = { "mp3" },
    sort = "album",
    start_track = 1,
    volume = 80,

    eq_bands = { 60, 250, 1000, 4000, 12000 },
    eq_gain = { 0, 0, 0, 0, 0 },

    spectrum = {
        bands = 24,
        min_db = -50,
        max_db = 0,
        fps = 25,
    },

    analysis = {
        rate = 22050,
        fft_size = 1024,
        cache_dir = home .. "/.cache/xtcw-mp3player",
        cache_max_mb = 512,
    },

    player = {
        engine = "dsp",              -- "dsp" | "mpg123"
        sink = "",                   -- "" = default Pulse sink
        command = "mpg123",
        output = "pulse",
        fallback_outputs = { "alsa", "dummy" },
    },

    chassis = {
        width = 1160,
        height = 770,
    },
}
