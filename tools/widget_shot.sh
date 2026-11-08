#!/bin/bash
# widget_shot.sh — render a single wbuild/Xt widget in isolation, capture a
# screenshot, and (optionally) verify the rendering with deepseek-flash.
#
# Usage:
#   tools/widget_shot.sh '<LUI fragment>' [options]
#
# The fragment is the bare widget element, for example:
#   tools/widget_shot.sh '(Wlabel :label "Hello" :fontSize 20)'
#
# The script wraps the fragment in a small window + gridbox container and
# injects the grid placement
#
#     :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3
#
# (:fill 3 == fill:both) before the fragment's closing ')'.  Your fragment
# keeps its own resources (fontSize, alignment, ...); the script only owns the
# layout placement.
#
# Screenshots are written to widget_photo/ by default.
#
# Options:
#   --width N        window width                          (default: 320)
#   --height N       window height                         (default: 160)
#   --out FILE       screenshot path (default widget_photo/<class>_<timestamp>.png)
#   --prompt TEXT    verification prompt for deepseek-flash
#   --wait SECONDS   settle time after the window appears  (default: 1)
#   --no-upload      capture only, skip the deepseek-flash check
#   --model MODEL    opencode model id                     (default: deepseek/deepseek-flash)
#   --agent NAME     opencode agent for the verify step    (default: plan, read-only)
#   --use-display    render on the current $DISPLAY instead of a private Xvfb
#   -h, --help       show this help
#
# Requirements:
#   * the tree is built: run `make all` (commander/commander_runner + libs)
#   * Xvfb (default; only skipped with --use-display), scrot, xwininfo,
#     python3 + Pillow
#   * opencode (only for the verify step; uses the stored deepseek provider auth)

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RUNNER="$ROOT/commander/commander_runner"
PHOTO_DIR="$ROOT/widget_photo"

WIDTH=320
HEIGHT=160
WAIT=1
OUT=""
PROMPT=""
UPLOAD=1
MODEL="deepseek/deepseek-flash"
AGENT="plan"
USE_DISPLAY=0
FRAGMENT=""

DEFAULT_PROMPT='Inspect the attached screenshot of a single Xt/Motif widget rendered in isolation. Check: (1) is the widget background drawn correctly (not black/uninitialized)? (2) is the text visible and not truncated? (3) are there any layout or alignment problems? Do not use any tools; answer briefly based only on the image.'

usage() {
    sed -n '2,/^set -u$/p' "$0" | sed '$d' | sed 's/^# \{0,1\}//'
}

while [ $# -gt 0 ]; do
    case "$1" in
        --width)    WIDTH="$2"; shift 2 ;;
        --height)   HEIGHT="$2"; shift 2 ;;
        --out)      OUT="$2"; shift 2 ;;
        --prompt)   PROMPT="$2"; shift 2 ;;
        --wait)     WAIT="$2"; shift 2 ;;
        --model)    MODEL="$2"; shift 2 ;;
        --agent)    AGENT="$2"; shift 2 ;;
        --no-upload) UPLOAD=0; shift ;;
        --use-display) USE_DISPLAY=1; shift ;;
        -h|--help)  usage; exit 0 ;;
        --)         shift; if [ $# -gt 0 ] && [ -z "$FRAGMENT" ]; then FRAGMENT="$1"; shift; fi; break ;;
        -*)         echo "widget_shot.sh: unknown option: $1" >&2; usage >&2; exit 2 ;;
        *)
            if [ -z "$FRAGMENT" ]; then FRAGMENT="$1"; shift
            else echo "widget_shot.sh: too many positional arguments" >&2; exit 2; fi ;;
    esac
done

if [ -z "$FRAGMENT" ]; then
    echo "widget_shot.sh: missing widget fragment (e.g. '(Wlabel :label \"Hi\")')" >&2
    usage >&2
    exit 2
fi
if ! [ -x "$RUNNER" ]; then
    echo "widget_shot.sh: $RUNNER not found. Run 'make all' first." >&2
    exit 3
