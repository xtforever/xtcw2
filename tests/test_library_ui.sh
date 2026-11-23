#!/bin/bash
# Headless smoke test for the standalone playlist-library GUI.
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/library_ui.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

cat > "$TMP/config.lua" <<LUA
return {
  music_dir = "$HOME/Musik",
  library_file = "$TMP/library.lua",
  library_playlist_dir = "$TMP/playlists",
}
LUA

ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}" \
MP3PLAYER_ROOT="$ROOT" \
MP3PLAYER_CONFIG="$TMP/config.lua" \
MP3PLAYER_AUTOCLOSE=800 \
timeout 20 "$ROOT/apps/mp3player/run_library.sh" "$TMP/config.lua" \
    >"$TMP/run.log" 2>&1
rc=$?
if [ "$rc" -ne 0 ]; then
    echo "FAIL: library UI exit $rc" >&2
    tail -30 "$TMP/run.log" >&2
    exit 1
fi
if grep -qiE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log"; then
    echo "FAIL: fatal pattern in library UI log" >&2
    grep -iE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log" | head -5 >&2
    exit 1
fi
if ! grep -q 'MUSIC LIBRARY' "$TMP/run.log"; then
    echo "FAIL: library UI did not render" >&2
    tail -20 "$TMP/run.log" >&2
    exit 1
fi
echo "test_library_ui: PASSED"
