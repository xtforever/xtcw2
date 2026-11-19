#!/bin/bash
# Headless test for the mandatory db_query binding (error path only).
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_db.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

export ADMPANEL_TEST_OUT="$TMP/out.txt"
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

timeout 20 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_db.lua" \
    >"$TMP/run.log" 2>&1
rc=$?

fail=0
if [ "$rc" -ne 0 ]; then
    echo "FAIL: commander exited $rc" >&2
    tail -20 "$TMP/run.log" >&2
    fail=1
fi
if [ ! -s "$TMP/out.txt" ]; then
    echo "FAIL: probe output missing" >&2
    tail -20 "$TMP/run.log" >&2
    exit 1
fi
grep -q '^result=nil$' "$TMP/out.txt" || { echo "FAIL: expected nil result" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q '^has_error=true$' "$TMP/out.txt" || { echo "FAIL: expected error text" >&2; cat "$TMP/out.txt" >&2; fail=1; }

if [ "$fail" -ne 0 ]; then
    exit 1
fi
echo "test_launcher_db: OK"
