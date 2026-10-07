#!/bin/bash
# Commander Runner — Demo Smoke Tests (headless, Xvfb)
#
# Runs every LUI demo and the Wbutton functional test against a virtual X
# server, verifying:
#   * the process starts and stays alive (no crash),
#   * no Lua/bootstrap/ASan errors are logged,
#   * a window is actually created,
#   * the window renders non-blank content,
#   * (Wbutton) widgets, macros and callbacks work.
set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
COMMANDER="$SCRIPT_DIR/commander_runner"
DEMOS_DIR="$ROOT/demos"
DISPLAY_NUM=":99"
TMPDIR="${TMPDIR:-/tmp}/lui_smoke"
mkdir -p "$TMPDIR"

# commander_runner must run from commander/ so '../lui/?.lua' resolves.
cd "$SCRIPT_DIR"

PASS=0
FAIL=0

RED='\033[0;31m'; GRN='\033[0;32m'; YLW='\033[1;33m'; BLU='\033[0;34m'; RST='\033[0m'
log()  { echo -e "${BLU}[TEST]${RST} $*"; }
pass() { echo -e "${GRN}[PASS]${RST} $*"; PASS=$((PASS+1)); }
fail() { echo -e "${RED}[FAIL]${RST} $*"; FAIL=$((FAIL+1)); }

ERR_PATTERN='Segmentation fault|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error|AddressSanitizer|Assertion|Aborted'

# Start a single Xvfb for all tests.
Xvfb "$DISPLAY_NUM" -screen 0 1280x800x24 >/dev/null 2>&1 &
XVFB_PID=$!
sleep 2
if ! kill -0 "$XVFB_PID" 2>/dev/null; then
    echo "FATAL: could not start Xvfb on $DISPLAY_NUM"
    exit 2
fi

cleanup_runner() {
    kill -9 "$1" 2>/dev/null || true
    pkill -9 -P "$1" 2>/dev/null || true
}

# Check the demo window region of the screenshot is not blank (rendered content).
check_screenshot() {
    local shot="$1" name="$2"
    if ! DISPLAY="$DISPLAY_NUM" scrot -o "$shot" 2>/dev/null; then
        fail "$name: scrot failed"
        return
    fi
    local nonblack
    nonblack=$(python3 - "$shot" <<'PY'
import sys
from PIL import Image
import numpy as np
try:
    a = np.asarray(Image.open(sys.argv[1]).convert('L'))
except Exception as e:
    print(-1); sys.exit(0)
region = a[0:450, 0:600]
print(int((region > 16).sum()))
PY
)
    if [ "$nonblack" -gt 1000 ]; then
        pass "$name: renders content ($nonblack non-black px)"
    else
        fail "$name: blank render ($nonblack non-black px)"
    fi
}

run_demo() {
    local lua="$1" name="$2"
    local log="$TMPDIR/${name}.log"
    local shot="$TMPDIR/${name}.png"

    log "Demo: $name"

    DISPLAY="$DISPLAY_NUM" "$COMMANDER" -Luafile "$lua" >"$log" 2>&1 &
    local pid=$!
    sleep 3

    if ! kill -0 "$pid" 2>/dev/null; then
        fail "$name: process exited early (crash?)"
        tail -n 5 "$log" | sed 's/^/      /'
        cleanup_runner "$pid"
        return
    fi

    if grep -qiE "$ERR_PATTERN" "$log"; then
        fail "$name: errors in log"
        grep -iE "$ERR_PATTERN" "$log" | head -3 | sed 's/^/      /'
    else
        pass "$name: no errors in log"
    fi

    if DISPLAY="$DISPLAY_NUM" xwininfo -root -tree 2>/dev/null | grep -q "luarunner"; then
        pass "$name: window created"
    else
        fail "$name: no window found"
    fi

    check_screenshot "$shot" "$name"

    cleanup_runner "$pid"
    sleep 0.3
}

# ── Wbutton functional test (macros + callbacks + properties) ──

