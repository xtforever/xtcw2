#!/bin/bash
# Regression test for run_widget.sh / lui/widget_harness.lua.
#
# Verifies the standard 700x600 probe window, the top host, the bottom-right
# Quit button, inline/file probes, setup/after hooks and error handling.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/run_widget_test.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

PASS=0
FAIL=0
ok()  { echo "ok - $1"; PASS=$((PASS + 1)); }
bad() { echo "FAIL - $1"; FAIL=$((FAIL + 1)); }

ERR_PATTERN='Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted'

if [ ! -x "$ROOT/commander/commander_runner" ]; then
    if ( cd "$ROOT" && make commander ) >"$TMP/commander-build.log" 2>&1; then
        ok "built commander_runner"
    else
        bad "built commander_runner"
        tail -20 "$TMP/commander-build.log" 2>/dev/null | sed 's/^/    /'
        exit 1
    fi
fi

# Run one probe. The result/autoclose options are inserted before the caller's
# harness options, so they are always before the widget argument.
probe() {
    local name="$1"; shift
    local result="$1"; shift
    local log="$TMP/$name.log"
    local rc=0
    xvfb-run --auto-servernum "$ROOT/run_widget.sh" \
        --result "$result" --autoclose 800 "$@" >"$log" 2>&1 || rc=$?
    echo "$rc" > "$TMP/$name.rc"
    echo "$log" > "$TMP/$name.logpath"
    echo "$result" > "$TMP/$name.resultpath"
}

check_rc() {
    local name="$1" expected="${2:-0}"
    local rc
    rc=$(cat "$TMP/$name.rc" 2>/dev/null || echo missing)
    if [ "$rc" = "$expected" ]; then
        ok "$name: exit status $expected"
    else
        bad "$name: exit status $rc (expected $expected)"
    fi
}

check_log() {
    local name="$1"
    local log
    log=$(cat "$TMP/$name.logpath" 2>/dev/null || true)
    if [ -z "$log" ] || [ ! -f "$log" ]; then
        bad "$name: log missing"
        return
    fi
    if grep -qiE "$ERR_PATTERN" "$log"; then
        bad "$name: error pattern in log"
        grep -iE "$ERR_PATTERN" "$log" | head -3 | sed 's/^/    /'
    else
        ok "$name: no error pattern"
    fi
}

value() {
    local file="$1" key="$2"
    awk -F= -v k="$key" '$1 == k { print $2; exit }' "$file" 2>/dev/null
}

check_value() {
    local name="$1" file="$2" key="$3" expected="$4"
    local got
    got=$(value "$file" "$key")
    if [ "$got" = "$expected" ]; then
        ok "$name: $key=$expected"
    else
        bad "$name: $key='$got' (expected '$expected')"
    fi
}

check_positive() {
    local name="$1" file="$2" key="$3"
    if awk -F= -v k="$key" '$1 == k { found=1; if ($2 + 0 > 0) positive=1 } END { exit !(found && positive) }' "$file" 2>/dev/null; then
        ok "$name: $key > 0"
    else
        bad "$name: $key is not positive"
    fi
}

check_less() {
    local name="$1" file="$2" key="$3" other="$4"
    local a b
    a=$(value "$file" "$key")
    b=$(value "$file" "$other")
    if [ -n "$a" ] && [ -n "$b" ] && [ "$a" -lt "$b" ] 2>/dev/null; then
        ok "$name: $key < $other"
    else
        bad "$name: $key='$a' is not less than $other='$b'"
    fi
}

# 1. Inline fragment.
probe inline "$TMP/inline.result" '(Wlabel :label "probe" :fontSize 18)'
check_rc inline
check_log inline
if [ -s "$TMP/inline.result" ]; then
    ok "inline: result file written"
    check_value inline "$TMP/inline.result" window_width 700
    check_value inline "$TMP/inline.result" window_height 600
    check_value inline "$TMP/inline.result" grid_width 700
    check_value inline "$TMP/inline.result" grid_height 600
    check_value inline "$TMP/inline.result" host_width 700
    check_value inline "$TMP/inline.result" probe_class Wlabel
    check_positive inline "$TMP/inline.result" host_height
    check_positive inline "$TMP/inline.result" quit_x
    check_positive inline "$TMP/inline.result" quit_y
    check_positive inline "$TMP/inline.result" quit_width
    check_positive inline "$TMP/inline.result" quit_height
    check_less inline "$TMP/inline.result" quit_width grid_width
else
    bad "inline: result file missing"
fi

# 2. Probe file.
probe file "$TMP/file.result" "$ROOT/tests/widgets/wlabel.lui"
check_rc file
check_log file
check_value file "$TMP/file.result" probe_class Wlabel

# 2b. A natural-sized probe stays centered inside the host.
probe natural "$TMP/natural.result" "$ROOT/tests/widgets/wbutton.lui"
check_rc natural
check_log natural
check_value natural "$TMP/natural.result" probe_class Wbutton
check_less natural "$TMP/natural.result" probe_width grid_width
check_positive natural "$TMP/natural.result" probe_x
check_positive natural "$TMP/natural.result" probe_y

