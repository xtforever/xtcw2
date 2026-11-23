#!/bin/bash
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if ! make -C "$ROOT/apps/mp3player/c" mp3engine.so >/tmp/mp3engine_build.log 2>&1; then
    echo "test_mp3engine_module: SKIPPED (mp3engine.so build failed)"
    exit 0
fi
FILE="${MP3ENGINE_TEST_FILE:-$HOME/Musik/verclub_music-summer-future-bass-577327.mp3}"
if [ ! -f "$FILE" ]; then
    echo "test_mp3engine_module: SKIPPED (test mp3 missing)"
    exit 0
fi
XTCW_ROOT="$ROOT" MP3ENGINE_TEST_FILE="$FILE" lua5.3 "$ROOT/tests/test_mp3engine_module.lua"