functional_wbutton() {
    local name="wbutton_functional"
    local result="$TMPDIR/${name}_result.txt"
    local log="$TMPDIR/${name}.log"
    local tmp_lua="$TMPDIR/${name}.lua"
    rm -f "$result"

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local ui = [[
(defmacro grid_opts (gx gy wx wy)
  :gridx gx :gridy gy :weightx wx :weighty wy)

(window :id "w" :title "Wbutton Test" :width 500 :height 420
  (grid :id "g" :weightx 1 :weighty 1
    (Wbutton :id "btn1" :label "Button 1" (grid_opts 0 1 1 0) :fill 3 :callback "LUA(on_btn)")
    (Wbutton :id "btn2" :label "Button 2" (grid_opts 1 1 1 0) :fill 3 :callback "LUA(on_btn)")
    (label :id "status" :label "Status: idle" (grid_opts 0 3 1 0) :fill 3)
  ))
]]

local clicks = 0
function on_btn()
    clicks = clicks + 1
    gui.set("status", "label", "Clicked " .. clicks)
end

lui.run(ui)

xtapptimeout(600, function()
    local f = io.open("$result", "w")
    local b1 = gui.get_widget("btn1")
    local b2 = gui.get_widget("btn2")
    f:write("btn1_created=" .. tostring(b1 ~= nil) .. "\n")
    f:write("btn2_created=" .. tostring(b2 ~= nil) .. "\n")
    f:write("btn1_label=" .. tostring(gui.get("btn1", "label")) .. "\n")

    if b1 then xtaction(b1, "notify") end
    if b2 then xtaction(b2, "notify") end

    xtapptimeout(400, function()
        f:write("clicks=" .. tostring(clicks) .. "\n")
        f:write("status=" .. tostring(gui.get("status", "label")) .. "\n")
        f:close()
        xtappexitflag()
    end)
end)
LUAEOF

    log "Functional: Wbutton (macro + callback + properties)"

    timeout 20 env DISPLAY="$DISPLAY_NUM" "$COMMANDER" -Luafile "$tmp_lua" >"$log" 2>&1
    rm -f "$tmp_lua"

    if [ -f "$result" ]; then
        local b1 b2 label clicks status
        b1=$(grep 'btn1_created=' "$result" | cut -d= -f2)
        b2=$(grep 'btn2_created=' "$result" | cut -d= -f2)
        label=$(grep 'btn1_label=' "$result" | cut -d= -f2)
        clicks=$(grep 'clicks=' "$result" | cut -d= -f2)
        status=$(grep 'status=' "$result" | cut -d= -f2)

        [ "$b1" = "true" ] && pass "Wbutton btn1 created" || fail "Wbutton btn1 not created"
        [ "$b2" = "true" ] && pass "Wbutton btn2 created" || fail "Wbutton btn2 not created"
        [ "$label" = "Button 1" ] && pass "Wbutton label correct" || fail "Wbutton label: '$label'"
        [ "$clicks" = "2" ] && pass "callback fired 2 times" || fail "callback fired $clicks times (expected 2)"
        echo "$status" | grep -q "Clicked" && pass "status label updated" || fail "status label: '$status'"
    else
        fail "Wbutton functional: no result file (script did not self-complete)"
        tail -n 5 "$log" | sed 's/^/      /'
    fi
}

# ── Inline function-valued callback (not LUA(name)) ──

functional_inline_callback() {
    local name="inline_callback"
    local result="$TMPDIR/${name}_result.txt"
    local log="$TMPDIR/${name}.log"
    local tmp_lua="$TMPDIR/${name}.lua"
    rm -f "$result"

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')
local backend = require('backend_xt')

local cb_fired = 0
local root = backend.build({
    'window', {keyword=':id'}, 'w', {keyword=':title'}, 'Inline CB', {keyword=':width'}, 300, {keyword=':height'}, 150,
    {'grid', {keyword=':id'}, 'g',
        {'Wbutton', {keyword=':id'}, 'b1', {keyword=':label'}, 'Click',
            {keyword=':callback'}, function() cb_fired = cb_fired + 1; gui.set('status','label','cb '..cb_fired) end},
        {'label', {keyword=':id'}, 'status', {keyword=':label'}, 'idle'}
    }
})
xtmanage(root)

xtapptimeout(500, function()
    local b = gui.get_widget("b1")
    if b then xtaction(b, "notify") end
    xtapptimeout(300, function()
        local f = io.open("$result", "w")
        f:write("cb_fired=" .. tostring(cb_fired) .. "\n")
        f:write("status=" .. tostring(gui.get("status", "label")) .. "\n")
        f:close()
        xtappexitflag()
    end)
end)
LUAEOF

    log "Functional: inline function callback"
    timeout 20 env DISPLAY="$DISPLAY_NUM" "$COMMANDER" -Luafile "$tmp_lua" >"$log" 2>&1
    rm -f "$tmp_lua"

    if [ -f "$result" ]; then
        local fired status
        fired=$(grep 'cb_fired=' "$result" | cut -d= -f2)
        status=$(grep 'status=' "$result" | cut -d= -f2)
        [ "$fired" = "1" ] && pass "inline callback fired" || fail "inline callback fired $fired times"
        [ "$status" = "cb 1" ] && pass "inline callback updated label" || fail "inline status label: '$status'"
    else
        fail "inline callback: no result file"
        tail -n 5 "$log" | sed 's/^/      /'
    fi
}

