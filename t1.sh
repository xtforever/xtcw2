#!/bin/bash
# LUI Gridbox Demos launcher
# Uses xvfb-run only if no X display is available.
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
CMD="$DIR/commander/commander_runner"

if [ ! -x "$CMD" ]; then
    echo "Error: commander_runner not found at $CMD"
    echo "Build it first: cd commander && make commander_runner"
    exit 1
fi

# Use xvfb-run only if no DISPLAY is set
if [ -n "$DISPLAY" ]; then
    RUN=""
else
    echo "No DISPLAY detected, using xvfb-run"
    RUN="xvfb-run --auto-servernum"
fi

DEMO="${1:-form}"
shift 2>/dev/null || true

case "$DEMO" in
    form|1)
        echo "=== Form Demo: labels, edits, toggle, spinbox in a grid ==="
        cd "$DIR/commander"
        exec $RUN ./commander_runner -Luafile ../demos/demo_form.lua "$@"
        ;;
    calc|2)
        echo "=== Calculator Demo: 4-column grid with spanning ==="
        cd "$DIR/commander"
        exec $RUN ./commander_runner -Luafile ../demos/demo_calc.lua "$@"
        ;;
    contacts|3)
        echo "=== Contacts Demo: multi-row form with property round-trips ==="
        cd "$DIR/commander"
        exec $RUN ./commander_runner -Luafile ../demos/demo_contacts.lua "$@"
        ;;
    login|4)
        echo "=== Login Demo: password fields, toggles, feedback ==="
        cd "$DIR/commander"
        exec $RUN ./commander_runner -Luafile ../demos/demo_login.lua "$@"
        ;;
    native|5)
        echo "=== Native C Test: Gridbox constraint resources ==="
        cd "$DIR/experimental/test_gridbox"
        if [ ! -x test_gridbox ]; then
            echo "Building test_gridbox..."
            make
        fi
        exec $RUN ./test_gridbox "$@"
        ;;
    *)
        echo "Usage: $0 <demo>"
        echo ""
        echo "Available demos:"
        echo "  form     — Form with labels, edits, spinbox, toggle (default)"
        echo "  calc     — Calculator with 4x6 grid and gridWidth spanning"
        echo "  contacts — Contact manager with multi-row form"
        echo "  login    — Login dialog with password fields"
        echo "  native   — Native C test of Gridbox constraint resources"
        echo ""
        echo "Add -keep to native demo to keep window open."
        echo "Set DISPLAY to run with a real X server; otherwise xvfb-run is used."
        exit 0
        ;;
esac