# 2c. WAppWindow probe (LUI alias + slot container).
probe appwindow "$TMP/appwindow.result" "$ROOT/tests/widgets/wappwindow.lui"
check_rc appwindow
check_log appwindow
check_value appwindow "$TMP/appwindow.result" probe_class WAppWindow
check_value appwindow "$TMP/appwindow.result" probe_id app

# 2d. WMenuBar probe (menu bar + popup menu containers).
probe menubar "$TMP/menubar.result" "$ROOT/tests/widgets/wmenubar.lui"
check_rc menubar
check_log menubar
check_value menubar "$TMP/menubar.result" probe_class WMenuBar
check_value menubar "$TMP/menubar.result" probe_id menubar

# 2e. WAction probe (checkable action item).
probe action "$TMP/action.result" "$ROOT/tests/widgets/waction.lui"
check_rc action
check_log action
check_value action "$TMP/action.result" probe_class WAction
check_value action "$TMP/action.result" probe_id act

# 2f. WToolBar probe.
probe toolbar "$TMP/toolbar.result" "$ROOT/tests/widgets/wtoolbar.lui"
check_rc toolbar
check_log toolbar
check_value toolbar "$TMP/toolbar.result" probe_class WToolBar
check_value toolbar "$TMP/toolbar.result" probe_id tb

# 2g. WStatusBar probe.
probe statusbar "$TMP/statusbar.result" "$ROOT/tests/widgets/wstatusbar.lui"
check_rc statusbar
check_log statusbar
check_value statusbar "$TMP/statusbar.result" probe_class WStatusBar
check_value statusbar "$TMP/statusbar.result" probe_id sb

# 2h. WComboBox probe.
probe combobox "$TMP/combobox.result" "$ROOT/tests/widgets/wcombobox.lui"
check_rc combobox
check_log combobox
check_value combobox "$TMP/combobox.result" probe_class WComboBox
check_value combobox "$TMP/combobox.result" probe_id combo

# 2i. WProgressBar probe.
probe progress "$TMP/progress.result" "$ROOT/tests/widgets/wprogressbar.lui"
check_rc progress
check_log progress
check_value progress "$TMP/progress.result" probe_class WProgressBar
check_value progress "$TMP/progress.result" probe_id progress

# 2j. WCheckBox probe.
probe checkbox "$TMP/checkbox.result" "$ROOT/tests/widgets/wcheckbox.lui"
check_rc checkbox
check_log checkbox
check_value checkbox "$TMP/checkbox.result" probe_class Wcheckbox

# 2k. WFormLayout probe.
probe form "$TMP/form.result" "$ROOT/tests/widgets/wformlayout.lui"
check_rc form
check_log form
check_value form "$TMP/form.result" probe_class WFormLayout
check_value form "$TMP/form.result" probe_id form

# 2l. WSpinBox probe.
probe spinbox "$TMP/spinbox.result" "$ROOT/tests/widgets/wspinbox.lui"
check_rc spinbox
check_log spinbox
check_value spinbox "$TMP/spinbox.result" probe_class WSpinBox
check_value spinbox "$TMP/spinbox.result" probe_id spin

# 2m. WRadioGroup probe.
probe radiogroup "$TMP/radiogroup.result" "$ROOT/tests/widgets/wradiogroup.lui"
check_rc radiogroup
check_log radiogroup
check_value radiogroup "$TMP/radiogroup.result" probe_class WRadioGroup
check_value radiogroup "$TMP/radiogroup.result" probe_id rg

# 2n. WLineEdit probe.
probe lineedit "$TMP/lineedit.result" "$ROOT/tests/widgets/wlineedit.lui"
check_rc lineedit
check_log lineedit
check_value lineedit "$TMP/lineedit.result" probe_class WLineEdit
check_value lineedit "$TMP/lineedit.result" probe_id le

# 2o. Table-view (WListView sort mode) probe.
probe tableview "$TMP/tableview.result" "$ROOT/tests/widgets/wtableview.lui"
check_rc tableview
check_log tableview
check_value tableview "$TMP/tableview.result" probe_class table-view
check_value tableview "$TMP/tableview.result" probe_id table

# 2p. WDialog probe.
probe dialog "$TMP/dialog.result" "$ROOT/tests/widgets/wdialog.lui"
check_rc dialog
check_log dialog
check_value dialog "$TMP/dialog.result" probe_class WDialog
check_value dialog "$TMP/dialog.result" probe_id dlg

# 2p2. WDialog input-row probe.
probe inputdialog "$TMP/inputdialog.result" "$ROOT/tests/widgets/winputdialog.lui"
check_rc inputdialog
check_log inputdialog
check_value inputdialog "$TMP/inputdialog.result" probe_class WDialog
check_value inputdialog "$TMP/inputdialog.result" probe_id idlg

# 2q. WTreeView probe.
probe treeview "$TMP/treeview.result" \
    --after "$ROOT/tests/widgets/populate_tree.lua" \
    "$ROOT/tests/widgets/wtreeview.lui"
check_rc treeview
check_log treeview
check_value treeview "$TMP/treeview.result" probe_class tree-view
check_value treeview "$TMP/treeview.result" probe_id tree
check_value treeview "$TMP/treeview.result" visible_count 40

