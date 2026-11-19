#!/bin/bash
# Real MySQL integration test.  Uses credentials parsed from ~/.my.cnf.
# Skips cleanly when no config/group is available.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_sqlreal.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

CNF="${ADMPANEL_MYCNF:-$HOME/.my.cnf}"
if [ ! -f "$CNF" ]; then
    echo "test_launcher_sql_real: SKIPPED (no $CNF)"
    exit 0
fi

export ADMPANEL_TEST_OUT="$TMP/out.txt"
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

timeout 30 "$ROOT/run_lui.sh" "$ROOT/tests/test_launcher_sql_real.lua" \
    >"$TMP/run.log" 2>&1
rc=$?

if [ "$rc" -ne 0 ]; then
    echo "FAIL: commander exited $rc" >&2
    tail -30 "$TMP/run.log" >&2
    exit 1
fi
if [ ! -s "$TMP/out.txt" ]; then
    echo "FAIL: probe output missing" >&2
    tail -30 "$TMP/run.log" >&2
    exit 1
fi
if grep -q '^skip=1$' "$TMP/out.txt"; then
    echo "test_launcher_sql_real: SKIPPED ($(grep '^reason=' "$TMP/out.txt" | cut -d= -f2-))"
    exit 0
fi
if ! grep -q '^ok=true$' "$TMP/out.txt"; then
    echo "FAIL: real query failed" >&2
    cat "$TMP/out.txt" >&2
    exit 1
fi
EXPECT_ROWS="${ADMPANEL_TEST_EXPECT_ROWS:-3}"
EXPECT_COLUMNS="${ADMPANEL_TEST_EXPECT_COLUMNS:-custid,dbservid}"
EXPECT_FIRST="${ADMPANEL_TEST_EXPECT_FIRST:-aamit}"
grep -q "^rows=$EXPECT_ROWS$" "$TMP/out.txt" || { echo "FAIL: row count" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q "^columns=$EXPECT_COLUMNS$" "$TMP/out.txt" || { echo "FAIL: columns" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q "^first_custid=$EXPECT_FIRST$" "$TMP/out.txt" || { echo "FAIL: first custid" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q "^items=$EXPECT_ROWS$" "$TMP/out.txt" || { echo "FAIL: sql.load_field items" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q "^first_key=$EXPECT_FIRST$" "$TMP/out.txt" || { echo "FAIL: sql.load_field key" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q '^group_ok=true$' "$TMP/out.txt" || { echo "FAIL: group-based db_query" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q "^group_items=$EXPECT_ROWS$" "$TMP/out.txt" || { echo "FAIL: group-based sql.load_field items" >&2; cat "$TMP/out.txt" >&2; exit 1; }
grep -q "^group_first_key=$EXPECT_FIRST$" "$TMP/out.txt" || { echo "FAIL: group-based sql.load_field key" >&2; cat "$TMP/out.txt" >&2; exit 1; }

echo "test_launcher_sql_real: OK"
