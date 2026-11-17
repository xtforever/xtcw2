#!/bin/bash
# Smoke test for the IconEditor demo: load the UI, draw a red diagonal via the
# plug-in test hook, save an XPM and verify the file.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/iconeditor.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

OUT="$TMP/icon.xpm"
cd "$ROOT/commander" || exit 1
ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}" \
ICONEDITOR_SMOKE="$OUT" \
timeout 25 xvfb-run --auto-servernum ./commander_runner \
    -Luafile ../demos/demo_iconeditor.lua >"$TMP/log" 2>&1
rc=$?

fail=0
if [ "$rc" -ne 0 ]; then
    echo "FAIL - commander exited $rc"
    tail -20 "$TMP/log" | sed 's/^/    /'
    fail=1
fi
if [ ! -s "$OUT" ]; then
    echo "FAIL - XPM output missing"
    tail -20 "$TMP/log" | sed 's/^/    /'
    fail=1
elif ! grep -q '^/\* XPM \*/' "$OUT"; then
    echo "FAIL - output is not an XPM file"
    fail=1
elif ! grep -q 'c #FF0000' "$OUT"; then
    echo "FAIL - saved XPM does not contain the red test stroke"
    fail=1
else
    echo "ok - saved XPM with the red test stroke"
fi

if [ "$fail" -eq 0 ]; then
    echo "test_iconeditor: PASSED"
    exit 0
fi
echo "test_iconeditor: FAILED"
exit 1
