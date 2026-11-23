#!/bin/bash
# Tests for the xtcw demo launcher menu (tools/t1_menu.lua + t1.sh wiring).
#
# The GUI part runs headless under Xvfb and uses the T1_MENU_AUTOSELECT hook
# to pick a demo from the real t1.sh registry.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/t1_menu_test.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

PASS=0
FAIL=0
ok()  { echo "ok - $1"; PASS=$((PASS + 1)); }
bad() { echo "FAIL - $1"; FAIL=$((FAIL + 1)); }

# 1. Registry export (the data from t1.sh).
if T1_SKIP_BUILD=1 "$ROOT/t1.sh" --print-menu-input > "$TMP/menu.tsv" 2>"$TMP/err1"; then
    ok "registry export succeeds"
else
    bad "registry export succeeds"
fi

for key in calc form theme cookbook native; do
    if awk -F'\t' -v k="$key" '$1 == k { found = 1 } END { exit !found }' "$TMP/menu.tsv"; then
        ok "registry contains $key"
    else
        bad "registry contains $key"
    fi
done

n=$(wc -l < "$TMP/menu.tsv")
if [ "$n" -ge 20 ]; then
    ok "registry has $n demos"
else
    bad "registry has $n demos"
fi

# 2. Direct CLI mode is unchanged.
if T1_SKIP_BUILD=1 RUN=echo "$ROOT/t1.sh" form > "$TMP/direct.out" 2>&1 \
   && grep -q 'demo_form.lua' "$TMP/direct.out"; then
    ok "direct demo still works"
else
    bad "direct demo still works"
fi

# 3. One-shot menu selection injection.
if T1_SKIP_BUILD=1 RUN=echo T1_MENU_SELECT=$'form\t5' "$ROOT/t1.sh" > "$TMP/inject.out" 2>&1 \
   && grep -q 'demo_form.lua' "$TMP/inject.out" \
   && grep -q -- '-Tracelevel 5' "$TMP/inject.out"; then
    ok "T1_MENU_SELECT runs the selected demo with trace"
else
    bad "T1_MENU_SELECT runs the selected demo with trace"
fi

# 4. Headless GUI menu with the real registry.  The CI test job builds only
# the toolkit libraries, so build commander_runner on demand.
GUI_OK=1
if [ ! -x "$ROOT/commander/commander_runner" ]; then
    if ( cd "$ROOT" && make commander ) >"$TMP/commander-build.log" 2>&1; then
        ok "built commander_runner"
    else
        bad "built commander_runner"
        tail -20 "$TMP/commander-build.log" 2>/dev/null | sed 's/^/    /'
        GUI_OK=0
    fi
fi

if [ "$GUI_OK" -eq 1 ]; then
    (
        cd "$ROOT/commander" && \
        T1_MENU_INPUT="$TMP/menu.tsv" \
        T1_MENU_RESULT="$TMP/result.tsv" \
        T1_MENU_DESC_OUT="$TMP/desc.txt" \
        T1_MENU_LAYOUT_OUT="$TMP/layout.txt" \
        T1_ROOT="$ROOT" \
        T1_MENU_AUTOSELECT=theme \
        T1_MENU_AUTOTRACE=5 \
        xvfb-run --auto-servernum ./commander_runner -Luafile ../tools/t1_menu.lua
    ) >"$TMP/menu.out" 2>"$TMP/menu.err"

    if [ -s "$TMP/result.tsv" ]; then
        IFS=$'\t' read -r key trace < "$TMP/result.tsv"
        if [ "$key" = theme ] && [ "$trace" = 5 ]; then
            ok "menu writes the selected key and trace"
        else
            bad "menu writes the selected key and trace (got '$key' '$trace')"
        fi
    else
        bad "menu writes a result file"
    fi

    if grep -q '\\bf' "$TMP/desc.txt" 2>/dev/null \
       && grep -q 'LuaRunner/file.svg' "$TMP/desc.txt" 2>/dev/null; then
        ok "menu loads and resolves the .retex description"
    else
        bad "menu loads and resolves the .retex description"
    fi

    if [ "$(cat "$TMP/layout.txt" 2>/dev/null)" = "720" ]; then
        ok "menu uses a splitter with the configured fraction"
    else
        bad "menu uses a splitter with the configured fraction"
    fi
else
    bad "menu writes a result file (commander_runner unavailable)"
    bad "menu loads and resolves the .retex description (commander_runner unavailable)"
    bad "menu uses a splitter with the configured fraction (commander_runner unavailable)"
fi

# 4b. Selecting a row must update the description (not just the initial row).
if [ "$GUI_OK" -eq 1 ]; then
    (
        cd "$ROOT/commander" && \
        T1_MENU_INPUT="$TMP/menu.tsv" \
        T1_MENU_RESULT="$TMP/late-result.tsv" \
        T1_MENU_DESC_OUT="$TMP/late-desc.txt" \
        T1_ROOT="$ROOT" \
        T1_MENU_LATE_SELECT=5 \
        xvfb-run --auto-servernum ./commander_runner -Luafile ../tools/t1_menu.lua
    ) >"$TMP/late.out" 2>"$TMP/late.err"

    if grep -q 'theme registry' "$TMP/late-desc.txt" 2>/dev/null; then
        ok "selection updates the description"
    else
        bad "selection updates the description"
    fi
else
    bad "selection updates the description (commander_runner unavailable)"
fi

# 5. An empty selection must not run anything.
if T1_SKIP_BUILD=1 RUN=echo T1_MENU_SELECT=$'\t0' "$ROOT/t1.sh" > "$TMP/cancel.out" 2>&1; then
    if [ ! -s "$TMP/cancel.out" ]; then
        ok "empty selection does nothing"
    else
        bad "empty selection does nothing"
    fi
else
    bad "empty selection does nothing"
fi

if [ "$FAIL" -ne 0 ]; then
    echo "test_t1_menu: $FAIL check(s) FAILED"
    exit 1
fi
echo "test_t1_menu: PASSED"
exit 0
