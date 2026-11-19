#!/bin/bash
# Headless test for SQL combo population (query binding stubbed in Lua).
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_sqlui.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

run_case() {
    local fail_mode="$1"
    local out="$TMP/out_${fail_mode}.txt"
    local log="$TMP/run_${fail_mode}.log"
    ADMPANEL_SQL_FAIL="$fail_mode" ADMPANEL_TEST_OUT="$out" \
        timeout 20 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_sql_ui.lua" \
        >"$log" 2>&1
    local rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "FAIL: sql case $fail_mode exited $rc" >&2
        tail -20 "$log" >&2
        return 1
    fi
    if [ ! -s "$out" ]; then
        echo "FAIL: sql case $fail_mode produced no output" >&2
        tail -20 "$log" >&2
        return 1
    fi
    return 0
}

fail=0
run_case "" || fail=1
if [ "$fail" -eq 0 ]; then
    grep -q '^selected=0$' "$TMP/out_.txt" || { echo "FAIL: sql selected" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^label=jens (db1)$' "$TMP/out_.txt" || { echo "FAIL: sql label" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^value=jens$' "$TMP/out_.txt" || { echo "FAIL: sql value" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^selected_key=jens$' "$TMP/out_.txt" || { echo "FAIL: selectedKey key" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^preview=echo jens$' "$TMP/out_.txt" || { echo "FAIL: sql preview" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^typed_key=anna$' "$TMP/out_.txt" || { echo "FAIL: typed key lookup" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^typed_selected_key=anna$' "$TMP/out_.txt" || { echo "FAIL: typed selectedKey" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^typed_display=anna$' "$TMP/out_.txt" || { echo "FAIL: typed display lookup" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^typed_display_selected_key=anna (db2)$' "$TMP/out_.txt" || { echo "FAIL: typed display selectedKey" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^typed_free=custom$' "$TMP/out_.txt" || { echo "FAIL: typed free text" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
    grep -q '^typed_preview=echo custom$' "$TMP/out_.txt" || { echo "FAIL: typed preview" >&2; cat "$TMP/out_.txt" >&2; fail=1; }
fi

run_case 1 || fail=1
if [ -f "$TMP/out_1.txt" ]; then
    grep -q '^status=SQL error: access denied$' "$TMP/out_1.txt" || { echo "FAIL: sql error status" >&2; cat "$TMP/out_1.txt" >&2; fail=1; }
    grep -q 'access denied' "$TMP/out_1.txt" || { echo "FAIL: sql error log" >&2; cat "$TMP/out_1.txt" >&2; fail=1; }
fi

if [ "$fail" -ne 0 ]; then
    exit 1
fi
echo "test_launcher_sql_ui: OK"
