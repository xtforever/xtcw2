#!/bin/bash
# Run the AdmPanel launcher behind the memory/time guard.
#
# This is the safe way to click around in the real test app. A runaway widget
# layout loop is killed at the cgroup memory limit instead of taking the
# desktop session down with the OOM killer, and the log/backtrace is kept.
#
# Usage:
#   apps/admpanel/run_guarded.sh [options] <config.lua|script.sh>
#
# Options (defaults in parentheses):
#   --mem MB        hard memory limit (1024)
#   --timeout SEC   wall-clock limit, 0 disables (600)
#   --outdir DIR    artifact directory (temporary /tmp/xtcw-guard.*)
#   --gdb           run under gdb and save a backtrace on a signal
#   --no-cgroup     do not use systemd-run
#   --quiet         do not print the log tail on failure
#   -h, --help      show this help
#
# Environment: ADMPANEL_GUARD_MEM_MB, ADMPANEL_GUARD_TIMEOUT_S,
#              ADMPANEL_GUARD_OUTDIR
#
# Examples:
#   apps/admpanel/run_guarded.sh apps/admpanel/examples/freigabe.lua
#   apps/admpanel/run_guarded.sh --mem 512 --gdb tests/fixtures/launcher_described.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

MEM_MB="${ADMPANEL_GUARD_MEM_MB:-1024}"
TIMEOUT_S="${ADMPANEL_GUARD_TIMEOUT_S:-600}"
OUTDIR="${ADMPANEL_GUARD_OUTDIR:-}"
GUARD_ARGS=()

usage() {
    sed -n '2,25p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

while [ $# -gt 0 ]; do
    case "$1" in
        --mem)
            [ $# -ge 2 ] || { echo "error: --mem requires a value" >&2; exit 2; }
            MEM_MB="$2"; shift 2 ;;
        --timeout)
            [ $# -ge 2 ] || { echo "error: --timeout requires a value" >&2; exit 2; }
            TIMEOUT_S="$2"; shift 2 ;;
        --outdir)
            [ $# -ge 2 ] || { echo "error: --outdir requires a value" >&2; exit 2; }
            OUTDIR="$2"; shift 2 ;;
        --gdb)
            GUARD_ARGS+=(--gdb); shift ;;
        --no-cgroup)
            GUARD_ARGS+=(--no-cgroup); shift ;;
        --quiet)
            GUARD_ARGS+=(--quiet); shift ;;
        -h|--help)
            usage; exit 0 ;;
        --)
            shift; break ;;
        -*)
            echo "error: unknown option: $1" >&2
            usage >&2
            exit 2 ;;
        *)
            break ;;
    esac
done

if [ $# -lt 1 ]; then
    echo "error: missing config.lua or script.sh target" >&2
    usage >&2
    exit 2
fi

if [ -n "$OUTDIR" ]; then
    GUARD_ARGS+=(--outdir "$OUTDIR")
fi

# Process-global fontconfig/Xt caches are reported as leaks on normal exit.
# Suppress them unless the caller explicitly wants a leak hunt.
if [ "${ADMPANEL_GUARD_LEAKS:-0}" != "1" ]; then
    export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
fi

exec "$ROOT/tools/run_guarded.sh" \
    --mem "$MEM_MB" --timeout "$TIMEOUT_S" "${GUARD_ARGS[@]}" \
    -- "$ROOT/apps/admpanel/run.sh" "$@"
