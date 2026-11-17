#!/bin/bash
# Integration check for the cookbook's improved-widget shell:
# WAppWindow slots, WMenuBar/WPopupMenu, menu callback -> tab switch.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/cookbook_widgets.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

cat > "$TMP/probe.lua" <<'LUA'
package.path = '../cookbook/app/?.lua;' .. package.path
dofile('../cookbook/app/main.lua')

xtapptimeout(400, function()
    gui.set('menu_file', 'open', true)
end)

xtapptimeout(2000, function()
    local f = io.open(os.getenv('COOKBOOK_PROBE_OUT'), 'w')
    for _, id in ipairs({'main','menubar','toolbar','toolbar_actions','tabs','status','ed_cat',
                         'gal_check','gal_progress','gal_combo','gal_radio','gal_name','tree_view',
                         'menu_file','menu_view','menu_help'}) do
        local g = gui.geometry(id) or {}
        f:write(string.format('%s.x=%d\n%s.y=%d\n%s.width=%d\n%s.height=%d\n',
                id, g.x or 0, id, g.y or 0, id, g.width or 0, id, g.height or 0))
    end
    local open = gui.get('menu_file', 'open')
    local confirm = gui.get_widget('mi_confirm')
    if confirm then xtaction(confirm, 'notify') end
    local confirm_checked = gui.get('mi_confirm', 'checked')
    local confirm_label = gui.get('mi_confirm', 'label') or ''
    local cat = gui.get_widget('ed_cat')
    if cat then xtaction(cat, 'select_index', 2) end
    local combo_label = gui.get('ed_cat', 'label')
    local combo_selected = gui.get('ed_cat', 'selected')
    local sortmi = gui.get_widget('mi_sort')
    if sortmi then xtaction(sortmi, 'notify') end
    local sort_status = gui.get('status', 'message')

    -- Row-id mapping: after sorting, row 0 must still resolve to Apple Crumble.
    gui.set('recipe_list', 'selected', 0)
    local sorted_id = gui.get('recipe_list', 'selectedId')
    cb_open_recipe()
    local sorted_title = gui.get('det_title', 'label')

    -- Editor CRUD + persistence + confirm delete.
    cb_new_recipe()
    gui.set('ed_title', 'label', 'Test Dish')
    gui.set('ed_author', 'label', 'Tester')
    gui.set('ed_serv', 'value', 2)
    gui.set('ed_time', 'value', 7)
    local edcat = gui.get_widget('ed_cat')
    if edcat then xtaction(edcat, 'select_index', 3) end
    gui.set('ed_veg', 'checked', true)
    local eding = gui.get_widget('ed_ingredients')
    if eding then xtaction(eding, 'set_text', '1\tcup\tFlour\n2\t\tEggs') end
    local edsteps = gui.get_widget('ed_steps')
    if edsteps then xtaction(edsteps, 'set_text', 'Mix.\nBake.') end
    cb_save()
    local save_status = gui.get('status', 'message')
    local saved_title = gui.get('det_title', 'label')
    local count_after_save = mls_len(gui.get('recipe_list', 'rows'))
    local pf = io.open(os.getenv('COOKBOOK_DATA'), 'r')
    local persist_file = pf ~= nil
    if pf then pf:close() end

    cb_delete_recipe()
    local confirm_open = gui.get('confirm_delete', 'open')
    local confirm_dlg = gui.get_widget('confirm_delete')
    if confirm_dlg then xtaction(confirm_dlg, 'ok') end
    local delete_status = gui.get('status', 'message')
    local count_after_delete = mls_len(gui.get('recipe_list', 'rows'))

    local mi = gui.get_widget('mi_detail')
    if mi then xtaction(mi, 'notify') end
    local tab = gui.get('tabs', 'selected')
    f:write('menu_open=' .. tostring(open) .. '\n')
    f:write('status_message=' .. tostring(gui.get('status', 'message')) .. '\n')
    f:write('confirm_checked=' .. tostring(confirm_checked) .. '\n')
    f:write('confirm_label_has_x=' .. tostring(confirm_label:find('%[x%]') ~= nil) .. '\n')
    f:write('combo_label=' .. tostring(combo_label) .. '\n')
    f:write('combo_selected=' .. tostring(combo_selected) .. '\n')
    f:write('sort_status=' .. tostring(sort_status) .. '\n')
    f:write('sorted_id=' .. tostring(sorted_id) .. '\n')
    f:write('sorted_title=' .. tostring(sorted_title) .. '\n')
    f:write('save_status=' .. tostring(save_status) .. '\n')
    f:write('saved_title=' .. tostring(saved_title) .. '\n')
    f:write('count_after_save=' .. tostring(count_after_save) .. '\n')
    f:write('persist_file=' .. tostring(persist_file) .. '\n')
    f:write('confirm_open=' .. tostring(confirm_open) .. '\n')
    f:write('delete_status=' .. tostring(delete_status) .. '\n')
    f:write('count_after_delete=' .. tostring(count_after_delete) .. '\n')
    f:write('gallery_check=' .. tostring(gui.get('gal_check', 'checked')) .. '\n')
    f:write('gallery_progress=' .. tostring(gui.get('gal_progress', 'value')) .. '\n')
    f:write('gallery_combo=' .. tostring(gui.get('gal_combo', 'label')) .. '\n')
    local radio = gui.get_widget('gal_radio')
    if radio then xtaction(radio, 'select_index', 2) end
    f:write('gallery_radio=' .. tostring(gui.get('gal_radio', 'selected')) .. '\n')
    f:write('gallery_name_label=' .. tostring(gui.get('gal_name', 'label')) .. '\n')
    f:write('gallery_name_placeholder=' .. tostring(gui.get('gal_name', 'placeholder')) .. '\n')
    local tree_before = gui.get('tree_view', 'visibleCount')
    local tree = gui.get_widget('tree_view')
    if tree then xtaction(tree, 'select_node', 0); xtaction(tree, 'expand') end
    local tree_after = gui.get('tree_view', 'visibleCount')
    f:write('tree_before=' .. tostring(tree_before) .. '\n')
    f:write('tree_after=' .. tostring(tree_after) .. '\n')
    local dlgbtn = gui.get_widget('gal_dialog_btn')
    if dlgbtn then xtaction(dlgbtn, 'notify') end
    local dlgopen = gui.get('gal_dialog', 'open')
    local dlg = gui.get_widget('gal_dialog')
    if dlg then xtaction(dlg, 'cancel') end
    local dlgstatus = gui.get('status', 'message')
    f:write('dialog_open=' .. tostring(dlgopen) .. '\n')
    f:write('dialog_status=' .. tostring(dlgstatus) .. '\n')
    cb_sort_recipe_list(1)
    f:write('header_sort_col=' .. tostring(gui.get('recipe_list', 'sortColumn')) .. '\n')
    f:write('header_sort_status=' .. tostring(gui.get('status', 'message')) .. '\n')
    f:write('tab_after_menu=' .. tostring(tab) .. '\n')
    f:close()
    gui.set('menu_file', 'open', false)
    xtappexitflag()
end)
LUA

