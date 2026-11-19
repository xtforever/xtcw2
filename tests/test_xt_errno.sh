#!/bin/bash
# Verify the errno-style error channel used by xtaction(): C actions set a
# code/message, xtaction() clears it before the call and returns both.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/xt_errno.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

cat > "$TMP/errno.lua" <<'LUA'
local gui = require('gui_xt')
local lui = require('lui')

local ok, err = lui.run([[
(window :id "w" :title "Errno" :width 300 :height 200
  (label :label "errno test"))
]])
if not ok then error("xt_errno: " .. tostring(err)) end

local function show(name, rc, msg)
    print(name .. "_RC=" .. tostring(rc) .. " MSG=" .. tostring(msg))
end

xtapptimeout(200, function()
    local rc, msg = xtaction("commander_runner", "iconeditor_save",
                             "/nonexistent/dir/x.xpm")
    show("SAVE", rc, msg)

    rc, msg = xtaction("commander_runner", "iconeditor_set_tool", "bogus")
    show("TOOL", rc, msg)

    rc, msg = xtaction("commander_runner", "iconeditor_set_color", "zzz")
    show("COLOR", rc, msg)

    rc, msg = xtaction("commander_runner", "iconeditor_undo")
    show("UNDO", rc, msg)

    rc, msg = xtaction("commander_runner", "iconeditor_resize", "999", "32")
    show("RESIZE", rc, msg)

    local okpath = os.getenv("ERRNO_OK_PATH") or ""
    if okpath ~= "" then
        rc, msg = xtaction("commander_runner", "iconeditor_save", okpath)
        show("OKSAVE", rc, msg)
    end

    graceful_shutdown()
end)
LUA

OKPATH="$TMP/ok.xpm"
cd "$ROOT/commander" || exit 1
ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}" \
ERRNO_OK_PATH="$OKPATH" \
timeout 25 xvfb-run --auto-servernum ./commander_runner \
    -Luafile "$TMP/errno.lua" >"$TMP/log" 2>&1
rc=$?

fail=0
check() {
    if ! grep -q "$1" "$TMP/log"; then
        echo "FAIL - expected: $1"
        fail=1
    fi
}

if [ "$rc" -ne 0 ]; then
    echo "FAIL - commander exited $rc"
    tail -20 "$TMP/log" | sed 's/^/    /'
    fail=1
fi

check '^SAVE_RC=4 MSG=iconeditor_save: cannot open'
check '^TOOL_RC=1 MSG=iconeditor_set_tool: unknown tool'
check '^COLOR_RC=1 MSG=iconeditor_set_color: invalid colour'
check '^UNDO_RC=7 MSG=iconeditor_undo: nothing to undo'
check '^RESIZE_RC=8 MSG=iconeditor_resize: size must be 4\.\.64'
check '^OKSAVE_RC=0 MSG=$'
check '^terminated normaly$'

if [ ! -s "$OKPATH" ]; then
    echo "FAIL - successful save did not create $OKPATH"
    fail=1
elif ! grep -q '^/\* XPM \*/' "$OKPATH"; then
    echo "FAIL - successful save did not write an XPM file"
    fail=1
fi

if [ "$fail" -eq 0 ]; then
    echo "test_xt_errno: PASSED"
    exit 0
fi
exit 1
