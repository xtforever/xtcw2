#!/bin/bash
# Headless test for confirmation dialogs and writing the answer to script stdin.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_prompt.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

export ADMPANEL_FIXTURE="$ROOT/tests/fixtures/launcher_prompt.sh"
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

run_case() {
    local answer="$1"
    local out="$TMP/out_$answer.txt"
    local log="$TMP/run_$answer.log"
    ADMPANEL_TEST_ANSWER="$answer" ADMPANEL_TEST_OUT="$out" \
        timeout 25 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_prompt_flow.lua" \
        >"$log" 2>&1
    local rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "FAIL: case $answer exited $rc" >&2
        tail -20 "$log" >&2
        return 1
    fi
    if [ ! -s "$out" ]; then
        echo "FAIL: case $answer produced no output" >&2
        tail -20 "$log" >&2
        return 1
    fi
    grep -q '^question=Continue$' "$out" || { echo "FAIL: $answer question" >&2; cat "$out" >&2; return 1; }
    grep -q '^options=y,n$' "$out" || { echo "FAIL: $answer options" >&2; cat "$out" >&2; return 1; }
    grep -q "\\[answer $answer\\]" "$out" || { echo "FAIL: $answer answer log" >&2; cat "$out" >&2; return 1; }
    grep -q "^log[0-9]*=answer=$answer$" "$out" || { echo "FAIL: $answer script output" >&2; cat "$out" >&2; return 1; }
    grep -q '^status=Finished with exit code 0$' "$out" || { echo "FAIL: $answer status" >&2; cat "$out" >&2; return 1; }
    return 0
}

fail=0
run_case y || fail=1
run_case n || fail=1

if [ "$fail" -ne 0 ]; then
    exit 1
fi
echo "test_launcher_prompt_flow: OK"
