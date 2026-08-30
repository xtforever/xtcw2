#!/bin/bash
# Extracts LAYOUT trace from a command and compares it with a baseline file
# Usage: ./verify_layout.sh <test_command> <baseline_file>

CMD=$1
BASELINE=$2

if [ -z "$CMD" ] || [ -z "$BASELINE" ]; then
    echo "Usage: $0 <test_command> <baseline_file>"
    exit 1
fi

TMP_OUT=$(mktemp)

# Run the command and extract LAYOUT traces, mask hexadecimal handles (addresses)
# Matches 0x followed by hex digits or just plain large integers if they look like handles
$CMD 2>&1 | grep "LAYOUT" | sed -E 's/handle (0x[0-9a-f]+|[0-9]{2,})/handle MASKED/g' > "$TMP_OUT"

if [ ! -f "$BASELINE" ]; then
    echo "Baseline file $BASELINE not found. Creating it from current run."
    cp "$TMP_OUT" "$BASELINE"
    rm "$TMP_OUT"
    exit 0
fi

# Compare
if diff -u "$BASELINE" "$TMP_OUT"; then
    echo "LAYOUT verification PASSED for $CMD"
    rm "$TMP_OUT"
    exit 0
else
    echo "LAYOUT verification FAILED for $CMD"
    echo "Check diff above."
    rm "$TMP_OUT"
    exit 1
fi
