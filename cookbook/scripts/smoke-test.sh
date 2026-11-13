#!/usr/bin/env bash
# cookbook/scripts/smoke-test.sh — headless start smoke test for one build mode.
#
#   ./smoke-test.sh debug
#   ./smoke-test.sh production
#
# Returns 0 on pass. Pass requires: no fatal pattern in the log AND the process
# is still alive after 3s AND a window named "luarunner" exists. The fatal
# pattern takes precedence over the "alive + window" check, because the host
# keeps the process alive even when the LUI script fails to load (see the
# `lui_bootstrap` behaviour noted in the plan).
#
# The build must already exist (run ./build.sh <mode> first).
set -uo pipefail

MODE="${1:-}"
case "$MODE" in
    debug|production) ;;
    *) echo "usage: $0 <debug|production>" >&2; exit 2 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN="$ROOT/commander/commander_runner"
LUA="../cookbook/app/main.lua"
THEME="../cookbook/themes/cookbook.hdf"
LOG="${TMPDIR:-/tmp}/cookbook-smoke-$MODE.log"

if [ ! -x "$BIN" ]; then
    echo "[smoke] $BIN not found; run scripts/build.sh $MODE first" >&2
    exit 2
fi

# Re-exec under a private Xvfb if no display is available, so both the app and
# xwininfo see the same DISPLAY.
if [ -z "${DISPLAY:-}" ] && [ -z "${COOKBOOK_IN_XVFB:-}" ]; then
    export COOKBOOK_IN_XVFB=1
    exec xvfb-run -a -s "-screen 0 1280x800x24" "$SCRIPT_DIR/$(basename "$0")" "$@"
fi

# AddressSanitizer can abort at startup on kernels with incompatible
# vm.mmap_rnd_bits. Disable leak detection and, if possible, disable ASLR.
RUNNER=()
if [ "$MODE" = production ]; then
    : # no sanitizer
else
    export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
    if command -v setarch >/dev/null 2>&1 && setarch "$(uname -m)" -R true >/dev/null 2>&1; then
        RUNNER=(setarch "$(uname -m)" -R)
    fi
fi

ERR_PATTERN='Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted'

cleanup() {
    if [ -n "${APP_PID:-}" ]; then
        kill -9 "$APP_PID" 2>/dev/null || true
        pkill -9 -P "$APP_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

echo "[smoke] mode=$MODE, launching app (DISPLAY=${DISPLAY:-unset})"
cd "$ROOT/commander"
"${RUNNER[@]}" timeout 30 "$BIN" -Luafile "$LUA" -themeFile "$THEME" -theme default >"$LOG" 2>&1 &
APP_PID=$!

sleep 3

alive=0
kill -0 "$APP_PID" 2>/dev/null && alive=1

window=0
win_geom=""
if [ -n "${DISPLAY:-}" ] && command -v xwininfo >/dev/null 2>&1; then
    xwininfo -root -tree >"$LOG.tree" 2>&1 || true
    # Toplevel app window line, e.g.:
    #   0x200022 "commander_runner": ("commander_runner" "luarunner")  600x400+0+0  +0+0
    # Assert the window has a real (positive) size, not a 0-height/0-width
    # shell left behind by a failed geometry negotiation.
    win_geom=$(grep -m1 '"luarunner")' "$LOG.tree" \
        | sed -E 's/.*"luarunner"\)[[:space:]]+([0-9]+x[0-9]+).*/\1/')
    case "$win_geom" in
        [0-9]*x[0-9]*) ;;
        *) win_geom="" ;;
    esac
    if [ -n "$win_geom" ]; then
        win_w=${win_geom%x*}
        win_h=${win_geom#*x}
        if [ "${win_w:-0}" -gt 0 ] && [ "${win_h:-0}" -gt 0 ]; then
            window=1
        fi
    fi
fi

cleanup
trap - EXIT INT TERM

fatal=0
if grep -qiE "$ERR_PATTERN" "$LOG"; then
    fatal=1
fi

if [ "$fatal" -eq 1 ]; then
    echo "[smoke] FAIL ($MODE): fatal pattern in log:"
    grep -iE "$ERR_PATTERN" "$LOG" | head -5 | sed 's/^/    /'
    exit 1
fi

if [ "$alive" -eq 1 ] && [ "$window" -eq 1 ]; then
    echo "[smoke] PASS ($MODE): process alive, window ${win_geom} present"
    exit 0
fi

echo "[smoke] FAIL ($MODE): alive=$alive window=$window geom=${win_geom:-none} (see $LOG)"
tail -n 10 "$LOG" | sed 's/^/    /'
exit 1
