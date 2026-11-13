#!/bin/bash
# Run a LUI script with the XTCW commander_runner.
#
# Usage:
#   ./run_lui.sh <script.lua> [commander_runner options...]
#
# The wrapper can be started from any directory. A relative <script.lua> path
# is resolved against the caller's current directory; if it does not exist
# there, it is also tried relative to the repository root. The runner itself is
# always started from commander/ because it loads ../lui/?.lua relative to its
# working directory.
#
# Examples:
#   ./run_lui.sh demos/demo_form.lua
#   ./run_lui.sh demos/demo_form.lua -Tracelevel 2
#   /path/to/xtcw/run_lui.sh /tmp/my_ui.lua
#   cd /tmp && /path/to/xtcw/run_lui.sh /path/to/xtcw/demos/demo_listview.lua
#
# When DISPLAY is unset, xvfb-run is used if available. Set COMMANDER_RUNNER
# to use a different commander_runner binary (useful for test builds).

set -euo pipefail

SOURCE="${BASH_SOURCE[0]}"
while [ -L "$SOURCE" ]; do
    SOURCE_DIR="$(cd -P -- "$(dirname -- "$SOURCE")" && pwd)"
    SOURCE="$(readlink -- "$SOURCE")"
    case "$SOURCE" in
        /*) ;;
        *) SOURCE="$SOURCE_DIR/$SOURCE" ;;
    esac
done
ROOT="$(cd -P -- "$(dirname -- "$SOURCE")" && pwd)"
CALLER_PWD="$(pwd -P)"
RUNNER="${COMMANDER_RUNNER:-$ROOT/commander/commander_runner}"

usage() {
    cat <<EOF
Usage: $(basename "$0") <script.lua> [commander_runner options...]

Runs a LUI script with commander_runner. The script path may be absolute or
relative to the current directory. Additional options are passed to
commander_runner (for example -Tracelevel 2 or -themeFile file.hdf).

Examples:
  $(basename "$0") demos/demo_form.lua
  $(basename "$0") demos/demo_form.lua -Tracelevel 2
  $(basename "$0") /tmp/my_ui.lua
EOF
}

if [ $# -lt 1 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    usage
    [ $# -ge 1 ] && exit 0
    exit 2
fi

LUI_SCRIPT="$1"
shift

resolve_path() {
    local p="$1"
    if command -v realpath >/dev/null 2>&1; then
        realpath -m -- "$p"
        return
    fi
    case "$p" in
        /*) printf '%s\n' "$p" ;;
        *)  printf '%s/%s\n' "$CALLER_PWD" "$p" ;;
    esac
}

SCRIPT_ABS="$(resolve_path "$LUI_SCRIPT")"
case "$RUNNER" in
    /*) ;;
    *) RUNNER="$CALLER_PWD/$RUNNER" ;;
esac

if [ ! -f "$SCRIPT_ABS" ]; then
    # Convenience: allow repository-relative paths from any directory.
    ROOT_CANDIDATE="$ROOT/$LUI_SCRIPT"
    if [ -f "$ROOT_CANDIDATE" ]; then
        SCRIPT_ABS="$ROOT_CANDIDATE"
    else
        echo "error: LUI script not found: $LUI_SCRIPT" >&2
        echo "  tried: $SCRIPT_ABS" >&2
        echo "  tried: $ROOT_CANDIDATE" >&2
        exit 1
    fi
fi

if [ ! -r "$SCRIPT_ABS" ]; then
    echo "error: LUI script is not readable: $SCRIPT_ABS" >&2
    exit 1
fi

if [ ! -x "$RUNNER" ]; then
    echo "error: commander_runner not found or not executable:" >&2
    echo "  $RUNNER" >&2
    echo "Build it from the repository root with:" >&2
    echo "  make commander" >&2
    exit 1
fi

# Give a useful message when the binary was built against a newer glibc or is
# missing shared libraries instead of letting exec fail with a linker error.
if command -v ldd >/dev/null 2>&1; then
    if ldd "$RUNNER" 2>&1 | grep -q "not found"; then
        echo "error: commander_runner cannot be loaded by the dynamic linker:" >&2
        ldd "$RUNNER" 2>&1 | grep "not found" >&2
        echo "Rebuild it from the repository root with:" >&2
        echo "  make commander" >&2
        exit 1
    fi
fi

cd -- "$ROOT/commander"

if [ -n "${DISPLAY:-}" ] || ! command -v xvfb-run >/dev/null 2>&1; then
    exec "$RUNNER" -Luafile "$SCRIPT_ABS" "$@"
else
    exec xvfb-run --auto-servernum "$RUNNER" -Luafile "$SCRIPT_ABS" "$@"
fi
