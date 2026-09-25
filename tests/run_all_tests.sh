#!/bin/bash
set -e
# Get script directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT="$DIR/.."

echo "Running all verified tests..."
# Build from root to ensure all exports are available
(cd "$ROOT" && make tests)

echo "1. test_mls_foundation"
"$ROOT/tests/test_mls_foundation"

echo "2. test_s_cstr"
"$ROOT/tests/test_s_cstr"

echo "3. test_mls_recursive"
"$ROOT/tests/test_mls_recursive"

echo "4. test_m_table"
"$ROOT/tests/test_m_table"

echo "5. test_mls_ownership"
"$ROOT/tests/test_mls_ownership"

echo "6. test_mls_stress"
"$ROOT/tests/test_mls_stress"

echo "7. test_retex_layout"
"$ROOT/tests/test_retex_layout"

echo "8. test_retex_linebreak (LAYOUT verification)"
"$DIR/verify_layout.sh" "$ROOT/tests/test_retex_linebreak" "$DIR/baselines/test_retex_linebreak.layout"

echo "9. test_retex_math (LAYOUT verification)"
"$DIR/verify_layout.sh" "$ROOT/tests/test_retex_math" "$DIR/baselines/test_retex_math.layout"

echo "10. test_wlabel (LAYOUT verification)"
xvfb-run --auto-servernum --server-num=99 "$DIR/verify_layout.sh" "$ROOT/tests/test_wlabel" "$DIR/baselines/test_wlabel.layout"

echo "11. test_retex_hit"
"$ROOT/tests/test_retex_hit"

echo "12. test_wlabel_selection (LAYOUT verification)"
xvfb-run --auto-servernum --server-num=99 "$DIR/verify_layout.sh" "$ROOT/tests/test_wlabel_selection" "$DIR/baselines/test_wlabel_selection.layout"

echo "13. test_wlistmulti (LAYOUT verification)"
xvfb-run --auto-servernum --server-num=99 "$DIR/verify_layout.sh" "$ROOT/tests/test_wlistmulti" "$DIR/baselines/test_wlistmulti.layout"

echo "14. test_trace"
if "$ROOT/tests/test_trace" 2>&1 | grep -q "LAYOUT"; then
    echo "test_trace passed (found LAYOUT trace)"
else
    echo "test_trace FAILED (LAYOUT trace missing)"
    exit 1
fi

echo "15. KaroEd multiline editor (keystroke-injection)"
make -C "$ROOT/experimental/karo_widget" test

echo "All tests PASSED."
