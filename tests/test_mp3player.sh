#!/bin/bash
# Headless smoke test for the mp3player chassis + C-FFT first slice.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/mp3player_test.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

fail=0
ok()  { echo "ok - $1"; }
bad() { echo "FAIL - $1"; fail=1; }

if ! make -C "$ROOT/apps/mp3player/c" >"$TMP/fft-build.log" 2>&1; then
    bad "build mp3fft.so"
    tail -20 "$TMP/fft-build.log" >&2
else
    ok "build mp3fft.so"
fi

if [ ! -x "$ROOT/commander/commander_runner" ]; then
    if ( cd "$ROOT" && make commander ) >"$TMP/commander-build.log" 2>&1; then
        ok "build commander_runner"
    else
        bad "build commander_runner"
        tail -20 "$TMP/commander-build.log" >&2
        exit 1
    fi
fi

ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}" \
MP3PLAYER_ROOT="$ROOT" \
MP3PLAYER_CONFIG="$ROOT/apps/mp3player/config.lua" \
MP3PLAYER_AUTOCLOSE=800 \
timeout 20 "$ROOT/run_lui.sh" "$ROOT/apps/mp3player/main.lua" -Tracelevel 1 \
    >"$TMP/run.log" 2>&1
rc=$?

if [ "$rc" -eq 0 ]; then
    ok "app exits cleanly"
else
    bad "app exit status $rc"
    tail -30 "$TMP/run.log" >&2
fi

if grep -qiE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log"; then
    bad "no fatal pattern in log"
    grep -iE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log" | head -5 >&2
else
    ok "no fatal pattern in log"
fi

if grep -q 'chassis: expose' "$TMP/run.log"; then
    ok "chassis rendered"
else
    bad "chassis expose missing"
    tail -20 "$TMP/run.log" >&2
fi

if [ "$fail" -ne 0 ]; then
    echo "test_mp3player: FAILED" >&2
    exit 1
fi
echo "test_mp3player: PASSED"
