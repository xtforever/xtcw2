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

echo "9b2. test_model (WModel/ProxyModel, pure Lua)"
if command -v lua5.3 >/dev/null 2>&1; then
    XTCW_ROOT="$ROOT" lua5.3 "$ROOT/tests/test_model.lua"
elif command -v lua >/dev/null 2>&1; then
    XTCW_ROOT="$ROOT" lua "$ROOT/tests/test_model.lua"
else
    echo "test_model SKIPPED (no lua interpreter)"
fi

echo "9c. test_t1_menu (xtcw launcher menu)"
"$ROOT/tests/test_t1_menu.sh"

echo "9c2. test_run_widget (single-widget probe harness)"
"$ROOT/tests/test_run_widget.sh"

echo "9c3. test_cookbook_widgets (WAppWindow + menu integration)"
"$ROOT/tests/test_cookbook_widgets.sh"

echo "9c4. test_docs_consistency (registry vs. widget sources)"
"$ROOT/tests/test_docs_consistency.sh"

echo "9c4b. test_theme_roles (surface roles in every shipped theme)"
"$ROOT/tests/test_theme_roles.sh"

echo "9c5. test_dialogs (gui.alert/confirm/prompt helpers)"
"$ROOT/tests/test_dialogs.sh"

echo "9d. test_wretex (realize/expose regression)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wretex"

echo "9e. test_eqfader (separate scale font)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_eqfader"

echo "9f. test_wspinbox (value label)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wspinbox"

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

echo "19b. test_wappwindow (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wappwindow"

echo "19c. test_wmenubar (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wmenubar"

echo "19d. test_waction (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_waction"

echo "19e. test_wtoolbar (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wtoolbar"

echo "19f. test_wstatusbar (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wstatusbar"

echo "19g. test_wcombobox (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wcombobox"

echo "19h. test_wcomboedit (editable combo + filtered dropdown)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wcomboedit"

echo "19h. test_wprogressbar (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wprogressbar"

echo "19i. test_wcheckbox (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wcheckbox"

echo "19j. test_wformlayout (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wformlayout"

echo "19k. test_wtooltip (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wtooltip"

echo "19l. test_wdialog (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wdialog"

echo "19m. test_wtreeview (scrolling, keyboard, focus)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wtreeview"

echo "19n. test_wradiogroup (keyboard selection and focus)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wradiogroup"

echo "19o. test_iconeditor (DrawCanvas icon editor + XPM save)"
"$ROOT/tests/test_iconeditor.sh"

echo "19p. test_lifecycle (graceful shutdown + close registry)"
"$ROOT/tests/test_lifecycle.sh"

echo "19q. test_xt_errno (xtaction error codes)"
"$ROOT/tests/test_xt_errno.sh"

echo "20. test_utf8"
"$ROOT/tests/test_utf8"

echo "21. test_utf32"
"$ROOT/tests/test_utf32"

echo "22. test_ptable"
"$ROOT/tests/test_ptable"

echo "22b. test_rtable (rich atom piece table)"
"$ROOT/tests/test_rtable"

echo "22c. test_markdown (Markdown subset parser)"
"$ROOT/tests/test_markdown"

echo "22d. test_shaping (HarfBuzz/FreeType shaping)"
"$ROOT/tests/test_shaping"

echo "22e. test_hyphen (optional libhyphen integration)"
"$ROOT/tests/test_hyphen"

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

echo "25d. test_distance (pt/mm/in to pixels + theme metric units)"
"$ROOT/tests/test_distance"

echo "26. test_wbravo (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wbravo"

echo "26b. test_rich_wbravo (WYSIWYG style editing)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_rich_wbravo"

echo "26c. test_wbravo_state (full-state snapshot save/load)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wbravo_state"

echo "26d. test_wmarkdown (Markdown viewer widget)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wmarkdown"
echo "26d. test_wlistview (multi-column list view)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wlistview"
echo "26e. test_wfilechooser (file chooser over WListView)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wfilechooser"

echo "27. test_mls_trace (trace level + post-mortem source lookup)"
"$ROOT/tests/test_mls_trace"

echo "28. test_wmenubutton (headless Xvfb)"
xvfb-run --auto-servernum --server-num=99 "$ROOT/tests/test_wmenubutton"

echo "29. AdmPanel launcher pure-Lua tests"
if command -v lua5.3 >/dev/null 2>&1; then
    lua5.3 "$ROOT/tests/test_launcher_vars.lua"
    lua5.3 "$ROOT/tests/test_launcher_prompt.lua"
    lua5.3 "$ROOT/tests/test_launcher_config.lua"
    lua5.3 "$ROOT/tests/test_launcher_sql.lua"
    lua5.3 "$ROOT/tests/test_launcher_scriptmeta.lua"
else
    echo "AdmPanel pure-Lua tests SKIPPED (no lua5.3)"
fi

echo "29b. AdmPanel launcher headless tests"
"$ROOT/tests/test_launcher_ui.sh"
"$ROOT/tests/test_launcher_script.sh"
"$ROOT/tests/test_launcher_prompt_flow.sh"
"$ROOT/tests/test_launcher_dialog.sh"
"$ROOT/tests/test_launcher_sql_ui.sh"
"$ROOT/tests/test_launcher_db.sh"
"$ROOT/tests/test_launcher_sql_real.sh"
"$ROOT/tests/test_launcher_described.sh"
"$ROOT/tests/test_launcher_required.sh"
"$ROOT/tests/test_launcher_dropdown_guard.sh"

echo "All tests PASSED."
