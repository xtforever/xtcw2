-- Thin Lua wrapper around the optional C module mp3fft.so.
-- Keeping this wrapper separate lets the UI start even when the module is
-- missing (spectrum off instead of a hard failure).

local M = {}

local ok, mod = pcall(require, "mp3fft")
M.available = ok and type(mod) == "table"
M.error = ok and nil or tostring(mod)
M.module = ok and mod or nil

function M.new(cfg)
    if not M.available then
        return nil, M.error
    end
    return M.module.new(cfg)
end

function M.process(analyzer, pcm_path, offset_bytes, channels)
    if not analyzer then
        return nil, "spectrum unavailable"
    end
    return analyzer:process_pcm_file(pcm_path, offset_bytes or 0, channels or 2)
end

return M
