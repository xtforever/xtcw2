#!/bin/bash
# Headless test for script-embedded launcher descriptions.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_described.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

export ADMPANEL_FIXTURE="$ROOT/tests/fixtures/launcher_described.sh"
export ADMPANEL_TEST_OUT="$TMP/out.txt"
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

timeout 25 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_described.lua" \
    >"$TMP/run.log" 2>&1
rc=$?

fail=0
if [ "$rc" -ne 0 ]; then
    echo "FAIL: commander exited $rc" >&2
    tail -30 "$TMP/run.log" >&2
    fail=1
fi
if [ ! -s "$TMP/out.txt" ]; then
    echo "FAIL: probe output missing" >&2
    tail -30 "$TMP/run.log" >&2
    exit 1
fi
grep -q '^title=Described Demo$' "$TMP/out.txt" || { echo "FAIL: title" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q -- '--customer jens --logfile /tmp/log --verbose' "$TMP/out.txt" || { echo "FAIL: command" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q 'args:--customer jens --logfile /tmp/log --verbose' "$TMP/out.txt" || { echo "FAIL: script args" >&2; cat "$TMP/out.txt" >&2; fail=1; }
grep -q '^status=Finished with exit code 0$' "$TMP/out.txt" || { echo "FAIL: status" >&2; cat "$TMP/out.txt" >&2; fail=1; }

if [ "$fail" -ne 0 ]; then
    echo "--- run.log ---" >&2
    tail -40 "$TMP/run.log" >&2
    exit 1
fi
echo "test_launcher_described: OK"
