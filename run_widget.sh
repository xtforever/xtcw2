#!/bin/bash
# Run a single LUI widget in a standard 700x600 probe window.
#
# The widget is placed in a host Gridbox that fills the window. A small Quit
# button sits at the bottom right. The generic run_lui.sh runner is unchanged;
# this script supplies the probe harness and forwards runner options.
#
# Usage:
#   ./run_widget.sh [harness options] <fragment|file> [run_lui options...]
#
# Examples:
#   ./run_widget.sh '(Wlabel :label "Hello" :fontSize 20)'
#   ./run_widget.sh tests/widgets/wlistview.lui
#   ./run_widget.sh --title "WspinBox" '(WspinBox :value 42 :min 0 :max 100)'
#   ./run_widget.sh --autoclose 500 '(Wlabel :label "probe")'
#   ./run_widget.sh --after tests/widgets/populate_list.lua tests/widgets/wlistview.lui
#   ./run_widget.sh '(Wlabel :label "probe")' -Tracelevel 5
#
# Harness options must come before the widget. Everything after the widget is
# passed to run_lui.sh / commander_runner.

set -euo pipefail

SOURCE="${BASH_SOURCE[0]}"
while [ -L "$SOURCE" ]; do
    SOURCE_DIR="$(cd -P -- "$(dirname -- "$SOURCE")" && pwd)"
    SOURCE="$(readlink -- "$SOURCE")"
    case "$SOURCE" in
        /*) ;;
        *) SOURCE="$SOURCE_DIR/$SOURCE" ;;
    esac
done
ROOT="$(cd -P -- "$(dirname -- "$SOURCE")" && pwd)"
CALLER_PWD="$(pwd -P)"

usage() {
    cat <<EOF
Usage: $(basename "$0") [harness options] <fragment|file> [run_lui options...]

Runs one LUI widget in a 700x600 window with a bottom-right Quit button.

Harness options:
  --title TITLE       window title (default: "Widget Probe")
  --setup FILE        Lua file run before the probe is built
  --after FILE        Lua file run after the probe is built
  --autoclose MS      exit automatically after MS milliseconds
  --result FILE       write probe/geometry data as key=value lines
  --                  end harness options
  -h, --help          show this help

Examples:
  $(basename "$0") '(Wlabel :label "Hello" :fontSize 20)'
  $(basename "$0") tests/widgets/wlistview.lui
  $(basename "$0") --title "WspinBox" '(WspinBox :value 42 :min 0 :max 100)'
  $(basename "$0") --autoclose 500 '(Wlabel :label "probe")'
  $(basename "$0") --after tests/widgets/populate_list.lua tests/widgets/wlistview.lui
  $(basename "$0") '(Wlabel :label "probe")' -Tracelevel 5
EOF
}

TITLE=""
SETUP=""
AFTER=""
AUTOCLOSE=""
RESULT=""
WIDGET=""
RUNNER_ARGS=()

while [ $# -gt 0 ]; do
    # Once the widget is known, everything else belongs to run_lui.sh.
    if [ -n "$WIDGET" ]; then
        RUNNER_ARGS+=("$@")
        break
    fi

    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        --title)
            [ $# -ge 2 ] || { echo "error: --title requires a value" >&2; exit 2; }
            TITLE="$2"
            shift 2
            ;;
        --setup)
            [ $# -ge 2 ] || { echo "error: --setup requires a file" >&2; exit 2; }
            SETUP="$2"
            shift 2
            ;;
        --after)
            [ $# -ge 2 ] || { echo "error: --after requires a file" >&2; exit 2; }
            AFTER="$2"
            shift 2
            ;;
        --autoclose)
            [ $# -ge 2 ] || { echo "error: --autoclose requires milliseconds" >&2; exit 2; }
            case "$2" in
                ''|*[!0-9]*) echo "error: --autoclose must be a non-negative integer" >&2; exit 2 ;;
            esac
            AUTOCLOSE="$2"
            shift 2
            ;;
        --result)
            [ $# -ge 2 ] || { echo "error: --result requires a file" >&2; exit 2; }
            RESULT="$2"
            shift 2
            ;;
        --)
            shift
            if [ $# -gt 0 ] && [ -z "$WIDGET" ]; then
                WIDGET="$1"
                shift
            fi
            RUNNER_ARGS+=("$@")
            break
            ;;
        -*)
            echo "error: unknown harness option: $1" >&2
            usage >&2
            exit 2
            ;;
        *)
            WIDGET="$1"
            shift
            ;;
    esac
done

if [ -z "$WIDGET" ]; then
    echo "error: missing widget fragment or probe file" >&2
    usage >&2
    exit 2
fi

resolve_path() {
    local p="$1"
    if command -v realpath >/dev/null 2>&1; then
        realpath -m -- "$p"
        return
    fi
    case "$p" in
        /*) printf '%s\n' "$p" ;;
        *)  printf '%s/%s\n' "$CALLER_PWD" "$p" ;;
    esac
}

WIDGET_INLINE=""
WIDGET_FILE=""
if [ -f "$WIDGET" ]; then
    WIDGET_FILE="$(resolve_path "$WIDGET")"
else
    case "$WIDGET" in
        '('*)
            WIDGET_INLINE="$WIDGET"
            ;;
        *)
            echo "error: probe file not found and fragment does not start with '(': $WIDGET" >&2
            exit 2
            ;;
    esac
fi

if [ -n "$SETUP" ]; then
    SETUP="$(resolve_path "$SETUP")"
    [ -f "$SETUP" ] || { echo "error: setup file not found: $SETUP" >&2; exit 2; }
fi
if [ -n "$AFTER" ]; then
    AFTER="$(resolve_path "$AFTER")"
    [ -f "$AFTER" ] || { echo "error: after file not found: $AFTER" >&2; exit 2; }
fi
if [ -n "$RESULT" ]; then
    RESULT="$(resolve_path "$RESULT")"
fi

TITLE="${TITLE:-Widget Probe}"

# The debug commander_runner is ASan-instrumented. Process-global fontconfig,
# Xt and task-manager allocations are reported as leaks on normal shutdown.
# Suppress them for interactive widget probing; set RUN_WIDGET_LEAKS=1 to keep
# leak detection enabled when investigating a widget-specific leak.
if [ "${RUN_WIDGET_LEAKS:-0}" != "1" ]; then
    export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
fi

export LUI_WIDGET="$WIDGET_INLINE"
export LUI_WIDGET_FILE="$WIDGET_FILE"
export LUI_WIDGET_TITLE="$TITLE"
export LUI_WIDGET_SETUP="$SETUP"
export LUI_WIDGET_AFTER="$AFTER"
export LUI_WIDGET_AUTOCLOSE="$AUTOCLOSE"
export LUI_WIDGET_RESULT="$RESULT"

if [ -z "${DISPLAY:-}" ] && [ -z "$AUTOCLOSE" ]; then
    echo "run_widget.sh: DISPLAY is unset; running headless under xvfb-run." >&2
    echo "run_widget.sh: use --autoclose MS for non-interactive runs." >&2
fi

if [ ${#RUNNER_ARGS[@]} -gt 0 ]; then
    exec "$ROOT/run_lui.sh" "$ROOT/lui/widget_harness.lua" "${RUNNER_ARGS[@]}"
else
    exec "$ROOT/run_lui.sh" "$ROOT/lui/widget_harness.lua"
fi
