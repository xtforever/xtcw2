#!/bin/bash
# Wretex Widget Demo Launcher
# Run from project root: ./run_wretex_demo.sh
set -e

DIR="$(cd "$(dirname "$0")" && pwd)"
CMD="$DIR/experimental/commander_runner"
DEMO="$DIR/demos/demo_wretex.lua"

# Build commander_runner if missing
if [ ! -x "$CMD" ]; then
    echo "commander_runner not found. Building..."
    cd "$DIR/experimental"
    make commander_runner
    cd "$DIR"
fi

# Use xvfb-run only if no DISPLAY is set
if [ -n "$DISPLAY" ]; then
    RUN=""
else
    echo "No DISPLAY detected, using xvfb-run"
    RUN="xvfb-run --auto-servernum"
fi

echo "Starting Wretex demo..."
cd "$DIR/experimental"
exec $RUN ./commander_runner -Tracelevel 2 -Luafile ../demos/demo_wretex.lua "$@"