export COOKBOOK_PROBE_OUT="$TMP/out.txt"
export COOKBOOK_DATA="$TMP/recipes.lua"
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
(
    cd "$ROOT/commander" || exit 1
    xvfb-run -a -s "-screen 0 1280x800x24" \
        timeout 20 ./commander_runner -Luafile "$TMP/probe.lua" \
        -themeFile ../cookbook/themes/cookbook.hdf -theme default
) >"$TMP/run.log" 2>&1
rc=$?

fail=0
if [ "$rc" -ne 0 ]; then
    echo "FAIL: commander exited $rc" >&2
    fail=1
fi
if grep -qiE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log"; then
    echo "FAIL: fatal pattern in log" >&2
    grep -iE 'Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted' "$TMP/run.log" | head -5 >&2
    fail=1
fi
if [ ! -s "$TMP/out.txt" ]; then
    echo "FAIL: probe output missing" >&2
    tail -20 "$TMP/run.log" >&2
    exit 1
fi

geom() { awk -F= -v k="$1.$2" '$1 == k { print $2; exit }' "$TMP/out.txt"; }
simple() { awk -F= -v k="$1" '$1 == k { print $2; exit }' "$TMP/out.txt"; }
check_gt() {
    local id="$1" key="$2" min="$3"
    local v
    v=$(geom "$id" "$key")
    if [ -z "$v" ] || [ "$v" -le "$min" ]; then
        echo "FAIL: $id.$key=$v (expected > $min)" >&2
        fail=1
    fi
}
check_eq_geom() {
    local id="$1" key="$2" expected="$3"
    local v
    v=$(geom "$id" "$key")
    if [ "$v" != "$expected" ]; then
        echo "FAIL: $id.$key='$v' (expected '$expected')" >&2
        fail=1
    fi
}
check_eq_simple() {
    local key="$1" expected="$2"
    local v
    v=$(simple "$key")
    if [ "$v" != "$expected" ]; then
        echo "FAIL: $key='$v' (expected '$expected')" >&2
        fail=1
    fi
}

check_gt main width 900
check_gt main height 600
check_eq_geom menubar height 28
check_gt toolbar height 0
check_gt toolbar_actions height 35
check_gt tabs height 300
check_gt status height 30
check_eq_simple menu_open true
check_eq_simple confirm_checked true
check_eq_simple confirm_label_has_x true
check_eq_simple combo_label Dessert
check_eq_simple combo_selected 2
check_eq_simple sort_status "Recipes sorted by name"
check_eq_simple sorted_id 3
check_eq_simple sorted_title "Apple Crumble"
check_eq_simple save_status "Saved: Test Dish"
check_eq_simple saved_title "Test Dish"
check_eq_simple count_after_save 6
check_eq_simple persist_file true
check_eq_simple confirm_open true
check_eq_simple delete_status "Deleted: Test Dish"
check_eq_simple count_after_delete 5
check_eq_simple gallery_check true
check_eq_simple gallery_progress 62
check_eq_simple gallery_combo Medium
check_eq_simple gallery_radio 2
check_eq_simple gallery_name_label ""
check_eq_simple gallery_name_placeholder "Type your name"
check_eq_simple tree_before 2
check_eq_simple tree_after 4
check_eq_simple dialog_open true
check_eq_simple dialog_status "Dialog: Cancel"
check_eq_simple header_sort_col 1
check_eq_simple header_sort_status "Sorted by column 2 (ascending)"
check_eq_simple tab_after_menu 1
if ! grep -q '^status_message=Screen:' "$TMP/out.txt"; then
    echo "FAIL: status message did not update" >&2
    fail=1
fi

# Slot order: menu < toolbar < content < status.
awk -F= '
    $1=="menubar.y" { my=$2 }
    $1=="toolbar.y" { ty=$2 }
    $1=="tabs.y"    { cy=$2 }
    $1=="status.y"  { sy=$2 }
    END { exit !(my < ty && ty < cy && cy < sy) }
' "$TMP/out.txt" || {
    echo "FAIL: WAppWindow slot order is wrong" >&2
    cat "$TMP/out.txt" >&2
    fail=1
}

if [ "$fail" -ne 0 ]; then
    echo "test_cookbook_widgets: FAILED"
    cat "$TMP/out.txt" >&2
    exit 1
fi

echo "test_cookbook_widgets: PASSED"
exit 0
