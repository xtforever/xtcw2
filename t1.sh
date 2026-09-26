#!/bin/bash
# XTCW Demo Launcher — a whiptail TUI for running the LUI demos.
#
#   ./t1.sh                      interactive menu (whiptail)
#   ./t1.sh form                 run a demo directly
#   ./t1.sh --trace 50 form      run with TRACE(50) output
#   ./t1.sh native               native Gridbox C test (window kept open)
#
# When no DISPLAY is set, xvfb-run is used automatically.
# Set T1_SKIP_BUILD=1 to skip the start.sh build check (used by tests).
# Set RUN=echo to print the command instead of running it (used by tests).

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- demo registry: key|title|short description ----------------------------
DEMOS=(
  "calc|Calculator|4x6 grid with gridWidth spanning"
  "contacts|Contacts|multi-row form, spinbox, property round-trip"
  "drawcanvas|DrawCanvas|custom drawing via a plug-in C callback"
  "form|Form|Gridbox layout: labels, edit, spinbox, toggle"
  "karoed|KaroEd|multiline editor: joins, selection, clipboard, tab"
  "login|Login|password fields, toggles, feedback"
  "wbutton|Wbutton|button variants: macros, sizes, alignment, callbacks"
  "wlabel_svg|Wlabel SVG|labels with inline SVG images"
  "wretex|Wretex|multiline retex text with inline SVG"
  "wretex_debug|Wretex debug|fixed-size retex widgets"
  "native|Native C|Gridbox constraint-resource C test"
)

is_demo() {
    local d key
    for d in "${DEMOS[@]}"; do
        IFS='|' read -r key _ _ <<< "$d"
        [ "$key" = "$1" ] && return 0
    done
    return 1
}

demo_lines() {
    local d key title desc
    for d in "${DEMOS[@]}"; do
        IFS='|' read -r key title desc <<< "$d"
        printf '  %-14s %s - %s\n' "$key" "$title" "$desc"
    done
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [<demo>] [--trace <N>] [-keep]

With no arguments an interactive menu is shown.

Demos:
$(demo_lines)

Options:
  --trace <N>   Xt trace level (0 = off; 2, 5, 50, ...)
  -keep         (native only) keep the test window open
EOF
}

# --- build guard: auto-build missing artifacts via start.sh ----------------
if [ "${T1_SKIP_BUILD:-0}" != "1" ]; then
    "$DIR/start.sh" || { echo "Build failed." >&2; exit 1; }
fi

# --- display selection -----------------------------------------------------
if [ -n "${DISPLAY:-}" ]; then
    DEFAULT_RUN=""
else
    DEFAULT_RUN="xvfb-run --auto-servernum"
fi
# Honour a pre-set RUN (e.g. RUN=echo for tests).
RUN="${RUN:-$DEFAULT_RUN}"

# --- argument parsing ------------------------------------------------------
DEMO_KEY=""
TRACE=""
KEEP=0

while [ $# -gt 0 ]; do
    case "$1" in
        --trace)
            [ $# -ge 2 ] || { echo "Error: --trace requires a value." >&2; exit 2; }
            case "$2" in
                ''|*[!0-9]*) echo "Error: --trace value must be a non-negative integer." >&2; exit 2 ;;
            esac
            TRACE="$2"; shift 2 ;;
        -keep)
            KEEP=1; shift ;;
        -h|--help)
            usage; exit 0 ;;
        -*)
            echo "Error: unknown option: $1" >&2; usage >&2; exit 2 ;;
        *)
            [ -z "$DEMO_KEY" ] || { echo "Error: too many arguments." >&2; exit 2; }
            DEMO_KEY="$1"; shift ;;
    esac
done

if [ -n "$DEMO_KEY" ] && ! is_demo "$DEMO_KEY"; then
    echo "Error: unknown demo: $DEMO_KEY" >&2
    usage >&2
    exit 2
fi
if [ "$KEEP" = 1 ] && [ "$DEMO_KEY" != native ]; then
    echo "Error: -keep is only valid for the 'native' demo." >&2
    exit 2
fi

# --- run helpers -----------------------------------------------------------
# $RUN is intentionally unquoted: it may hold "xvfb-run --auto-servernum".
run_lua_demo() {
    local key="$1" trace="${2:-}" dargs=()
    dargs=(-Luafile "../demos/demo_${key}.lua")
    [ -n "$trace" ] && dargs+=(-Tracelevel "$trace")
    ( cd "$DIR/commander" && $RUN ./commander_runner "${dargs[@]}" )
}

run_native() {
    ( cd "$DIR/experimental/test_gridbox" && make >/dev/null && $RUN ./test_gridbox -keep )
}

# --- interactive whiptail menu ---------------------------------------------
pick_demo() {
    local d key title desc items=()
    for d in "${DEMOS[@]}"; do
        IFS='|' read -r key title desc <<< "$d"
        items+=("$key" "$title - $desc")
    done
    items+=("quit" "Exit the launcher")
    whiptail --title "XTCW Demo Launcher" \
        --menu "Select a demo to run:" 20 78 12 \
        "${items[@]}" 3>&1 1>&2 2>&3
}

pick_trace() {
    whiptail --title "Trace level" \
        --menu "TRACE output level for the demo:" 15 64 5 \
        "none" "0  - no trace output" \
        "2"    "2  - high-level flow" \
        "5"    "5  - detailed" \
        "50"   "50 - widget-level TRACE" \
        "back" "return to the demo menu" \
        3>&1 1>&2 2>&3
}

interactive_main() {
    while true; do
        local sel tsel trace=""
        sel=$(pick_demo) || exit 0          # Cancel/Esc on the main menu quits
        case "$sel" in
            ""|quit) exit 0 ;;
        esac

        if [ "$sel" = native ]; then
            run_native || true
            continue
        fi

        tsel=$(pick_trace) || continue      # Cancel on trace menu -> back to demos
        case "$tsel" in
            ""|back) continue ;;
            none) trace="0" ;;
            *) trace="$tsel" ;;
        esac

        run_lua_demo "$sel" "$trace" || true
    done
}

# --- fallback menu when whiptail is unavailable ----------------------------
select_main() {
    local labels=() keys=() d key title desc k
    for d in "${DEMOS[@]}"; do
        IFS='|' read -r key title desc <<< "$d"
        keys+=("$key")
        labels+=("$title - $desc")
    done
    while true; do
        local choice
        PS3="Select a demo (or Quit): "
        select choice in "${labels[@]}" "Quit"; do
            if [ -z "$choice" ]; then
                echo "Invalid selection."
                break
            fi
            if [ "$choice" = "Quit" ]; then
                exit 0
            fi
            k="${keys[$((REPLY - 1))]}"
            if [ "$k" = native ]; then
                run_native || true
            else
                run_lua_demo "$k" || true
            fi
            break
        done || exit 0
    done
}

# --- dispatch --------------------------------------------------------------
if [ -n "$DEMO_KEY" ]; then
    if [ "$DEMO_KEY" = native ]; then
        run_native
    else
        run_lua_demo "$DEMO_KEY" "$TRACE"
    fi
elif command -v whiptail >/dev/null 2>&1 && [ -t 0 ] && [ -t 1 ]; then
    interactive_main
elif [ -t 0 ] && [ -t 1 ]; then
    select_main
else
    echo "Error: no TTY for the interactive menu; specify a demo." >&2
    usage >&2
    exit 2
fi
