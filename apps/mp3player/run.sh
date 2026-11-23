#!/bin/bash
# Start the XTCW mp3player demo.
#
#   ./apps/mp3player/run.sh [config.lua]

set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"

export MP3PLAYER_ROOT="$ROOT"
export MP3PLAYER_CONFIG="${1:-$DIR/config.lua}"

# Build the C FFT module on demand.  It is optional for the UI: if this fails,
# the app still starts and disables the spectrum later.
if ! make -C "$DIR/c" >/dev/null 2>&1; then
    echo "mp3player: warning: could not build mp3fft.so (spectrum will be off)" >&2
fi
if ! make -C "$DIR/c" mp3engine.so >/dev/null 2>&1; then
    echo "mp3player: warning: could not build mp3engine.so (using mpg123 fallback)" >&2
fi

exec "$ROOT/run_lui.sh" "$DIR/main.lua" -themeFile "$DIR/theme.hdf" -theme default
