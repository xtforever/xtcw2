#!/usr/bin/env lua5.3
-- Tests for lui/retex_file.lua (pure Lua, no X server needed).
--
--   lua5.3 tests/test_retex_file.lua
--
-- The test derives the repository root from arg[0] so it works from the repo
-- root as well as from tests/.

local script = arg and arg[0] or "tests/test_retex_file.lua"
local script_dir = script:match("^(.*)[/\\][^/\\]*$") or "."
local root = script_dir .. "/.."
package.path = root .. "/lui/?.lua;" .. package.path

local rf = require("retex_file")

local failures = 0
local function check(cond, msg)
    if cond then
        print("ok - " .. msg)
    else
        failures = failures + 1
        print("FAIL - " .. msg)
    end
end

local function write_file(path, data)
    local f = assert(io.open(path, "wb"))
    assert(f:write(data))
    assert(f:close())
end

local function read_file(path)
    local f = assert(io.open(path, "rb"))
    local data = f:read("*a")
    f:close()
    return data
end

local tmpdir = os.tmpname()
os.remove(tmpdir)
local mk = os.execute("mkdir -p '" .. tmpdir .. "'")
assert(mk == true or mk == 0, "cannot create temp dir " .. tmpdir)

-- --- load ---------------------------------------------------------------
do
    local p = tmpdir .. "/basic.retex"
    local src = "Hello \\bf bold\\rm  and \239\188\129 UTF-8.\n"
    write_file(p, src)
    local text, base = rf.load(p)
    check(text == src, "load preserves bytes")
    check(base == tmpdir, "load returns the file's directory")
end

do
    local p = tmpdir .. "/empty.retex"
    write_file(p, "")
    local text, base = rf.load(p)
    check(text == "", "load empty file")
    check(base == tmpdir, "load empty file returns base dir")
end

do
    local text, err = rf.load(tmpdir .. "/does-not-exist.retex")
    check(text == nil and type(err) == "string", "missing file returns nil, error")
end

do
    local p = tmpdir .. "/bom.retex"
    write_file(p, "\239\187\191with BOM")
    local text = rf.load(p)
    check(text == "with BOM", "leading UTF-8 BOM is stripped")
end

do
    local p = tmpdir .. "/nul.retex"
    write_file(p, "a\0b")
    local text, err = rf.load(p)
    check(text == nil and type(err) == "string", "NUL byte is rejected on load")
end

do
    local text, err = rf.load(nil)
    check(text == nil and type(err) == "string", "load(nil) returns an error")
    text, err = rf.load("")
    check(text == nil and type(err) == "string", "load('') returns an error")
end

-- --- save ---------------------------------------------------------------
do
    local p = tmpdir .. "/roundtrip.retex"
    local src = "line 1\r\n\tline 2 \195\164\195\182\195\188\n\\includesvg{a.svg}{1pt}{1pt}\n"
    local ok, err = rf.save(p, src)
    check(ok == true, "save succeeds")
    check(err == nil, "save returns no error")
    check(read_file(p) == src, "save preserves bytes")
    check(rf.load(p) == src, "save/load round-trip")
end

do
    local p = tmpdir .. "/space dir/my description.retex"
    local mk2 = os.execute("mkdir -p '" .. tmpdir .. "/space dir'")
    assert(mk2 == true or mk2 == 0)
    local ok = rf.save(p, "spaces")
    check(ok == true, "save path with spaces")
    local text, base = rf.load(p)
    check(text == "spaces", "load path with spaces")
    check(base == tmpdir .. "/space dir", "base dir with spaces")
end

do
    local p = tmpdir .. "/replace.retex"
    write_file(p, "old")
    local ok = rf.save(p, "new")
    check(ok == true and read_file(p) == "new", "save replaces an existing file")
    local tmp = io.open(p .. ".tmp", "rb")
    check(tmp == nil, "no temporary file is left behind")
    if tmp then tmp:close() end
end

do
    local ok, err = rf.save(tmpdir .. "/no/such/dir/x.retex", "x")
    check(ok == nil and type(err) == "string", "save into a missing directory fails")
end

do
    local p = tmpdir .. "/keep.retex"
    write_file(p, "old")
    local mkdir = os.execute("mkdir -p '" .. p .. ".tmp'")
    assert(mkdir == true or mkdir == 0)
    local ok, err = rf.save(p, "new")
    check(ok == nil and type(err) == "string", "save fails when the temp path is a directory")
    check(read_file(p) == "old", "existing file is preserved on save failure")
    os.execute("rmdir '" .. p .. ".tmp'")
end

do
    local ok, err = rf.save(nil, "x")
    check(ok == nil and type(err) == "string", "save(nil, ...) returns an error")
    ok, err = rf.save(tmpdir .. "/x.retex", nil)
    check(ok == nil and type(err) == "string", "save(..., nil) returns an error")
    ok, err = rf.save(tmpdir .. "/nul-save.retex", "a\0b")
    check(ok == nil and type(err) == "string", "save rejects NUL bytes")
end

-- --- include path resolution -------------------------------------------
do
    local s = rf.resolve_includes(
        "\\includesvg{icon.svg}{12pt}{12pt}", "/base")
    check(s == "\\includesvg{/base/icon.svg}{12pt}{12pt}",
          "relative include is resolved against base_dir")
end

do
    local s = rf.resolve_includes(
        "\\includesvg{/abs/icon.svg}{1pt}{1pt}", "/base")
    check(s == "\\includesvg{/abs/icon.svg}{1pt}{1pt}",
          "absolute include is unchanged")
end

do
    local s = rf.resolve_includes(
        "a \\includesvg{a.svg}{1}{1} b \\includesvg{b.svg}{2}{2}", "/base")
    check(s == "a \\includesvg{/base/a.svg}{1}{1} b \\includesvg{/base/b.svg}{2}{2}",
          "multiple includes are resolved")
end

do
    local s = "plain \\bf text without includes"
    check(rf.resolve_includes(s, "/base") == s,
          "text without includes is unchanged")
end

do
    local s = rf.resolve_includes(
        "\\includesvg{my icon.svg}{1}{1}", "/base dir")
    check(s == "\\includesvg{/base dir/my icon.svg}{1}{1}",
          "include path with spaces")
end

do
    local s = rf.resolve_includes(
        "\\includesvg{}{1}{1}", "/base")
    check(s == "\\includesvg{}{1}{1}", "empty include path is unchanged")
end

do
    local s = rf.resolve_includes(
        "\\includesvg{icon.svg}{1}{1}", ".")
    check(s == "\\includesvg{./icon.svg}{1}{1}",
          "base '.' yields ./path")
end

-- --- large file ---------------------------------------------------------
do
    local p = tmpdir .. "/large.retex"
    local src = string.rep("0123456789abcdef\n", 100000) -- ~1.7 MB
    local ok = rf.save(p, src)
    check(ok == true, "save large file")
    local text = rf.load(p)
    check(text == src, "load large file round-trip")
end

-- --- cleanup ------------------------------------------------------------
os.execute("rm -rf '" .. tmpdir .. "'")

if failures > 0 then
    print(string.format("test_retex_file: %d check(s) FAILED", failures))
    os.exit(1)
end
print("test_retex_file: PASSED")
os.exit(0)
