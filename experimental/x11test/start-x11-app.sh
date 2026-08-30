#!/bin/bash

# 1. Setup the Cleanup Trap
# This function runs no matter how the script ends (success, error, or Ctrl+C)
cleanup() {
    echo "Cleaning up all processes..."
    # Kill the entire process group (the minus sign before $BASHPID is key)
    trap - SIGTERM # prevent infinite loop
    killall -TERM x11vnc
    killall -TERM matchbox-window-manager
    killall -TERM Xvfb
}

# Register the trap for EXIT and Interrupt signals
trap cleanup EXIT INT TERM

# 2. Environment Setup
DISPLAY_NUM=":99"
FILE_NAME="/tmp/screenshot.png"
rm "$FILE_NAME" 2>&1 >/dev/null
export DISPLAY=$DISPLAY_NUM

# Remove stale locks
rm -f /tmp/.X${DISPLAY_NUM:1}-lock

# 3. Start Xvfb
Xvfb $DISPLAY_NUM -screen 0 800x600x24 & 

# Wait for X server to be ready
timeout 5 bash -c "until xset q &>/dev/null; do sleep 0.1; done"

# 4. Launch the UI Stack
# We don't need to save PIDs anymore because 'kill -$$' handles the group
matchbox-window-manager &
x11vnc -display $DISPLAY_NUM -forever -nopw -bg -quiet &

# 5. Launch the App
$1 &

# Give it time to render
sleep 2

# 6. Capture (Force overwrite)
scrot -f "$FILE_NAME"

echo "Capture complete. Exiting script..."
# The 'cleanup' function triggered by 'trap' handles the killing automatically here
