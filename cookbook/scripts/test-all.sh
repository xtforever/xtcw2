#!/usr/bin/env bash
# cookbook/scripts/test-all.sh — build + smoke-test BOTH modes.
#
# For each mode: clean-build xtcw2 + the app, run the start smoke test, then
# archive the freshly built binary to cookbook/dist/<mode>/commander_runner
# (outside build/, so the next `make distclean` cannot wipe it).
#
# Aborts on the first failure (CI-friendly).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DIST="$ROOT/cookbook/dist"

overall=0
for mode in debug production; do
    echo
    echo "==================== $mode ===================="
    "$SCRIPT_DIR/build.sh" "$mode"

    if "$SCRIPT_DIR/smoke-test.sh" "$mode"; then
        mkdir -p "$DIST/$mode"
        cp "$ROOT/commander/commander_runner" "$DIST/$mode/commander_runner"
        echo "[test-all] PASS $mode (archived: $DIST/$mode/commander_runner)"
    else
        echo "[test-all] FAIL $mode"
        overall=1
        break
    fi
done

echo
if [ "$overall" -eq 0 ]; then
    echo "[test-all] all modes passed"
else
    echo "[test-all] FAILED"
fi
exit "$overall"
