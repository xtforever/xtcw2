#!/bin/bash
# run_tests.sh — KaroEd keystroke-injection test suite.
#
# Builds test_karoed and replays key scenarios under Xvfb, then asserts on the
# TRACE(KARO_TESTING) lines the instrumented widget emits. No screenshots.
#
#   cd experimental/karo_widget && ./run_tests.sh
#
# Requires the toolkit libraries to be built first (from the repo root: make).
# Exit status: 0 if all checks pass, 1 otherwise.
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

RESULTS_DIR="$SCRIPT_DIR/test_results"
mkdir -p "$RESULTS_DIR"

PASS=0
FAIL=0

if [ -t 1 ]; then
    RED=$'\033[0;31m'; GRN=$'\033[0;32m'; BLU=$'\033[0;34m'; RST=$'\033[0m'
else
    RED=''; GRN=''; BLU=''; RST=''
fi

log()  { echo "${BLU}[TEST]${RST} $*"; }
pass() { echo "${GRN}[PASS]${RST} $*"; PASS=$((PASS + 1)); }
fail() { echo "${RED}[FAIL]${RST} $*"; FAIL=$((FAIL + 1)); }

# assert_seq <logfile> <desc> <pattern>...
# Each pattern must occur (as a literal substring) after the previous match.
assert_seq() {
    local file="$1" desc="$2"; shift 2
    local cursor=0 pat ln
    for pat in "$@"; do
        # Pass the pattern via the environment so awk does not interpret
        # backslash escapes (e.g. the literal "\n" in multi-line copy traces).
        ln=$(KARO_PAT="$pat" awk -v c="$cursor" 'NR>c && index($0,ENVIRON["KARO_PAT"])>0 { print NR; exit }' "$file")
        if [ -z "$ln" ]; then
            fail "$desc — missing (in order): $pat"
            return 1
        fi
        cursor="$ln"
    done
    pass "$desc"
    return 0
}

# assert_absent <logfile> <desc> <pattern>
# Fails if the literal pattern occurs anywhere in the log.
assert_absent() {
    local file="$1" desc="$2" pat="$3"
    if grep -qF -- "$pat" "$file"; then
        fail "$desc — unexpected match: $pat"
    else
        pass "$desc"
    fi
}

# run_scenario <name> <expected TRACE substrings...>
run_scenario() {
    local name="$1"; shift
    local logfile="$RESULTS_DIR/$name.log"
    log "scenario: $name"

    xvfb-run --auto-servernum ./test_karoed "$name" >"$logfile" 2>&1
    local rc=$?

    if [ "$rc" -ne 0 ]; then
        fail "$name — driver exited $rc (see $logfile)"
        return
    fi
    if grep -qiE 'Segmentation fault|core dumped|Aborted' "$logfile"; then
        fail "$name — crash detected (see $logfile)"
        return
    fi
    if ! grep -q '\[50\]' "$logfile"; then
        fail "$name — no TRACE(50) output (see $logfile)"
        return
    fi
    if grep -q 'still allocated' "$logfile"; then
        fail "$name — memory leak detected (see $logfile)"
        return
    fi
    assert_seq "$logfile" "$name" "$@"
}

# ── Build ────────────────────────────────────────────────────────────────
log "building test_karoed"
if ! make test_karoed >"$RESULTS_DIR/build.log" 2>&1; then
    fail "build test_karoed (see $RESULTS_DIR/build.log)"
    tail -20 "$RESULTS_DIR/build.log"
    echo
    echo "  Note: build the toolkit libraries first (repo root: 'make')."
    exit 1
fi
pass "build test_karoed"

# ── Scenarios ────────────────────────────────────────────────────────────
echo
log "=== KaroEd keystroke-injection tests (TRACE capture) ==="

run_scenario type \
    "KaroEd insert_char len=1 'a' cursor=(1,0)" \
    "KaroEd insert_char len=1 'b' cursor=(2,0)" \
    "KaroEd insert_char len=1 'c' cursor=(3,0)" \
    "KaroEd line0='abc'" \
    "KaroEd cursor=(3,0)"

run_scenario return \
    "KaroEd insert_char len=1 'a' cursor=(1,0)" \
    "KaroEd insert_char len=1 'b' cursor=(2,0)" \
    "KaroEd key_return cursor=(0,1)" \
    "KaroEd insert_char len=1 'c' cursor=(1,1)" \
    "KaroEd insert_char len=1 'd' cursor=(2,1)" \
    "KaroEd line0='ab'" \
    "KaroEd line1='cd'" \
    "KaroEd cursor=(2,1)"

run_scenario backspace \
    "KaroEd insert_char len=1 'c' cursor=(3,0)" \
    "KaroEd backspace type=char cursor=(2,0)" \
    "KaroEd line0='ab'" \
    "KaroEd cursor=(2,0)"

# Backspace at column 0 joins the line with the previous one
run_scenario join_backspace \
    "KaroEd backward_char cursor=(0,1)" \
    "KaroEd join_prev cursor=(2,0)" \
    "KaroEd line0='abcd'" \
    "KaroEd lines=1" \
    "KaroEd cursor=(2,0)"