# ── Task copy end-to-end (task bindings + immediate event dispatch) ──

functional_task_copy() {
    local name="task_copy"
    local result="$TMPDIR/${name}_result.txt"
    local log="$TMPDIR/${name}.log"
    local tmp_lua="$TMPDIR/${name}.lua"
    rm -f "$result"

    cat > "$tmp_lua" <<LUAEOF
local gui = require('gui_xt')

local src = "$TMPDIR/task_src.txt"
local dst = "$TMPDIR/task_dst.txt"
local f = io.open(src, "w"); f:write("hello task world"); f:close()
os.remove(dst)

local complete = false
local err = false
function on_task(msg)
    if msg.type == TASK_EVENT_COMPLETE then complete = true end
    if msg.type == TASK_EVENT_ERROR then err = true end
end

lui.run([[
(window :id "w" :title "Task Test" :width 300 :height 150
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "status" :label "running" :gridx 0 :gridy 0)))
]])

task_set_handler("on_task")
local job = task_copy(src, dst)

xtapptimeout(2000, function()
    local d = io.open(dst, "r")
    local content = d and d:read("*a") or "NIL"
    if d then d:close() end
    local f = io.open("$result", "w")
    f:write("job=" .. tostring(job) .. "\n")
    f:write("complete=" .. tostring(complete) .. "\n")
    f:write("err=" .. tostring(err) .. "\n")
    f:write("dst=" .. tostring(content) .. "\n")
    f:close()
    xtappexitflag()
end)
LUAEOF

    log "Functional: task_copy end-to-end"
    timeout 25 env DISPLAY="$DISPLAY_NUM" "$COMMANDER" -Luafile "$tmp_lua" >"$log" 2>&1
    rm -f "$tmp_lua"

    if [ -f "$result" ]; then
        local job complete err dst
        job=$(grep 'job=' "$result" | cut -d= -f2)
        complete=$(grep 'complete=' "$result" | cut -d= -f2)
        err=$(grep 'err=' "$result" | cut -d= -f2)
        dst=$(grep 'dst=' "$result" | cut -d= -f2)

        [ -n "$job" ] && [ "$job" -ge 1 ] 2>/dev/null && pass "task_copy returned job id $job" || fail "task_copy job id: '$job'"
        [ "$complete" = "true" ] && pass "task completion event fired" || fail "task complete: '$complete'"
        [ "$err" = "false" ] && pass "no task error" || fail "task error: '$err'"
        [ "$dst" = "hello task world" ] && pass "file copied correctly" || fail "dst content: '$dst'"
    else
        fail "task_copy: no result file"
        tail -n 5 "$log" | sed 's/^/      /'
    fi
}

echo ""
echo "=========================================="
echo "  Commander Runner — Demo Smoke Tests"
echo "=========================================="
echo ""

# Rebuild to be sure the binary matches the source.
log "Building commander_runner"
if ! (cd "$SCRIPT_DIR" && make commander_runner >/dev/null 2>&1); then
    fail "build failed"
    kill -9 "$XVFB_PID" 2>/dev/null
    exit 1
fi
pass "build OK"

for f in "$DEMOS_DIR"/demo_*.lua; do
    [ -e "$f" ] || continue
    name="$(basename "$f" .lua)"
    run_demo "$f" "$name"
done

functional_wbutton
functional_inline_callback
functional_task_copy

kill -9 "$XVFB_PID" 2>/dev/null || true

echo ""
echo "=========================================="
echo "  RESULTS: $PASS passed, $FAIL failed"
echo "=========================================="

[ "$FAIL" -gt 0 ] && exit 1
exit 0
