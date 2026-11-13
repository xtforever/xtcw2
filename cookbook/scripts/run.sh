#!/usr/bin/env bash
# cookbook/scripts/run.sh — run the cookbook app against an existing build.
#
#   ./run.sh
#   xvfb-run -a ./run.sh          # when no DISPLAY is available
#
# commander_runner must run from commander/ (it resolves ../lui/?.lua and the
# cookbook module/asset paths relative to the working directory).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

cd "$ROOT/commander"
ARGS=(./commander_runner
      -Luafile ../cookbook/app/main.lua
      -themeFile ../cookbook/themes/cookbook.hdf
      -theme default
      "$@")

# Optional hard stop for automated starts; interactive use leaves it unset.
if [ -n "${COOKBOOK_TIMEOUT:-}" ]; then
    exec timeout "$COOKBOOK_TIMEOUT" "${ARGS[@]}"
fi
exec "${ARGS[@]}"