# Delete at end of line joins the next line into the current one
run_scenario join_delete \
    "KaroEd prev_line cursor=(2,0)" \
    "KaroEd join_next cursor=(2,0)" \
    "KaroEd line0='abcd'" \
    "KaroEd lines=1" \
    "KaroEd cursor=(2,0)"

# select_all -> delete selection -> paste reconstructs both lines
run_scenario copy_paste \
    "KaroEd copy='xy\nzw'" \
    "KaroEd line0='xy'" \
    "KaroEd line1='zw'" \
    "KaroEd lines=2" \
    "KaroEd cursor=(2,1)"

# Ctrl+C copies the selection without modifying the buffer
run_scenario copy_action \
    "KaroEd select_range anchor=(0,0) cursor=(1,0)" \
    "KaroEd copy='a'" \
    "KaroEd line0='ab'" \
    "KaroEd lines=2"

# Ctrl+X copies then deletes the multi-line selection
run_scenario cut_action \
    "KaroEd copy='ab\ncd'" \
    "KaroEd line0=''" \
    "KaroEd lines=1" \
    "KaroEd cursor=(0,0)"

# public API: writeln appends, get_text round-trips, callback fires
run_scenario api \
    "KaroEd callback kind=6" \
    "KaroEd callback kind=6" \
    "api get_text='\nhello\nworld'" \
    "KaroEd line1='hello'" \
    "KaroEd line2='world'"

# fileForm creation path (FRM/SCR lifetime, no leaks)
run_scenario fileform \
    "KaroEd line0="

# Return auto-indents the new line to match the current line
run_scenario indent \
    "KaroEd key_return cursor=(2,1)" \
    "KaroEd line0='  ab'" \
    "KaroEd line1='  cd'" \
    "KaroEd cursor=(4,1)"

# Tab inserts spaces up to the next 8-column stop
run_scenario tab \
    "KaroEd insert_tab cursor=(8,0)" \
    "KaroEd insert_char len=1 'b' cursor=(9,0)" \
    "KaroEd line0='a"

# Inserting in the middle of a line shifts the tail (memmove path)
run_scenario insert_midline \
    "KaroEd insert_char len=1 'c' cursor=(2,0)" \
    "KaroEd backward_char cursor=(1,0)" \
    "KaroEd insert_char len=1 'b' cursor=(2,0)" \
    "KaroEd line0='abc'" \
    "KaroEd cursor=(2,0)"

run_scenario delete \
    "KaroEd insert_char len=1 'c' cursor=(3,0)" \
    "KaroEd backward_char cursor=(2,0)" \
    "KaroEd remove_char line=0 cursor=(2,0)" \
    "KaroEd line0='ab'" \
    "KaroEd cursor=(2,0)"

run_scenario arrows \
    "KaroEd key_return cursor=(0,1)" \
    "KaroEd insert_char len=1 'd' cursor=(2,1)" \
    "KaroEd prev_line cursor=(2,0)" \
    "KaroEd backward_char cursor=(1,0)" \
    "KaroEd next_line cursor=(1,1)" \
    "KaroEd forward_char cursor=(2,1)" \
    "KaroEd line0='ab'" \
    "KaroEd line1='cd'" \
    "KaroEd cursor=(2,1)"

run_scenario clamp_column \
    "KaroEd insert_char len=1 'g' cursor=(4,1)" \
    "KaroEd prev_line cursor=(1,0)" \
    "KaroEd insert_char len=1 'X' cursor=(2,0)" \
    "KaroEd line0='aX'" \
    "KaroEd cursor=(2,0)"

run_scenario home_end \
    "KaroEd caret=(0,0)" \
    "KaroEd cursor_home cursor=(0,0)" \
    "KaroEd caret=(3,0)" \
    "KaroEd cursor_end cursor=(3,0)" \
    "KaroEd line0='abc'" \
    "KaroEd cursor=(3,0)" \
    "KaroEd selection anchor=(0,0) active=0 text=''"

# locked: mutations must be rejected and no change callback fired
run_scenario locked \
    "KaroEd line0=''" \
    "KaroEd lines=1" \
    "KaroEd cursor=(0,0)"
assert_absent "$RESULTS_DIR/locked.log" "locked — no change callback" "KaroEd callback"

# select_all: whole buffer selected and copied (multi-line selection)
run_scenario select_all \
    "KaroEd copy='ab\ncd'" \
    "KaroEd select_all anchor=(0,0) cursor=(2,1)" \
    "KaroEd line1='cd'" \
    "KaroEd cursor=(2,1)" \
    "KaroEd selection anchor=(0,0) active=1 text='ab\ncd'"

# typing over a multi-line selection replaces the whole range
run_scenario type_over_selection \
    "KaroEd select_range anchor=(0,0) cursor=(2,1)" \
    "KaroEd line0='X'" \
    "KaroEd lines=1" \
    "KaroEd cursor=(1,0)"

# reversed anchor (cursor before anchor) must replace the same range
run_scenario type_over_selection_rev \
    "KaroEd select_range anchor=(2,1) cursor=(0,0)" \
    "KaroEd line0='X'" \
    "KaroEd lines=1" \
    "KaroEd cursor=(1,0)"

# ── Summary ──────────────────────────────────────────────────────────────
echo
echo "=========================================="
echo "  RESULTS: $PASS passed, $FAIL failed"
echo "=========================================="

[ "$FAIL" -gt 0 ] && exit 1
exit 0
