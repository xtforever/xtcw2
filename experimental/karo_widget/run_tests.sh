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
        ln=$(awk -v c="$cursor" -v p="$pat" 'NR>c && index($0,p)>0 { print NR; exit }' "$file")
        if [ -z "$ln" ]; then
            fail "$desc — missing (in order): $pat"
            return 1
        fi
        cursor="$ln"
    done
    pass "$desc"
    return 0
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
    "KaroEd backward_char cursor=(2,0)" \
    "KaroEd remove_char line=0 cursor=(2,0)" \
    "KaroEd line0='ab'" \
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
    "KaroEd cursor_home cursor=(0,0)" \
    "KaroEd cursor_end cursor=(3,0)" \
    "KaroEd line0='abc'" \
    "KaroEd cursor=(3,0)"

# ── Summary ──────────────────────────────────────────────────────────────
echo
echo "=========================================="
echo "  RESULTS: $PASS passed, $FAIL failed"
echo "=========================================="

[ "$FAIL" -gt 0 ] && exit 1
exit 0