fi

PROMPT="${PROMPT:-$DEFAULT_PROMPT}"

# --- derive class name and inject the grid placement attributes -------------
# Only the FIRST line carries the widget class; sed would otherwise rewrite
# every nested child line in a multi-line fragment.
CLASS="$(printf '%s\n' "$FRAGMENT" | head -n 1 | sed -E 's/^[[:space:]]*\(([A-Za-z0-9_]+).*/\1/')"
[ -n "$CLASS" ] || CLASS="widget"
# sanitize to a safe, single-token filename component
CLASS="$(printf '%s' "$CLASS" | tr -c 'A-Za-z0-9_.-' '_' | sed -E 's/_+/_/g; s/^_+|_+$//g')"
[ -n "$CLASS" ] || CLASS="widget"

# strip trailing whitespace, then insert the grid attrs before the last ')'
FRAG_TRIMMED="$(printf '%s' "$FRAGMENT" | sed -e 's/[[:space:]]*$//')"
case "$FRAG_TRIMMED" in
    *')') INNER="${FRAG_TRIMMED%\)} :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3)" ;;
    *)    INNER="($FRAG_TRIMMED :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3)" ;;
esac

# --- output path ------------------------------------------------------------
mkdir -p "$PHOTO_DIR"
if [ -z "$OUT" ]; then
    OUT="$PHOTO_DIR/${CLASS}_$(date +%Y%m%d-%H%M%S).png"
elif [ "${OUT#/}" = "$OUT" ]; then
    # relative path -> resolve against the repo root
    OUT="$ROOT/$OUT"
fi

# --- temp workspace + cleanup ----------------------------------------------
TMPD="$(mktemp -d /tmp/widget_shot.XXXXXX)"
LUAFILE="$TMPD/shot.lua"
RAW="$TMPD/raw.png"
LOG="$TMPD/runner.log"
XVFB_PID=""
RUNNER_PID=""
STARTED_XVFB=0

cleanup() {
    if [ -n "$RUNNER_PID" ]; then
        kill -9 "$RUNNER_PID" 2>/dev/null
        pkill -9 -P "$RUNNER_PID" 2>/dev/null
    fi
    if [ "$STARTED_XVFB" = 1 ] && [ -n "$XVFB_PID" ]; then
        kill -9 "$XVFB_PID" 2>/dev/null
    fi
    rm -rf "$TMPD"
}
trap cleanup EXIT INT TERM

# --- write the LUI file (literal fragment, no shell expansion) --------------
{
    printf '(window :id "w" :title "widget_shot" :width %s :height %s\n' "$WIDTH" "$HEIGHT"
    printf '  (grid :id "g" :weightx 1 :weighty 1\n'
    printf '    %s))\n' "$INNER"
} > "$LUAFILE"

# --- ensure an X display ----------------------------------------------------
# Default: a private Xvfb, so the capture is isolated and deterministic and
# never picks up unrelated windows from the user's desktop.  --use-display
# renders on the current $DISPLAY instead (useful to watch the widget live).
if [ "$USE_DISPLAY" = 1 ] && [ -n "${DISPLAY:-}" ]; then
    echo "widget_shot.sh: rendering on existing DISPLAY=$DISPLAY" >&2
else
    DISPLAY_NUM=""
    for d in $(seq 99 130); do
        if [ ! -e "/tmp/.X${d}-lock" ] && [ ! -e "/tmp/.X11-unix/X${d}" ]; then
            DISPLAY_NUM=":$d"
            break
        fi
    done
    if [ -z "$DISPLAY_NUM" ]; then
        echo "widget_shot.sh: could not find a free X display (99-130)" >&2
        exit 4
    fi
    Xvfb "$DISPLAY_NUM" -screen 0 "$((WIDTH + 80))x$((HEIGHT + 80))x24" >/dev/null 2>&1 &
    XVFB_PID=$!
    STARTED_XVFB=1
    export DISPLAY="$DISPLAY_NUM"
    for _ in $(seq 1 50); do
        xwininfo -root >/dev/null 2>&1 && break
        sleep 0.1
    done
