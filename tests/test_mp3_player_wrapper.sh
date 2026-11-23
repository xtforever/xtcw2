#!/bin/bash
# Headless test: mpg123 -R through the minimal AdmPanel script wrapper.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/mp3player_wrapper.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

fail=0
ok()  { echo "ok - $1"; }
bad() { echo "FAIL - $1"; fail=1; }

if ! command -v mpg123 >/dev/null 2>&1; then
    echo "test_mp3_player_wrapper: SKIPPED (mpg123 missing)"
    exit 0
fi

MP3="$TMP/silence.mp3"
if command -v ffmpeg >/dev/null 2>&1; then
    if ! ffmpeg -hide_banner -loglevel error -y \
        -f lavfi -i anullsrc=r=44100:cl=stereo -t 2 -q:a 9 "$MP3" \
        >"$TMP/ffmpeg.log" 2>&1; then
        echo "test_mp3_player_wrapper: SKIPPED (ffmpeg mp3 encoder unavailable)"
        exit 0
    fi
else
    echo "test_mp3_player_wrapper: SKIPPED (ffmpeg missing)"
    exit 0
fi

if [ ! -x "$ROOT/commander/commander_runner" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

OUTPUT="dummy"
if command -v pactl >/dev/null 2>&1 && pactl info >/dev/null 2>&1; then
    OUTPUT="pulse"
fi

ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}" \
MP3PLAYER_ROOT="$ROOT" \
MP3PLAYER_TEST_OUTPUT="$OUTPUT" \
MP3PLAYER_TEST_MP3="$MP3" \
MP3PLAYER_TEST_OUT="$TMP/result.txt" \
timeout 25 "$ROOT/run_lui.sh" "$ROOT/tests/test_mp3_player_wrapper.lua" \
    >"$TMP/run.log" 2>&1
rc=$?

if [ "$rc" -ne 0 ]; then
    bad "probe exit status $rc"
    tail -30 "$TMP/run.log" >&2
fi
if grep -qiE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log"; then
    bad "no fatal pattern in log"
    grep -iE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log" | head -5 >&2
else
    ok "no fatal pattern in log"
fi

if [ ! -s "$TMP/result.txt" ]; then
    bad "result file missing"
    tail -30 "$TMP/run.log" >&2
else
    for key in playing stopped saw_f; do
        if grep -q "^$key=true$" "$TMP/result.txt"; then
            ok "$key"
        else
            bad "$key"
        fi
    done
    if grep -q '^output=pulse$' "$TMP/result.txt"; then
        if grep -q '^paused=true$' "$TMP/result.txt"; then
            ok "paused"
        else
            bad "paused"
        fi
    else
        echo "ok - paused (skipped: dummy output is not real-time)"
    fi
    pos=$(awk -F= '$1 == "max_pos" { print $2 }' "$TMP/result.txt")
    if awk -v p="$pos" 'BEGIN { exit !(p > 0) }'; then
        ok "position advanced"
    else
        bad "position did not advance (max_pos=$pos)"
    fi
fi

if [ "$fail" -ne 0 ]; then
    echo "test_mp3_player_wrapper: FAILED" >&2
    exit 1
fi
echo "test_mp3_player_wrapper: PASSED"
