#!/bin/bash
# Headless smoke/functional test for the AdmPanel LUI form.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_ui.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

export ADMPANEL_CONFIG="$ROOT/apps/admpanel/examples/freigabe.lua"
export ADMPANEL_TEST_OUT="$TMP/out.txt"
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

timeout 25 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_ui.lua" \
    >"$TMP/run.log" 2>&1
rc=$?

fail=0
if [ "$rc" -ne 0 ]; then
    echo "FAIL: commander exited $rc" >&2
    tail -20 "$TMP/run.log" >&2
    fail=1
fi
if grep -qiE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log"; then
    echo "FAIL: fatal pattern in log" >&2
    grep -iE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log" | head -5 >&2
    fail=1
fi
if [ ! -s "$TMP/out.txt" ]; then
    echo "FAIL: probe output missing" >&2
    tail -30 "$TMP/run.log" >&2
    exit 1
fi

grep -q '^title=freigabe erstellen$' "$TMP/out.txt" || { echo "FAIL: title" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q '^field_count=4$' "$TMP/out.txt" || { echo "FAIL: field count" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q '^fld1_exists=true$' "$TMP/out.txt" || { echo "FAIL: fld_1 missing" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q '^fld4_exists=true$' "$TMP/out.txt" || { echo "FAIL: fld_4 missing" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q 'adanetcon' "$TMP/out.txt" || { echo "FAIL: preview did not update" >&2; cat "$TMP/out.txt" >&2; fail=1; }

if [ "$fail" -ne 0 ]; then
    echo "--- run.log ---" >&2
    tail -40 "$TMP/run.log" >&2
    exit 1
fi
echo "test_launcher_ui: OK"