# 2r. Table-view + WModel filter (rowCount reflects the filtered view).
probe tablefilter "$TMP/tablefilter.result" \
    --after "$ROOT/tests/widgets/populate_table_filter.lua" \
    "$ROOT/tests/widgets/wtableview.lui"
check_rc tablefilter
check_log tablefilter
check_value tablefilter "$TMP/tablefilter.result" probe_class table-view
check_value tablefilter "$TMP/tablefilter.result" row_count 3

# 2s. Tooltip resource on a button (manager registration must not break creation).
probe tooltip "$TMP/tooltip.result" "$ROOT/tests/widgets/wtooltip.lui"
check_rc tooltip
check_log tooltip
check_value tooltip "$TMP/tooltip.result" probe_class Wbutton
check_value tooltip "$TMP/tooltip.result" probe_id tipbtn

# 3. Model data through --after.
probe list "$TMP/list.result" \
    --after "$ROOT/tests/widgets/populate_list.lua" \
    "$ROOT/tests/widgets/wlistview.lui"
check_rc list
check_log list
check_value list "$TMP/list.result" probe_class WListView
check_value list "$TMP/list.result" probe_id lv

# 4. --setup and --after hooks both run.
cat > "$TMP/setup.lua" <<'LUA'
local f = io.open(os.getenv('HARNESS_HOOK_FILE'), 'a')
if f then f:write('setup\n'); f:close() end
LUA
cat > "$TMP/after.lua" <<'LUA'
local f = io.open(os.getenv('HARNESS_HOOK_FILE'), 'a')
if f then f:write('after\n'); f:close() end
LUA
export HARNESS_HOOK_FILE="$TMP/hooks.txt"
probe hooks "$TMP/hooks.result" \
    --setup "$TMP/setup.lua" --after "$TMP/after.lua" \
    '(Wlabel :label "hooks")'
unset HARNESS_HOOK_FILE
check_rc hooks
check_log hooks
if grep -q '^setup$' "$TMP/hooks.txt" 2>/dev/null; then
    ok "hooks: setup ran"
else
    bad "hooks: setup did not run"
fi
if grep -q '^after$' "$TMP/hooks.txt" 2>/dev/null; then
    ok "hooks: after ran"
else
    bad "hooks: after did not run"
fi

# 4b. --setup callback globals are visible to the probe.
cat > "$TMP/cb_setup.lua" <<'LUA'
function on_probe_click()
    local f = io.open(os.getenv('HARNESS_CLICK_FILE'), 'w')
    if f then f:write('clicked\n'); f:close() end
end
LUA
cat > "$TMP/cb_after.lua" <<'LUA'
local w = gui.get_widget('probe_widget')
if w then xtaction(w, 'notify') end
LUA
export HARNESS_CLICK_FILE="$TMP/click.txt"
probe callback "$TMP/callback.result" \
    --setup "$TMP/cb_setup.lua" --after "$TMP/cb_after.lua" \
    '(Wbutton :id "probe_widget" :label "Click" :callback "LUA(on_probe_click)")'
unset HARNESS_CLICK_FILE
check_rc callback
check_log callback
if grep -q '^clicked$' "$TMP/click.txt" 2>/dev/null; then
    ok "callback: setup-defined handler fired"
else
    bad "callback: setup-defined handler did not fire"
fi

# 5. Missing widget is a usage error.
"$ROOT/run_widget.sh" >"$TMP/noargs.log" 2>&1
rc=$?
if [ "$rc" -ne 0 ] && grep -q 'missing widget fragment' "$TMP/noargs.log"; then
    ok "no args: rejected with a clear message"
else
    bad "no args: expected a clear usage error (rc=$rc)"
fi

# 6. Multiple top-level widgets are rejected.
xvfb-run --auto-servernum "$ROOT/run_widget.sh" --autoclose 400 \
    '(Wlabel :label "a") (Wlabel :label "b")' >"$TMP/multi.log" 2>&1
rc=$?
if [ "$rc" -ne 0 ] && grep -q 'exactly one widget' "$TMP/multi.log"; then
    ok "multiple widgets: rejected"
else
    bad "multiple widgets: expected a clear error (rc=$rc)"
fi

# 7. A window/shell probe is rejected.
xvfb-run --auto-servernum "$ROOT/run_widget.sh" --autoclose 400 \
    '(window :title "not a probe")' >"$TMP/window.log" 2>&1
rc=$?
if [ "$rc" -ne 0 ] && grep -q 'not a window/shell' "$TMP/window.log"; then
    ok "window probe: rejected"
else
    bad "window probe: expected a clear error (rc=$rc)"
fi

# 8. Help text.
if "$ROOT/run_widget.sh" --help >"$TMP/help.log" 2>&1 \
   && grep -q '^Usage:' "$TMP/help.log"; then
    ok "help: usage text"
else
    bad "help: usage text missing"
fi

if [ "$FAIL" -ne 0 ]; then
    echo "test_run_widget: $FAIL check(s) FAILED"
    exit 1
fi
echo "test_run_widget: PASSED"
exit 0
