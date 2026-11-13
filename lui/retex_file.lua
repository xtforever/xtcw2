-- retex_file.lua — load/save .retex text files.
--
-- A .retex file is plain UTF-8 text using the re-tex markup understood by
-- Wretex/Wlabel (\bf, \it, \fontface{...}, \includesvg{...}{...}{...}, ...
-- and % comments).  There is no metadata header in this version: the file
-- body is passed to the renderer verbatim.
--
-- This module is GUI-free so it can be used by t1_menu, tools and tests.
--
--   local rf = require('retex_file')
--   local text, base = rf.load(path)          -- nil, err on failure
--   local ok, err = rf.save(path, text)       -- atomic temp+rename
--   local resolved = rf.resolve_includes(text, base)
--
-- Paths in \includesvg are resolved relative to the .retex file's directory
-- by resolve_includes(); absolute paths are left unchanged.

local M = {}

-- Directory part of path.  "foo.retex" -> ".", "/x.retex" -> "/",
-- "a/b/c.retex" -> "a/b".
local function dirname(path)
    local dir = path:match("^(.*)[/\\][^/\\]*$")
    if dir == nil then return "." end
    if dir == "" then return "/" end
    return dir
end

local function strip_bom(s)
    if s:sub(1, 3) == "\239\187\191" then
        return s:sub(4)
    end
    return s
end

-- Load a .retex file.  Returns text, base_dir on success, or nil, error.
function M.load(path)
    if type(path) ~= "string" or path == "" then
        return nil, "retex_file.load: invalid path"
    end

    local f, err = io.open(path, "rb")
    if not f then
        return nil, err or ("cannot open " .. path)
    end

    local text, rerr = f:read("*a")
    local cerr = f:close()
    if not text then
        return nil, rerr or ("cannot read " .. path)
    end
    if not cerr then
        return nil, "cannot close " .. path
    end
    if text:find("\0", 1, true) then
        return nil, "retex_file.load: NUL byte in " .. path
    end

    return strip_bom(text), dirname(path)
end

-- Write a .retex file atomically: write path.tmp, then rename over path.
-- Returns true on success, or nil, error.
function M.save(path, text)
    if type(path) ~= "string" or path == "" then
        return nil, "retex_file.save: invalid path"
    end
    if type(text) ~= "string" then
        return nil, "retex_file.save: text must be a string"
    end
    if text:find("\0", 1, true) then
        return nil, "retex_file.save: NUL byte in text"
    end

    local tmp = path .. ".tmp"
    local f, err = io.open(tmp, "wb")
    if not f then
        return nil, err or ("cannot write " .. tmp)
    end

    local wok, werr = f:write(text)
    local cerr = f:close()
    if not wok or not cerr then
        os.remove(tmp)
        return nil, werr or ("cannot write " .. tmp)
    end

    local rok, rerr = os.rename(tmp, path)
    if not rok then
        os.remove(tmp)
        return nil, rerr or ("cannot rename " .. tmp .. " to " .. path)
    end

    return true
end

-- Rewrite the filename argument of \includesvg{file}{w}{h} so that relative
-- paths are relative to base_dir.  Absolute paths are unchanged.
function M.resolve_includes(text, base_dir)
    if type(text) ~= "string" then return text end
    if type(base_dir) ~= "string" or base_dir == "" then base_dir = "." end

    return (text:gsub("(\\includesvg%s*{)([^}]*)(})",
        function(pre, path, post)
            if path == "" or path:sub(1, 1) == "/" then
                return pre .. path .. post
            end
            return pre .. base_dir .. "/" .. path .. post
        end))
end

return M
