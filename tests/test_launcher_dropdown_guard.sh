#!/bin/bash
# Headless guard test for the AdmPanel drop-down (WComboBox) popup.
#
# Starts the real launcher under apps/admpanel/run_guarded.sh, clicks the
# first combo, closes the popup with Escape, quits, and checks that the
# process stayed inside the memory cap. This is the automated version of
# "open the test app and click the drop-down" that previously took the
# session down with a runaway layout loop.
#
# Optional:
#   ADMPANEL_DROPDOWN_ITEMS=N   number of static combo items (default: 7)
#   ADMPANEL_DROPDOWN_MEM_MB=N  guard memory cap (default: 256)
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/admpanel_dropdown.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

if ! command -v xdotool >/dev/null 2>&1; then
    echo "test_launcher_dropdown_guard: SKIPPED (xdotool not installed)"
    exit 0
fi
if ! command -v xvfb-run >/dev/null 2>&1; then
    echo "test_launcher_dropdown_guard: SKIPPED (xvfb-run not installed)"
    exit 0
fi

BIN="$ROOT/commander/commander_runner"
if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make commander ) >"$TMP/build.log" 2>&1 || {
        echo "FAIL: commander_runner build failed" >&2
        tail -20 "$TMP/build.log" >&2
        exit 1
    }
fi

ITEMS="${ADMPANEL_DROPDOWN_ITEMS:-7}"
MEM_MB="${ADMPANEL_DROPDOWN_MEM_MB:-256}"

cat >"$TMP/config.lua" <<'LUA'
local values = {}
for i = 1, tonumber(os.getenv('ADMPANEL_DROPDOWN_ITEMS') or '7') do
    values[i] = i
end
return {
    title = 'Dropdown Guard Test',
    script = 'echo $sandbox',
    fields = {
        { type = 'option', title = 'sandbox', variable = 'sandbox',
          values = values },
    },
}
LUA

cat >"$TMP/inner.sh" <<'SH'
#!/bin/bash
set -u
ROOT="$1"
TMP="$2"
ITEMS="$3"
MEM_MB="$4"

export ADMPANEL_DROPDOWN_ITEMS="$ITEMS"
"$ROOT/apps/admpanel/run_guarded.sh" \
    --mem "$MEM_MB" --timeout 20 --quiet --outdir "$TMP/guard" \
    "$TMP/config.lua" >"$TMP/guard.out" 2>&1 &
guard_pid=$!

wid=""
for _ in {1..60}; do
    wid=$(xdotool search --class 'commander_runner' 2>/dev/null | head -1)
    [ -n "$wid" ] || wid=$(xdotool search --name 'commander_runner' 2>/dev/null | head -1)
    [ -n "$wid" ] && break
    sleep 0.2
done
if [ -z "$wid" ]; then
    echo "no commander_runner window" >"$TMP/inner.err"
    wait "$guard_pid" 2>/dev/null || true
    exit 1
fi

xdotool windowfocus "$wid" >/dev/null 2>&1 || true
# The form is deterministic: first combo sits in the first form row.
# Open and dismiss with Escape.
xdotool mousemove --window "$wid" 480 90 click 1
sleep 1
xdotool key --window "$wid" Escape
sleep 1
# Open again and commit the first popup row (the popup starts below the combo).
xdotool mousemove --window "$wid" 480 90 click 1
sleep 1
xdotool mousemove --window "$wid" 300 123 click 1
sleep 1
# Quit button in the action row.
xdotool mousemove --window "$wid" 716 618 click 1

wait "$guard_pid" 2>/dev/null
exit $?
SH
chmod +x "$TMP/inner.sh"

if ! xvfb-run --auto-servernum --server-args='-screen 0 1280x1024x24' \
        bash "$TMP/inner.sh" "$ROOT" "$TMP" "$ITEMS" "$MEM_MB" \
        >"$TMP/xvfb.log" 2>&1; then
    echo "FAIL: guarded launcher/click run failed" >&2
    cat "$TMP/guard.out" >&2 2>/dev/null || true
    tail -30 "$TMP/xvfb.log" >&2 2>/dev/null || true
    [ -f "$TMP/inner.err" ] && cat "$TMP/inner.err" >&2
    if [ -f "$TMP/guard/run.log" ]; then
        echo "--- last log lines ---" >&2
        tail -30 "$TMP/guard/run.log" >&2
    fi
    exit 1
fi

if [ ! -f "$TMP/guard/meta.txt" ]; then
    echo "FAIL: guard meta.txt missing" >&2
    cat "$TMP/guard.out" >&2 2>/dev/null || true
    exit 1
fi

status=$(sed -n 's/^status=//p' "$TMP/guard/meta.txt")
exit_code=$(sed -n 's/^exit_code=//p' "$TMP/guard/meta.txt")
peak=$(sed -n 's/^peak_memory_mb=//p' "$TMP/guard/meta.txt")
[ -n "$peak" ] || peak=0

fail=0
[ "$status" = "ok" ] || { echo "FAIL: guard status=$status" >&2; fail=1; }
[ "$exit_code" = "0" ] || { echo "FAIL: app exit=$exit_code" >&2; fail=1; }
if [ "$peak" -ge "$MEM_MB" ]; then
    echo "FAIL: peak ${peak}MB reached the ${MEM_MB}MB cap" >&2
    fail=1
fi

if [ "$fail" -ne 0 ]; then
    echo "--- guard.out ---" >&2
    cat "$TMP/guard.out" >&2 2>/dev/null || true
    echo "--- last log lines ---" >&2
    tail -40 "$TMP/guard/run.log" >&2 2>/dev/null || true
    exit 1
fi

echo "test_launcher_dropdown_guard: OK (${ITEMS} items, peak ${peak}MB/${MEM_MB}MB)"
