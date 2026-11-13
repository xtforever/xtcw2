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

xtapptimeout(900, function()
    local f = io.open(os.getenv('COOKBOOK_PROBE_OUT'), 'w')
    for _, id in ipairs({'main','menubar','toolbar','toolbar_actions','tabs','status',
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
    local mi = gui.get_widget('mi_detail')
    if mi then xtaction(mi, 'notify') end
    local tab = gui.get('tabs', 'selected')
    f:write('menu_open=' .. tostring(open) .. '\n')
    f:write('confirm_checked=' .. tostring(confirm_checked) .. '\n')
    f:write('confirm_label_has_x=' .. tostring(confirm_label:find('%[x%]') ~= nil) .. '\n')
    f:write('tab_after_menu=' .. tostring(tab) .. '\n')
    f:close()
    gui.set('menu_file', 'open', false)
    xtappexitflag()
end)
LUA

export COOKBOOK_PROBE_OUT="$TMP/out.txt"
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
check_gt main height 700
check_eq_geom menubar height 28
check_gt toolbar height 0
check_gt toolbar_actions height 35
check_gt tabs height 300
check_gt status height 0
check_eq_simple menu_open true
check_eq_simple confirm_checked true
check_eq_simple confirm_label_has_x true
check_eq_simple tab_after_menu 1

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
