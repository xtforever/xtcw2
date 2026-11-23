#!/bin/bash
# XTCW Demo Launcher — an xtcw GUI menu for running the LUI demos.
#
#   ./t1.sh                      interactive menu (xtcw GUI; text fallback)
#   ./t1.sh form                 run a demo directly
#   ./t1.sh --trace 50 form      run with TRACE(50) output
#   ./t1.sh cookbook             LUI reference app (multi-screen recipe manager)
#   ./t1.sh native               native Gridbox C test (window kept open)
#
# The menu is built with tools/t1_menu.lua and displays optional rich
# descriptions from demos/desc/<key>.retex.  Without a DISPLAY it falls back
# to a plain text menu.
# Set T1_SKIP_BUILD=1 to skip the start.sh build check (used by tests).
# Set RUN=echo to print the command instead of running it (used by tests).

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- demo registry: key|title|short description ----------------------------
DEMOS=(
  "calc|Calculator|4x6 grid with gridWidth spanning"
  "contacts|Contacts|multi-row form, spinbox, property round-trip"
  "drawcanvas|DrawCanvas|custom drawing via a plug-in C callback"
  "iconeditor|Icon Editor|XPM icon editor with grid, preview, tools and undo"
  "eqfader|EQ Fader|10-band Cairo vertical EQ fader"
  "theme|Theme|central theme registry: colours + borders"
  "form|Form|Gridbox layout: labels, edit, spinbox, toggle"
  "karoed|KaroEd|multiline editor: joins, selection, clipboard, tab"
  "bravo|Wbravo|piece-table editor: retex, undo/redo, clipboard"
  "rich_bravo|Wbravo rich|piece-table editor: styles, paragraph menu, text I/O, snapshots"
  "listview|WListView|themed multi-column list with scrollbar and icons"
  "widget_help|Widget Reference|searchable widget resources and methods (Wmarkdown)"
  "filechooser|File Chooser|WListView-based file chooser + Wbravo text import"
  "login|Login|password fields, toggles, feedback"
  "wbutton|Wbutton|button variants: macros, sizes, alignment, callbacks"
  "wlabel_svg|Wlabel SVG|labels with inline SVG images"
  "wretex|Wretex|multiline retex text with inline SVG"
  "wretex_debug|Wretex debug|fixed-size retex widgets"
  "wtoggleswitch|Toggle Switch|animated oval switch"
  "viewport|Viewport|scrollable view with right/bottom scrollbars"
  "wtabwidget|Tab Widget|tab group with scrollable pages"
  "wmenubutton|Burger Menu|burger button with a tabbed popup menu"
  "cookbook|Cookbook|LUI reference app (multi-screen recipe manager)"
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

print_menu_input() {
    local d key title desc
    for d in "${DEMOS[@]}"; do
        IFS='|' read -r key title desc <<< "$d"
        printf '%s\t%s\t%s\n' "$key" "$title" "$desc"
    done
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [<demo>] [--trace <N>] [-keep]

With no arguments an interactive menu is shown (xtcw GUI when a DISPLAY is
available, otherwise a plain text menu).

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
        --print-menu-input)
            print_menu_input; exit 0 ;;
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

# The cookbook reference app lives outside demos/ and ships its own theme.
run_cookbook() {
    local trace="${1:-}" dargs=()
    dargs=(-Luafile "../cookbook/app/main.lua"
           -themeFile "../cookbook/themes/cookbook.hdf"
           -theme default)
    [ -n "$trace" ] && dargs+=(-Tracelevel "$trace")
    ( cd "$DIR/commander" && $RUN ./commander_runner "${dargs[@]}" )
}

# --- interactive xtcw GUI menu ---------------------------------------------
# Runs tools/t1_menu.lua with the demo registry as TSV.  The menu writes the
# selection to a temp file (key<TAB>trace); its own stdout/stderr are hidden so
# they cannot pollute the captured selection.  Returns 1 on cancel/quit.
run_gui_menu() {
    local input result sel trace d key title desc
    input=$(mktemp "${TMPDIR:-/tmp}/t1_menu_in.XXXXXX") || return 1
    result=$(mktemp "${TMPDIR:-/tmp}/t1_menu_out.XXXXXX") || { rm -f "$input"; return 1; }

    for d in "${DEMOS[@]}"; do
        IFS='|' read -r key title desc <<< "$d"
        printf '%s\t%s\t%s\n' "$key" "$title" "$desc" >> "$input"
    done

    ( cd "$DIR/commander" && \
      T1_MENU_INPUT="$input" T1_MENU_RESULT="$result" T1_ROOT="$DIR" \
      $RUN ./commander_runner -Luafile ../tools/t1_menu.lua ) >/dev/null 2>&1

    sel=""; trace=""
    if [ -s "$result" ]; then
        IFS=$'\t' read -r sel trace < "$result" || true
    fi
    rm -f "$input" "$result"

    # A valid result wins even if the process exited non-zero afterwards
    # (e.g. an ASan/fontconfig leak report on shutdown).
    [ -n "$sel" ] || return 1
    printf '%s\t%s\n' "$sel" "${trace:-0}"
}

run_selected_demo() {
    local key="$1" trace="${2:-0}"
    case "$key" in
        native)   run_native || true ;;
        cookbook) run_cookbook "$trace" || true ;;
        *)        run_lua_demo "$key" "$trace" || true ;;
    esac
}

interactive_gui() {
    while true; do
        local result sel trace
        result=$(run_gui_menu) || exit 0
        IFS=$'\t' read -r sel trace <<< "$result"
        [ -n "$sel" ] || exit 0
        run_selected_demo "$sel" "$trace"
    done
}

# --- fallback text menu when no DISPLAY is available -----------------------
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
            run_selected_demo "$k" 0
            break
        done || exit 0
    done
}

# --- dispatch --------------------------------------------------------------
if [ -n "$DEMO_KEY" ]; then
    if [ "$DEMO_KEY" = native ]; then
        run_native
    elif [ "$DEMO_KEY" = cookbook ]; then
        run_cookbook "$TRACE"
    else
        run_lua_demo "$DEMO_KEY" "$TRACE"
    fi
elif [ -n "${T1_MENU_SELECT:-}" ]; then
    # One-shot selection for tests/scripting: key<TAB>trace.  Split manually
    # so an empty key (leading tab) is preserved and means "cancel".
    sel="${T1_MENU_SELECT%%$'\t'*}"
    if [ "$sel" = "$T1_MENU_SELECT" ]; then
        trace=""
    else
        trace="${T1_MENU_SELECT#*$'\t'}"
    fi
    [ -n "$sel" ] || exit 0
    run_selected_demo "$sel" "${trace:-0}"
elif [ "${T1_MENU_MODE:-}" = cli ]; then
    select_main
elif [ "${T1_MENU_MODE:-}" = gui ]; then
    if [ -z "${DISPLAY:-}" ]; then
        echo "Error: T1_MENU_MODE=gui requires a DISPLAY." >&2
        exit 2
    fi
    interactive_gui
elif [ -n "${DISPLAY:-}" ]; then
    interactive_gui
elif [ -t 0 ] && [ -t 1 ]; then
    select_main
else
    echo "Error: no DISPLAY for the GUI menu and no TTY for the text menu; specify a demo." >&2
    usage >&2
    exit 2
fi
