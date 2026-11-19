#!/bin/bash
# Smoke test for the shared lifecycle API: window-close registration and
# graceful_shutdown() from Lua.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/lifecycle.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

cat > "$TMP/lifecycle.lua" <<'LUA'
local gui = require('gui_xt')
local lui = require('lui')

local id1 = window_close_register(function() return true end)
local id2 = window_close_register(function() return true end)
print("ID1=" .. tostring(id1))
print("ID2=" .. tostring(id2))
window_close_unregister(id2)

local ok, err = lui.run([[
(window :id "w" :title "Lifecycle" :width 240 :height 120
  (label :label "Lifecycle"))
]])
if not ok then error("lifecycle: " .. tostring(err)) end

xtapptimeout(300, function()
    print("GRACEFUL")
    graceful_shutdown()
end)
LUA

cd "$ROOT/commander" || exit 1
ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}" \
timeout 25 xvfb-run --auto-servernum ./commander_runner \
    -Luafile "$TMP/lifecycle.lua" >"$TMP/log" 2>&1
rc=$?

fail=0
if [ "$rc" -ne 0 ]; then
    echo "FAIL - commander exited $rc"
    tail -20 "$TMP/log" | sed 's/^/    /'
    fail=1
fi
if ! grep -q '^ID1=1$' "$TMP/log"; then
    echo "FAIL - first close handler was not registered"
    fail=1
fi
if ! grep -q '^ID2=2$' "$TMP/log"; then
    echo "FAIL - second close handler was not registered"
    fail=1
fi
if ! grep -q '^GRACEFUL$' "$TMP/log"; then
    echo "FAIL - graceful_shutdown() did not run"
    fail=1
fi
if ! grep -q '^terminated normaly$' "$TMP/log"; then
    echo "FAIL - normal shutdown path did not complete"
    fail=1
fi
if grep -q 'still allocated' "$TMP/log"; then
    echo "FAIL - shutdown left MLS lists allocated"
    grep 'still allocated' "$TMP/log" | head -3 | sed 's/^/    /'
    fail=1
fi
if grep -q 'Lua CB Error\|Execution error\|Assertion' "$TMP/log"; then
    echo "FAIL - error in log"
    grep -E 'Lua CB Error|Execution error|Assertion' "$TMP/log" | head -3 | sed 's/^/    /'
    fail=1
fi

if [ "$fail" -eq 0 ]; then
    echo "test_lifecycle: PASSED"
    exit 0
fi
exit 1
