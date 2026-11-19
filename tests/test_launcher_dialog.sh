#!/bin/bash
# Headless test for the real WDialog confirmation path.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_dialog.XXXXXX")"
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
    local button="$1" prompt="$2" expected="$3"
    local out="$TMP/out_${button}.txt"
    local log="$TMP/run_${button}.log"
    ADMPANEL_TEST_BUTTON="$button" ADMPANEL_TEST_PROMPT="$prompt" \
        ADMPANEL_TEST_OUT="$out" \
        timeout 20 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_dialog.lua" \
        >"$log" 2>&1
    local rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "FAIL: $button exited $rc" >&2
        tail -20 "$log" >&2
        return 1
    fi
    if ! grep -q "^letter=$expected$" "$out"; then
        echo "FAIL: $button expected $expected" >&2
        cat "$out" >&2
        return 1
    fi
    return 0
}

fail=0
run_case ok 'Continue (Y/n)?' y || fail=1
run_case cancel 'Continue (Y/n)?' n || fail=1
run_case aux 'Continue (Y/n/a)?' a || fail=1

if [ "$fail" -ne 0 ]; then
    exit 1
fi
echo "test_launcher_dialog: OK"
