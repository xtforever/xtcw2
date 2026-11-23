#!/bin/bash
# Headless smoke test for the widget reference browser (tools/widget_help.lua).
#
# Types a query into the search field and checks that the window renders
# content without Lua/ASan errors.  This also covers the WListView borrowed
# columnTitles regression: the browser sets :columnTitles from Lua.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/widget_help.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

if ! command -v xvfb-run >/dev/null 2>&1 || ! command -v xdotool >/dev/null 2>&1 ||
   ! command -v import >/dev/null 2>&1; then
    echo "test_widget_help: SKIPPED (xvfb-run/xdotool/import not installed)"
    exit 0
fi

if [ ! -x "$ROOT/commander/commander_runner" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

xvfb-run --auto-servernum bash -c '
    root="$1"; log="$2"; shot="$3"
    cd "$root" || exit 1
    timeout 10 ./run_lui.sh tools/widget_help.lua >"$log" 2>&1 &
    app=$!
    sleep 4
    xdotool mousemove 400 85 click 1
    sleep 0.3
    xdotool type --delay 40 "EqFader"
    sleep 1.5
    xdotool mousemove 600 500 click 1
    sleep 0.3
    xdotool key Tab
    sleep 0.3
    xdotool key Return
    sleep 1.5
    import -window root "$shot"
    wait "$app"
' bash "$ROOT" "$TMP/app.log" "$TMP/shot.png"

rc=0
if grep -qE 'AddressSanitizer|ABORTING|LUI load error|Syntax Error|Execution error' "$TMP/app.log"; then
    echo "FAIL: widget_help reported an error"
    grep -E 'AddressSanitizer|ABORTING|LUI load error|Syntax Error|Execution error' "$TMP/app.log" | head -5
    rc=1
fi

if [ ! -s "$TMP/shot.png" ] || [ "$(stat -c %s "$TMP/shot.png")" -lt 10000 ]; then
    echo "FAIL: widget_help window rendered blank"
    rc=1
fi

if [ "$rc" -eq 0 ]; then
    echo "test_widget_help PASSED"
fi
exit "$rc"
