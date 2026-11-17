#!/bin/bash
# Integration test for the themed dialog helpers (gui.alert/confirm/prompt).
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/dialogs.XXXXXX")"
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
export DIALOG_OUT="$TMP/out.txt"

cd "$ROOT/commander" || exit 1
timeout 30 xvfb-run --auto-servernum ./commander_runner \
    -Luafile "$ROOT/tests/test_dialogs.lua" >"$TMP/log" 2>&1
rc=$?

fail=0
check() {
    local key="$1" want="$2"
    local got
    got=$(grep -m1 "^$key=" "$TMP/out.txt" 2>/dev/null | cut -d= -f2-)
    if [ "$got" = "$want" ]; then
        echo "ok - $key=$want"
    else
        echo "FAIL - $key='$got' (expected '$want')"
        fail=1
    fi
}

if [ "$rc" -ne 0 ]; then
    echo "FAIL - commander exited $rc"
    tail -20 "$TMP/log" | sed 's/^/    /'
    fail=1
fi

if [ ! -s "$TMP/out.txt" ]; then
    echo "FAIL - probe output missing"
    tail -20 "$TMP/log" | sed 's/^/    /'
    exit 1
fi

check alert ok
check confirm_cancel false
check prompt Bob
check confirm_ok true

if [ "$fail" -eq 0 ]; then
    echo "test_dialogs: PASSED"
    exit 0
fi
echo "test_dialogs: FAILED"
exit 1
