#!/bin/bash
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
export MP3PLAYER_ROOT="$ROOT"
export MP3PLAYER_CONFIG="${1:-$DIR/config.lua}"
exec "$ROOT/run_lui.sh" "$DIR/library_ui.lua" -themeFile "$DIR/theme.hdf" -theme default
