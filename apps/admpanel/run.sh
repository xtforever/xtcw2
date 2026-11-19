#!/bin/bash
# Run the AdmPanel LUI launcher.
#
#   ./apps/admpanel/run.sh apps/admpanel/examples/freigabe.lua
#   ./apps/admpanel/run.sh tests/fixtures/launcher_described.sh
#
# A .sh target is treated as a script with a <launcher-begin> block.
# Any other target is treated as a Lua config file.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

if [ $# -lt 1 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "usage: $(basename "$0") <config.lua|script.sh>" >&2
    exit 2
fi

target="$(realpath -m -- "$1")"
case "$target" in
    *.sh)
        export ADMPANEL_SCRIPT="$target"
        unset ADMPANEL_CONFIG
        ;;
    *)
        export ADMPANEL_CONFIG="$target"
        unset ADMPANEL_SCRIPT
        ;;
esac

exec "$ROOT/run_lui.sh" "$ROOT/apps/admpanel/main.lua"