fi

# --- run the widget ---------------------------------------------------------
( cd "$ROOT/commander" && exec env DISPLAY="$DISPLAY" "$RUNNER" -Luafile "$LUAFILE" ) >"$LOG" 2>&1 &
RUNNER_PID=$!
disown "$RUNNER_PID" 2>/dev/null || true

# Wait for the top-level window ("commander_runner" / class "luarunner").
FOUND=0
for _ in $(seq 1 150); do
    if DISPLAY="$DISPLAY" xwininfo -root -tree 2>/dev/null | grep -q 'luarunner'; then
        FOUND=1
        break
    fi
    if ! kill -0 "$RUNNER_PID" 2>/dev/null; then
        break
    fi
    sleep 0.1
done

if [ "$FOUND" = 0 ]; then
    echo "widget_shot.sh: widget window did not appear" >&2
    echo "--- commander_runner log ---" >&2
    tail -n 30 "$LOG" >&2
    exit 5
fi

# The required settle time before the screenshot.
sleep "$WAIT"

# --- capture ----------------------------------------------------------------
if ! DISPLAY="$DISPLAY" scrot -o "$RAW" 2>"$TMPD/scrot.err"; then
    echo "widget_shot.sh: scrot failed: $(cat "$TMPD/scrot.err" 2>/dev/null)" >&2
    exit 6
fi

DISPLAY="$DISPLAY" python3 - "$RAW" "$OUT" <<'PY'
import os, re, subprocess, sys
from PIL import Image

raw, out = sys.argv[1], sys.argv[2]
try:
    tree = subprocess.check_output(
        ["xwininfo", "-root", "-tree"],
        env={**os.environ},
        text=True, errors="replace",
    )
except Exception:
    tree = ""

geom = None
for line in tree.splitlines():
    if 'luarunner' not in line:
        continue
    # Match the geometry "WxH" only when followed by "+X+Y", so the window id
    # (e.g. "0x20000c") is not mistaken for a size.
    m = re.search(r'(\d+)x(\d+)(?=[+-]\d+[+-]\d+)', line)
    if not m:
        continue
    w, h = int(m.group(1)), int(m.group(2))
    if w <= 0 or h <= 0:
        continue
    # Offsets after "WxH": relative +X+Y then absolute +X+Y. Use absolute.
    offs = re.findall(r'[+-]\d+', line[m.end():])
    if len(offs) >= 4:
        x, y = int(offs[2]), int(offs[3])
    elif len(offs) >= 2:
        x, y = int(offs[0]), int(offs[1])
    else:
        x, y = 0, 0
    geom = (x, y, x + w, y + h)
    break

img = Image.open(raw)
if geom:
    img = img.crop(geom)
img.save(out)
print("widget_shot.sh: saved screenshot -> %s%s" % (
    out, (" (cropped %dx%d+%d+%d)" % (geom[2]-geom[0], geom[3]-geom[1], geom[0], geom[1])) if geom else " (full screen)"))
PY

# --- verify with deepseek-flash via opencode --------------------------------
STATUS=0
if [ "$UPLOAD" = 1 ]; then
    if ! command -v opencode >/dev/null 2>&1; then
        echo "widget_shot.sh: opencode not found; skipping verification (use --no-upload to silence)" >&2
    else
        echo "widget_shot.sh: verifying with $MODEL via opencode ..." >&2
        # --file is an array option, so put the prompt after "--" to keep it a
        # positional message rather than another file path.
        ( cd "$ROOT" && opencode run --model "$MODEL" --agent "$AGENT" --file "$OUT" -- "$PROMPT" )
        STATUS=$?
        if [ "$STATUS" -ne 0 ]; then
            echo "widget_shot.sh: opencode verify exited with status $STATUS" >&2
        fi
    fi
fi

exit "$STATUS"
