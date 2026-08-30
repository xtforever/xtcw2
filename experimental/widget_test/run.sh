#!/bin/bash
set -e
cd "$(dirname "$0")"

echo "=== Building wlabel_demo ==="
make clean 2>/dev/null || true
make

echo ""
echo "=== Running wlabel_demo ==="
echo "Resize the window to verify text reflow and auto-height behavior."
echo "Close the window or click Quit to exit."
echo ""

if [ -z "$DISPLAY" ]; then
    echo "No DISPLAY set, using xvfb-run with 8s timeout"
    timeout 8s xvfb-run -a ./wlabel_demo -geometry 750x700 || true
else
    ./wlabel_demo -geometry 750x700
fi
