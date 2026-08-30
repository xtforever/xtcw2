#!/bin/bash
# Widget Test Runner — runs LUI widget tests under xvfb-run,
# captures TRACE(50) output, verifies widget creation, properties, and actions.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
COMMANDER="$PROJECT_DIR/experimental/commander_runner"
TRACE_LEVEL=50
RESULTS_DIR="$SCRIPT_DIR/test_results"
PASS=0
FAIL=0

mkdir -p "$RESULTS_DIR"

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[1;33m'
BLU='\033[0;34m'
RST='\033[0m'

log()   { echo -e "${BLU}[TEST]${RST} $*"; }
pass()  { echo -e "${GRN}[PASS]${RST} $*"; PASS=$((PASS+1)); }
fail()  { echo -e "${RED}[FAIL]${RST} $*"; FAIL=$((FAIL+1)); }

assert_trace_has() {
    local log="$1" pattern="$2" desc="$3"
    if grep -q "$pattern" "$log" 2>/dev/null; then
        pass "$desc"
    else
        fail "$desc — pattern not found: $pattern"
    fi
}

assert_trace_count() {
    local log="$1" pattern="$2" min="$3" desc="$4"
    local count
    count=$(grep -c "$pattern" "$log" 2>/dev/null || true)
    if [ "$count" -ge "$min" ]; then
        pass "$desc (found $count, need >= $min)"
    else
        fail "$desc (found $count, need >= $min)"
    fi
}

run_test() {
    local lua_file="$1"
    local test_name="$2"
    local timeout_secs="${3:-10}"
    local logbase="$RESULTS_DIR/${test_name}"

    log "Running: $test_name"
    log "  lua=$lua_file"

    (
        cd "$PROJECT_DIR/experimental"
        xvfb-run --auto-servernum \
            ./commander_runner -Tracelevel "$TRACE_LEVEL" -Luafile "$lua_file" \
            >"${logbase}" 2>&1 &
        local pid=$!
        local waited=0
        while kill -0 "$pid" 2>/dev/null && [ "$waited" -lt "$((timeout_secs * 2))" ]; do
            sleep 0.5
            waited=$((waited+1))
        done
        if kill -0 "$pid" 2>/dev/null; then
            kill -9 "$pid" 2>/dev/null || true
            sleep 0.1
            kill -9 $(pgrep -P "$pid") 2>/dev/null || true
            wait "$pid" 2>/dev/null || true
        else
            wait "$pid" 2>/dev/null || true
        fi
    ) &

    local wrapper_pid=$!
    # Wait with overall timeout
    local waited=0
    while kill -0 "$wrapper_pid" 2>/dev/null && [ "$waited" -lt "$((timeout_secs * 2 + 10))" ]; do
        sleep 0.5
        waited=$((waited+1))
    done
    if kill -0 "$wrapper_pid" 2>/dev/null; then
        kill -9 "$wrapper_pid" 2>/dev/null || true
    fi

    LAST_LOG="${logbase}"
    LAST_OUT="${logbase}"
    sleep 0.2

    local lines
    lines=$(wc -l < "$LAST_LOG" 2>/dev/null || echo 0)
    log "  Captured $lines lines of TRACE output"
    local trace50
    trace50=$(grep -c '\[50\]' "$LAST_LOG" 2>/dev/null || echo 0)
    log "  TRACE(50) lines: $trace50"
}

# ── Test 1: Widget Creation and Properties ──────────────────────

test_widget_creation() {
    local test_name="widget_creation"
    local tmp_lua
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')

local ui = [[
(window :id "tw" :title "Test Win" :width 200 :height 150
  (grid :id "gr"
    (command :id "btn1" :label "OK" :callback "LUA(on_btn)")
    (label :id "lbl1" :label "Hello")
  ))
]]

function on_btn() end

lui.run(ui)

xtapptimeout(500, function()
    local f = io.open("${result_file}", "w")
    f:write("btn1=" .. tostring(gui.get("btn1", "label")) .. "\n")
    f:write("lbl1=" .. tostring(gui.get("lbl1", "label")) .. "\n")

    gui.set("btn1", "label", "Changed")
    f:write("btn1_changed=" .. tostring(gui.get("btn1", "label")) .. "\n")

    f:close()
    xtapptimeout(200, function() xtappexitflag() end)
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 10

    if [ -f "$result_file" ]; then
        local btn_orig lbl_orig btn_ch
        btn_orig=$(grep 'btn1=' "$result_file" | cut -d= -f2)
        lbl_orig=$(grep 'lbl1=' "$result_file" | cut -d= -f2)
        btn_ch=$(grep 'btn1_changed=' "$result_file" | cut -d= -f2)

        [ "$btn_orig" = "OK" ] && pass "Button initial label correct" || fail "Button initial label: '$btn_orig', expected 'OK'"
        [ "$lbl_orig" = "Hello" ] && pass "Label initial text correct" || fail "Label initial text: '$lbl_orig', expected 'Hello'"
        [ "$btn_ch" = "Changed" ] && pass "Label set/get round-trip works" || fail "Label round-trip: '$btn_ch', expected 'Changed'"
    else
        fail "No result file produced — widgets may not have been created"
    fi

    rm -f "$tmp_lua"
}

# ── Test 2: Button Actions with Callback ──────────────────────

test_button_callback() {
    local test_name="button_callback"
    local tmp_lua
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')

local ui = [[
(window :id "tw" :title "Button Test" :width 200 :height 100
  (grid :id "gr"
    (command :id "btn1" :label "Press Me" :callback "LUA(on_press)")
    (label :id "result" :label "Not clicked")
  ))
]]

local click_count = 0

function on_press()
    click_count = click_count + 1
    gui.set("result", "label", "Clicked " .. click_count .. " times")
end

lui.run(ui)

-- Command widget requires set() before notify() — notify guards on command.set flag
-- Callbacks dispatched via 100ms polling timer, must wait
xtapptimeout(500, function()
    local b = gui.get_widget("btn1")
    if b then
        xtaction(b, "set")
        xtaction(b, "notify")
        xtaction(b, "unset")
    end

    xtapptimeout(300, function()
        local f = io.open("${result_file}", "w")
        f:write("click_count=" .. tostring(click_count) .. "\n")
        f:write("result=" .. tostring(gui.get("result", "label")) .. "\n")
        f:close()
        xtappexitflag()
    end)
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 10

    if [ -f "$result_file" ]; then
        local cnt res
        cnt=$(grep 'click_count=' "$result_file" | cut -d= -f2)
        res=$(grep 'result=' "$result_file" | cut -d= -f2)

        [ "$cnt" = "1" ] && pass "Button callback fired once" || fail "Button callback count: $cnt, expected 1"
        echo "$res" | grep -q "Clicked" && pass "Result label updated after click" || fail "Result label: '$res', expected 'Clicked...'"
    else
        fail "No result file for button callback test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 3: Custom Widget Actions (WpixBtn with TRACE(50)) ─────

test_wpixbtn_action() {
    local test_name="wpixbtn_action"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<'LUAEOF'
local gui = require('gui_xt')

local ui = [[
(window :id "tw" :title "WpixBtn Test" :width 200 :height 100
  (grid :id "gr"
    (WpixBtn :id "pb1" :callback "LUA(on_pb)")
  ))
]]

function on_pb() end

lui.run(ui)

xtapptimeout(500, function()
    local w = gui.get_widget("pb1")
    if w then
        xtaction(w, "highlight")
        xtaction(w, "notify")
        xtaction(w, "reset")
    end
    xtapptimeout(200, function() xtappexitflag() end)
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 10

    # WpixBtn has TRACE(50) on highlight, notify, reset (added in wbuild_widgets/WpixBtn.c)
    assert_trace_has "$LAST_LOG" '\[50\]' "TRACE(50) level output present"
    assert_trace_has "$LAST_LOG" 'WpixBtn' "WpixBtn widget action TRACE found"

    rm -f "$tmp_lua"
}

# ── Test 4: TRACE(50) Level Verification ──────────────────────

test_trace_level() {
    local test_name="trace_level"

    local log="$RESULTS_DIR/wpixbtn_action"

    if [ -f "$log" ]; then
        local trace50_count
        trace50_count=$(grep -c '\[50\]' "$log" 2>/dev/null || echo 0)
        if [ "$trace50_count" -ge 1 ]; then
            pass "TRACE(50) level is active (found $trace50_count entries in WpixBtn test)"
        else
            fail "TRACE(50) level not active in WpixBtn test (found 0 entries)"
        fi
    else
        fail "WpixBtn test log not found for trace_level verification"
    fi
}

# ── Test 5: SpinBox property round-trip ──────────────────────

test_spinbox_properties() {
    local test_name="spinbox_props"
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local ui = [[
(window :id "tw" :title "SpinBox Props" :width 300 :height 100
  (grid :id "gr"
    (WspinBox :id "spin" :gridx 0 :gridy 0 :value 5 :min 0 :max 100)
  ))
]]
lui.run(ui)
xtapptimeout(500, function()
    local f = io.open("${result_file}", "w")
    f:write("value_init=" .. tostring(gui.get("spin", "value")) .. "\n")
    f:write("min_init=" .. tostring(gui.get("spin", "min")) .. "\n")
    f:write("max_init=" .. tostring(gui.get("spin", "max")) .. "\n")
    gui.set("spin", "value", 42)
    f:write("value_after=" .. tostring(gui.get("spin", "value")) .. "\n")
    local w = gui.get_widget("spin")
    f:write("created=" .. tostring(w ~= nil) .. "\n")
    f:close()
    xtapptimeout(200, function() xtappexitflag() end)
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 10

    if [ -f "$result_file" ]; then
        local v_init v_after min_v max_v created
        v_init=$(grep 'value_init=' "$result_file" | cut -d= -f2)
        v_after=$(grep 'value_after=' "$result_file" | cut -d= -f2)
        min_v=$(grep 'min_init=' "$result_file" | cut -d= -f2)
        max_v=$(grep 'max_init=' "$result_file" | cut -d= -f2)
        created=$(grep 'created=' "$result_file" | cut -d= -f2)

        [ "$created" = "true" ] && pass "SpinBox widget created" || fail "SpinBox not created"
        [ "$v_init" = "5" ] && pass "SpinBox initial value=5" || fail "SpinBox initial value: '$v_init', expected 5"
        [ "$v_after" = "42" ] && pass "SpinBox value round-trip 5→42" || fail "SpinBox value after set: '$v_after', expected 42"
        [ "$min_v" = "0" ] && pass "SpinBox min=0" || fail "SpinBox min: '$min_v', expected 0"
        [ "$max_v" = "100" ] && pass "SpinBox max=100" || fail "SpinBox max: '$max_v', expected 100"
    else
        fail "No result file for SpinBox property test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 6: Edit/Password property round-trip ──────────────

test_edit_properties() {
    local test_name="edit_props"
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local ui = [[
(window :id "tw" :title "Edit Props" :width 400 :height 100
  (grid :id "gr"
    (edit :id "ed" :gridx 0 :gridy 0 :weightx 1 :label "StartText")
    (Wpassword :id "pw" :gridx 0 :gridy 1 :weightx 1 :label "Secret")
  ))
]]
lui.run(ui)
xtapptimeout(500, function()
    local f = io.open("${result_file}", "w")
    local ed_val = gui.get("ed", "label")
    local pw_val = gui.get("pw", "label")
    f:write("ed_init=" .. tostring(ed_val) .. "\n")
    f:write("pw_init=" .. tostring(pw_val) .. "\n")
    gui.set("ed", "label", "Changed")
    gui.set("pw", "label", "NewSecret")
    f:write("ed_after=" .. tostring(gui.get("ed", "label")) .. "\n")
    f:write("pw_after=" .. tostring(gui.get("pw", "label")) .. "\n")
    f:write("ed_value_fallback=" .. tostring(gui.get("ed", "value")) .. "\n")
    f:close()
    xtapptimeout(200, function() xtappexitflag() end)
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 10

    if [ -f "$result_file" ]; then
        local ed_init pw_init ed_after pw_after ed_fallback
        ed_init=$(grep 'ed_init=' "$result_file" | cut -d= -f2)
        pw_init=$(grep 'pw_init=' "$result_file" | cut -d= -f2)
        ed_after=$(grep 'ed_after=' "$result_file" | cut -d= -f2)
        pw_after=$(grep 'pw_after=' "$result_file" | cut -d= -f2)
        ed_fallback=$(grep 'ed_value_fallback=' "$result_file" | cut -d= -f2)

        [ "$ed_init" = "StartText" ] && pass "Edit initial label" || fail "Edit initial label: '$ed_init', expected 'StartText'"
        [ "$pw_init" = "Secret" ] && pass "Password initial label" || fail "Password initial label: '$pw_init', expected 'Secret'"
        [ "$ed_after" = "Changed" ] && pass "Edit label round-trip" || fail "Edit after set: '$ed_after', expected 'Changed'"
        [ "$pw_after" = "NewSecret" ] && pass "Password label round-trip" || fail "Password after set: '$pw_after', expected 'NewSecret'"
        [ "$ed_fallback" = "Changed" ] && pass "Edit value→label fallback" || fail "Edit value fallback: '$ed_fallback', expected 'Changed'"
    else
        fail "No result file for Edit property test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 7: Toggle state round-trip ──────────────────────

test_toggle_state() {
    local test_name="toggle_state"
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local ui = [[
(window :id "tw" :title "Toggle Test" :width 300 :height 100
  (grid :id "gr"
    (toggle :id "tog" :label "Toggle Me" :gridx 0 :gridy 0 :callback "LUA(on_tog)")
    (label :id "state_lbl" :label "OFF" :gridx 1 :gridy 0)
  ))
]]
local toggled = false
function on_tog()
    toggled = true
    local s = gui.get("tog", "state")
    gui.set("state_lbl", "label", "State: " .. tostring(s))
end
lui.run(ui)
xtapptimeout(500, function()
    local f = io.open("${result_file}", "w")
    f:write("initial_state=" .. tostring(gui.get("tog", "state")) .. "\n")
    local b = gui.get_widget("tog")
    if b then
        xtaction(b, "set")
        xtaction(b, "notify")
        xtaction(b, "unset")
    end
    xtapptimeout(300, function()
        f:write("toggled=" .. tostring(toggled) .. "\n")
        f:write("final_label=" .. tostring(gui.get("state_lbl", "label")) .. "\n")
        f:close()
        xtappexitflag()
    end)
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 10

    if [ -f "$result_file" ]; then
        local init toggled_result final_label
        init=$(grep 'initial_state=' "$result_file" | cut -d= -f2)
        toggled_result=$(grep 'toggled=' "$result_file" | cut -d= -f2)
        final_label=$(grep 'final_label=' "$result_file" | cut -d= -f2)

        [ "$toggled_result" = "true" ] && pass "Toggle callback fired" || fail "Toggle callback: '$toggled_result', expected 'true'"
        echo "$final_label" | grep -q "State:" && pass "Toggle state label updated" || fail "Toggle state label: '$final_label'"
    else
        fail "No result file for Toggle test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 8: All-widgets creation smoke test ──────────────

test_all_widgets_creation() {
    local test_name="all_widgets"
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local store = gui.create_store(2)
gui.store_append(store, {"A", "B"})
local tags = {
    'window', 'label', 'Wlabel', 'button', 'Wbutton', 'command', 'toggle',
    'edit', 'Wedit', 'grid', 'WpixBtn', 'separator', 'check',
    'list-view', 'splitter', 'Wsplitter', 'WspinBox', 'Wpassword', 'Wcombo', 'wlist4'
}
local ui = '(window :id "w" :title "All" :width 600 :height 400\n  (grid :id "g"\n'
for i, tag in ipairs(tags) do
    ui = ui .. '    (' .. tag .. ' :id "w_' .. tag .. '"'
    if tag == 'command' or tag == 'button' or tag == 'Wbutton' or tag == 'WpixBtn' then
        ui = ui .. ' :label "Test"'
    end
    if tag == 'label' or tag == 'Wlabel' or tag == 'Wedit' or tag == 'edit' or tag == 'Wpassword' then
        ui = ui .. ' :label "Hello"'
    end
    if tag == 'WspinBox' then ui = ui .. ' :value 5' end
    if tag == 'Wcombo' then ui = ui .. ' :label "Choose"' end
    if tag == 'list-view' then
        ui = ui .. ' :columns (200 80) :model ' .. tostring(store.handle)
    end
    if tag == 'splitter' or tag == 'Wsplitter' then ui = ui .. ' :fraction 300' end
    ui = ui .. ')\n'
end
ui = ui .. '  ))\n'
lui.run(ui)
xtapptimeout(800, function()
    local f = io.open("${result_file}", "w")
    local found = 0
    local missing = {}
    for _, tag in ipairs(tags) do
        local w = gui.get_widget("w_" .. tag)
        if w then found = found + 1 else table.insert(missing, tag) end
    end
    f:write("count=" .. found .. "/" .. #tags .. "\n")
    if #missing > 0 then f:write("missing=" .. table.concat(missing, ",") .. "\n") end
    f:close()
    xtappexitflag()
end)
LUAEOF

    run_test "$tmp_lua" "$test_name" 12

    if [ -f "$result_file" ]; then
        local count_line missing
        count_line=$(grep 'count=' "$result_file" | cut -d= -f2)
        missing=$(grep 'missing=' "$result_file" | cut -d= -f2)

        local found total
        found=$(echo "$count_line" | cut -d/ -f1)
        total=$(echo "$count_line" | cut -d/ -f2)

        if [ -n "$missing" ]; then
            fail "Missing widgets: $missing"
        else
            pass "All widgets created ($found/$total)"
        fi
    else
        fail "No result file for all-widgets test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 9: Widget geometry (L-level) ──────────────────────

test_widget_geometry() {
    local test_name="widget_geometry"
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local ui = [[
(window :id "tw" :title "Geometry Test" :width 300 :height 200
  (grid :id "gr"
    (label :id "lbl" :label "Hello" :gridx 0 :gridy 0 :weightx 1)
    (command :id "btn" :label "OK" :gridx 0 :gridy 1 :weightx 1)
    (WspinBox :id "spin" :gridx 0 :gridy 2 :value 5)
  ))
]]
lui.run(ui)
xterror_count(true)
xtapptimeout(1500, function()
    local f = io.open("/dev/shm/${test_name}_result.txt", "w")
    local ids = {"gr", "lbl", "btn", "spin"}
    for _, id in ipairs(ids) do
        local g = gui.geometry(id)
        if g then
            f:write(id .. "_w=" .. tostring(g.width) .. "\n")
            f:write(id .. "_h=" .. tostring(g.height) .. "\n")
            f:write(id .. "_pos=" .. tostring(g.width > 0 and g.height > 0) .. "\n")
        else
            f:write(id .. "=NIL\n")
        end
    end
    f:write("xerrors=" .. tostring(xterror_count(false)) .. "\n")
    f:close()
    xtappexitflag()
end)
LUAEOF

    (
        cd "$PROJECT_DIR/experimental"
        xvfb-run --auto-servernum \
            ./commander_runner -Tracelevel "$TRACE_LEVEL" -Luafile "$tmp_lua" \
            >/dev/null 2>&1
    ) &
    local pid=$!
    local waited=0
    while kill -0 "$pid" 2>/dev/null && [ "$waited" -lt 20 ]; do
        sleep 0.5
        waited=$((waited+1))
    done
    kill -9 "$pid" 2>/dev/null || true
    killall -9 Xvfb 2>/dev/null || true
    sleep 0.3

    LAST_LOG="$RESULTS_DIR/${test_name}"
    if [ -f "/dev/shm/${test_name}_result.txt" ]; then
        cp "/dev/shm/${test_name}_result.txt" "$result_file"
        rm -f "/dev/shm/${test_name}_result.txt"
    fi

    if [ -f "$result_file" ]; then
        local ids="gr lbl btn spin"
        for id in $ids; do
            local pos
            pos=$(grep "${id}_pos=" "$result_file" | cut -d= -f2)
            if [ "$pos" = "true" ]; then
                pass "Geometry: $id has positive dimensions"
            else
                local w h
                w=$(grep "${id}_w=" "$result_file" | cut -d= -f2)
                h=$(grep "${id}_h=" "$result_file" | cut -d= -f2)
                fail "Geometry: $id has invalid dimensions ($w x $h)"
            fi
        done
        local xerrors
        xerrors=$(grep 'xerrors=' "$result_file" | cut -d= -f2)
        if [ "$xerrors" = "0" ]; then
            pass "No X11 protocol errors during geometry test"
        else
            fail "X11 errors during geometry test: $xerrors"
        fi
    else
        fail "No result file for geometry test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 10: X11 error detection ──────────────────────

test_x11_errors() {
    local test_name="x11_errors"
    local result_file="$RESULTS_DIR/${test_name}_result.txt"
    local tmp_lua
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
xterror_count(true)
local ui = [[
(window :id "tw" :title "Error Test" :width 300 :height 200
  (grid :id "gr"
    (label :id "lbl" :label "Test" :gridx 0 :gridy 0)
    (command :id "btn" :label "OK" :gridx 0 :gridy 1)
    (toggle :id "tog" :label "Toggle" :gridx 0 :gridy 2)
    (WspinBox :id "spin" :gridx 0 :gridy 3 :value 1)
  ))
]]
lui.run(ui)
xtapptimeout(800, function()
    local f = io.open("/dev/shm/${test_name}_result.txt", "w")
    local errs = xterror_count(false)
    f:write("create_errors=" .. tostring(errs) .. "\n")

    gui.set("btn", "label", "Changed")
    gui.set("spin", "value", 99)
    local b = gui.get_widget("btn")
    if b then
        xtaction(b, "set")
        xtaction(b, "notify")
        xtaction(b, "unset")
    end

    xtapptimeout(300, function()
        local errs2 = xterror_count(false)
        f:write("total_errors=" .. tostring(errs2) .. "\n")
        f:write("action_errors=" .. tostring(errs2 - errs) .. "\n")
        f:close()
        xtappexitflag()
    end)
end)
LUAEOF

    (
        cd "$PROJECT_DIR/experimental"
        xvfb-run --auto-servernum \
            ./commander_runner -Tracelevel "$TRACE_LEVEL" -Luafile "$tmp_lua" \
            >/dev/null 2>&1
    ) &
    local pid=$!
    local waited=0
    while kill -0 "$pid" 2>/dev/null && [ "$waited" -lt 20 ]; do
        sleep 0.5
        waited=$((waited+1))
    done
    kill -9 "$pid" 2>/dev/null || true
    killall -9 Xvfb 2>/dev/null || true
    sleep 0.3

    if [ -f "/dev/shm/${test_name}_result.txt" ]; then
        cp "/dev/shm/${test_name}_result.txt" "$result_file"
        rm -f "/dev/shm/${test_name}_result.txt"
    fi

    if [ -f "$result_file" ]; then
        local create_errors total_errors
        create_errors=$(grep 'create_errors=' "$result_file" | cut -d= -f2)
        total_errors=$(grep 'total_errors=' "$result_file" | cut -d= -f2)

        if [ "$create_errors" = "0" ]; then
            pass "No X11 errors during widget creation"
        else
            fail "X11 errors during creation: $create_errors"
        fi
        if [ "$total_errors" = "0" ]; then
            pass "No X11 errors during actions/properties"
        else
            fail "X11 errors during actions: $total_errors total"
        fi
    else
        fail "No result file for X11 error test"
    fi

    rm -f "$tmp_lua"
}

# ── Test 11: Wcombo and Wlabel TRACE(50) actions ──────────

test_combo_wlabel_trace() {
    local test_name="combo_wlabel_trace"
    local tmp_lua
    local log_file="$RESULTS_DIR/${test_name}"
    tmp_lua=$(mktemp "${RESULTS_DIR}/${test_name}_XXXXXX.lua")

    cat > "$tmp_lua" <<'LUAEOF'
local gui = require('gui_xt')
local ui = [[
(window :id "tw" :title "Combo/Wlabel Trace" :width 300 :height 150
  (grid :id "gr"
    (Wcombo :id "cb" :gridx 0 :gridy 0 :weightx 1 :callback "LUA(on_cb)")
    (Wlabel :id "wl" :label "Selectable text" :gridx 0 :gridy 1 :weightx 1)
  ))
]]
function on_cb() end
lui.run(ui)
xtapptimeout(500, function()
    local cb = gui.get_widget("cb")
    local wl = gui.get_widget("wl")
    if cb then
        xtaction(cb, "focus_in")
        xtaction(cb, "set_cursor")
        xtaction(cb, "focus_out")
    end
    if wl then
        xtaction(wl, "select_start")
        xtaction(wl, "select_end")
        xtaction(wl, "info")
    end
    xtapptimeout(300, function() xtappexitflag() end)
end)
LUAEOF

    (
        cd "$PROJECT_DIR/experimental"
        xvfb-run --auto-servernum \
            ./commander_runner -Tracelevel 50 -Luafile "$tmp_lua" \
            >"${log_file}" 2>&1
    ) &
    local pid=$!
    local waited=0
    while kill -0 "$pid" 2>/dev/null && [ "$waited" -lt 20 ]; do
        sleep 0.5
        waited=$((waited+1))
    done
    kill -9 "$pid" 2>/dev/null || true
    killall -9 Xvfb 2>/dev/null || true
    sleep 0.3

    LAST_LOG="${log_file}"

    local lines
    lines=$(wc -l < "$LAST_LOG" 2>/dev/null || echo 0)
    log "  Captured $lines lines of TRACE output"
    local trace50
    trace50=$(grep -c '\[50\]' "$LAST_LOG" 2>/dev/null || echo 0)
    log "  TRACE(50) lines: $trace50"

    assert_trace_has "$LAST_LOG" '\[50\].*Wcombo' "Wcombo TRACE(50) actions found"
    assert_trace_has "$LAST_LOG" '\[50\].*Wlabel' "Wlabel TRACE(50) actions found"

    rm -f "$tmp_lua"
}

# ── Run All ────────────────────────────────────────────────────

echo ""
echo "=========================================="
echo "  Widget Test Suite — TRACE(50) Capture"
echo "=========================================="
echo ""

test_widget_creation
test_button_callback
test_wpixbtn_action
test_trace_level
test_spinbox_properties
test_edit_properties
test_toggle_state
test_all_widgets_creation
killall -9 commander_runner Xvfb 2>/dev/null || true
sleep 0.5
test_widget_geometry
test_x11_errors
test_combo_wlabel_trace

echo ""
echo "=========================================="
echo "  RESULTS: $PASS passed, $FAIL failed"
echo "=========================================="

[ "$FAIL" -gt 0 ] && exit 1
exit 0