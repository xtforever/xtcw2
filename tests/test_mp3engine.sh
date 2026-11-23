#!/bin/bash
# Standalone libmpg123 + libpulse-simple DSP proof.
# Skips when libmpg123-dev is not installed.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/mp3engine.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

if ! pkg-config --exists libmpg123; then
    echo "test_mp3engine: SKIPPED (libmpg123-dev missing)"
    exit 0
fi
if ! pkg-config --exists libpulse-simple; then
    echo "test_mp3engine: SKIPPED (libpulse-simple missing)"
    exit 0
fi
if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "test_mp3engine: SKIPPED (ffmpeg missing)"
    exit 0
fi

if ! make -C "$ROOT/apps/mp3player/c" mp3engine_test >"$TMP/build.log" 2>&1; then
    echo "FAIL: mp3engine_test build" >&2
    tail -20 "$TMP/build.log" >&2
    exit 1
fi

if ! ffmpeg -hide_banner -loglevel error -y \
    -f lavfi -i "sine=frequency=1000:sample_rate=44100:duration=1" \
    -ac 2 -q:a 9 "$TMP/tone.mp3" >"$TMP/ffmpeg.log" 2>&1; then
    echo "test_mp3engine: SKIPPED (ffmpeg mp3 encoder unavailable)"
    exit 0
fi

"$ROOT/apps/mp3player/c/mp3engine_test" --null --seconds 1 "$TMP/tone.mp3" \
    >"$TMP/out.log" 2>"$TMP/err.log"
rc=$?
if [ "$rc" -ne 0 ]; then
    echo "FAIL: mp3engine_test exit $rc" >&2
    cat "$TMP/err.log" >&2
    exit 1
fi
if ! grep -q '^spec ' "$TMP/out.log"; then
    echo "FAIL: no spectrum output" >&2
    cat "$TMP/out.log" >&2
    exit 1
fi
if ! grep -q 'format: 44100 Hz, 2 ch' "$TMP/err.log"; then
    echo "FAIL: unexpected format" >&2
    cat "$TMP/err.log" >&2
    exit 1
fi

echo "test_mp3engine: PASSED"
