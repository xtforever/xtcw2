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

echo "9b. test_retex_file (Lua .retex loader/writer)"
if command -v lua5.3 >/dev/null 2>&1; then
    lua5.3 "$ROOT/tests/test_retex_file.lua"
elif command -v lua >/dev/null 2>&1; then
    lua "$ROOT/tests/test_retex_file.lua"
else
    echo "test_retex_file SKIPPED (no lua interpreter)"
fi

echo "9c. test_t1_menu (xtcw launcher menu)"
"$ROOT/tests/test_t1_menu.sh"

echo "9d. test_wretex (realize/expose regression)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wretex"

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

echo "16. test_theme_parse"
"$ROOT/tests/test_theme_parse"

echo "17. test_wtoggleswitch (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wtoggleswitch"

echo "18. test_wviewport (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wviewport"

echo "19. test_wtabwidget (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wtabwidget"

echo "20. test_utf8"
"$ROOT/tests/test_utf8"

echo "21. test_utf32"
"$ROOT/tests/test_utf32"

echo "22. test_ptable"
"$ROOT/tests/test_ptable"

echo "22b. test_rtable (rich atom piece table)"
"$ROOT/tests/test_rtable"

echo "23. test_tokenize_plain"
"$ROOT/tests/test_tokenize_plain"

echo "24. test_retex_unicode"
"$ROOT/tests/test_retex_unicode"

echo "25. test_retex_plain"
"$ROOT/tests/test_retex_plain"

echo "25b. test_retex_rich_lines (rich TeX layout line metadata + caret)"
"$ROOT/tests/test_retex_rich_lines"

echo "25c. test_retex_styled (styled atom layout: fonts/styles/sizes)"
"$ROOT/tests/test_retex_styled"

echo "26. test_wbravo (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wbravo"

echo "26b. test_rich_wbravo (WYSIWYG style editing)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_rich_wbravo"

echo "26c. test_wbravo_state (full-state snapshot save/load)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wbravo_state"
echo "26d. test_wlistview (multi-column list view)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wlistview"
echo "26e. test_wfilechooser (file chooser over WListView)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wfilechooser"

echo "27. test_mls_trace (trace level + post-mortem source lookup)"
"$ROOT/tests/test_mls_trace"

echo "28. test_wmenubutton (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wmenubutton"

echo "All tests PASSED."
