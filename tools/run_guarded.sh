#!/bin/bash
# Run a command (usually a GUI test app) behind a memory/time guard.
#
# Why: a layout/leak loop in a widget can allocate hundreds of MB per second
# and take the whole desktop session down with the OOM killer. This wrapper
# puts the process into a transient cgroup with a hard MemoryMax, captures
# stdout/stderr, samples peak memory and optionally runs under gdb so a fatal
# signal still leaves a backtrace behind.
#
# Usage:
#   tools/run_guarded.sh [options] -- <command> [args...]
#
# Options:
#   --mem MB        hard memory limit in MiB (default: 1024)
#   --high MB       optional soft memory limit in MiB (throttles, not kills)
#   --timeout SEC   wall-clock limit in seconds, 0 disables (default: 120)
#   --outdir DIR    artifact directory (default: /tmp/xtcw-guard.<time>.<pid>)
#   --gdb           run under gdb -batch and save a full backtrace on a signal
#   --no-cgroup     do not use systemd-run (falls back to a plain process)
#   --ulimit-v      in fallback mode, cap RLIMIT_AS with --mem (breaks ASan)
#   --quiet         do not print the log tail on failure
#   -h, --help      show this help
#
# Exit status is the command's status. A clean exit with a fatal pattern in
# the log is reported as 1. SIGKILL (137) is annotated as a likely cgroup OOM.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

MEM_MB=1024
HIGH_MB=""
TIMEOUT_S=120
OUTDIR=""
USE_GDB=0
USE_CGROUP=1
USE_ULIMIT_V=0
QUIET=0
COMMAND=()

usage() {
    sed -n '2,25p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

while [ $# -gt 0 ]; do
    case "$1" in
        --mem)
            [ $# -ge 2 ] || { echo "error: --mem requires a value" >&2; exit 2; }
            MEM_MB="$2"; shift 2 ;;
        --high)
            [ $# -ge 2 ] || { echo "error: --high requires a value" >&2; exit 2; }
            HIGH_MB="$2"; shift 2 ;;
        --timeout)
            [ $# -ge 2 ] || { echo "error: --timeout requires a value" >&2; exit 2; }
            TIMEOUT_S="$2"; shift 2 ;;
        --outdir)
            [ $# -ge 2 ] || { echo "error: --outdir requires a value" >&2; exit 2; }
            OUTDIR="$2"; shift 2 ;;
        --gdb)
            USE_GDB=1; shift ;;
        --no-cgroup)
            USE_CGROUP=0; shift ;;
        --ulimit-v)
            USE_ULIMIT_V=1; shift ;;
        --quiet)
            QUIET=1; shift ;;
        -h|--help)
            usage; exit 0 ;;
        --)
            shift; COMMAND=("$@"); break ;;
        -*)
            echo "error: unknown option: $1" >&2
            usage >&2
            exit 2 ;;
        *)
            COMMAND=("$@"); break ;;
    esac
done

if [ ${#COMMAND[@]} -eq 0 ]; then
    echo "error: missing command" >&2
    usage >&2
    exit 2
fi

case "$MEM_MB" in ''|*[!0-9]*) echo "error: --mem must be a positive integer" >&2; exit 2 ;; esac
[ "$MEM_MB" -gt 0 ] || { echo "error: --mem must be > 0" >&2; exit 2; }
case "$TIMEOUT_S" in ''|*[!0-9]*) echo "error: --timeout must be a non-negative integer" >&2; exit 2 ;; esac
if [ -n "$HIGH_MB" ]; then
    case "$HIGH_MB" in ''|*[!0-9]*) echo "error: --high must be a positive integer" >&2; exit 2 ;; esac
fi

if [ -z "$OUTDIR" ]; then
    OUTDIR="${TMPDIR:-/tmp}/xtcw-guard.$(date +%Y%m%d-%H%M%S).$$"
fi
mkdir -p -- "$OUTDIR"
OUTDIR="$(cd -- "$OUTDIR" && pwd)"
LOG="$OUTDIR/run.log"
META="$OUTDIR/meta.txt"
GDB_LOG="$OUTDIR/backtrace.txt"
: >"$LOG"

UNIT=""
CGROUP_DIR=""
MODE="plain"
RUN=("${COMMAND[@]}")

cleanup() {
    if [ -n "$UNIT" ]; then
        systemctl --user stop "$UNIT" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

# --- pick the isolation mechanism ------------------------------------------
if [ "$USE_CGROUP" = 1 ] && command -v systemd-run >/dev/null 2>&1 \
        && command -v systemctl >/dev/null 2>&1; then
    probe_unit="xtcw-guard-probe-$$-$RANDOM.scope"
    if systemd-run --user --scope --quiet --collect --unit="$probe_unit" \
            -- true >/dev/null 2>&1; then
        MODE="cgroup"
    fi
fi

if [ "$MODE" = "cgroup" ]; then
    UNIT="xtcw-guard-$(date +%s)-$$-$RANDOM.scope"
    cg_args=(--user --scope --quiet --collect --unit="$UNIT"
             -p "MemoryMax=${MEM_MB}M" -p "MemorySwapMax=0")
    # MemoryHigh is a soft throttle. Do not set it by default: a throttled
    # cgroup can crawl instead of hitting MemoryMax, which turns a runaway
    # allocation into a timeout rather than a clean OOM kill.
    if [ -n "$HIGH_MB" ]; then
        cg_args+=(-p "MemoryHigh=${HIGH_MB}M")
    fi
    RUN=(systemd-run "${cg_args[@]}" -- "${RUN[@]}")
else
    if [ "$USE_ULIMIT_V" = 1 ]; then
        MODE="ulimit-v"
        RUN=(bash -c 'ulimit -v "$1"; shift; exec "$@"' _ "$(( MEM_MB * 1024 ))" "${RUN[@]}")
    else
        MODE="plain"
        echo "guard: warning: no cgroup memory limit available; using timeout only" >&2
        echo "guard: warning: install/enable a systemd user manager or pass --ulimit-v" >&2
        # ASan can enforce an RSS cap from inside the process without
        # reserving virtual address space (RLIMIT_AS would break ASan).
        case ":${ASAN_OPTIONS:-}:" in
            *:hard_rss_limit_mb:*) ;;
            *)
                if [ -z "${ASAN_OPTIONS:-}" ]; then
                    export ASAN_OPTIONS="hard_rss_limit_mb=${MEM_MB}"
                else
                    export ASAN_OPTIONS="$ASAN_OPTIONS:hard_rss_limit_mb=${MEM_MB}"
                fi
                ;;
        esac
    fi
fi

# --- optional gdb wrapper --------------------------------------------------
if [ "$USE_GDB" = 1 ]; then
    if ! command -v gdb >/dev/null 2>&1; then
        echo "error: --gdb requested but gdb is not installed" >&2
        exit 2
    fi
    # A signal stops the inferior; print a full backtrace for every thread.
    RUN=(gdb -q -batch \
        -ex 'set pagination off' \
        -ex 'set confirm off' \
        -ex 'set print thread-events off' \
        -ex 'handle SIGPIPE nostop noprint pass' \
        -ex run \
        -ex 'echo \n===== backtrace =====\n' \
        -ex 'thread apply all bt full' \
        --args "${RUN[@]}")
    if [ -z "${ASAN_OPTIONS:-}" ]; then
        export ASAN_OPTIONS="detect_leaks=0:abort_on_error=1"
    else
        case ":${ASAN_OPTIONS:-}:" in
            *:abort_on_error:*) ;;
            *) export ASAN_OPTIONS="$ASAN_OPTIONS:abort_on_error=1" ;;
        esac
    fi
fi

# --- wall-clock limit inside the cgroup ------------------------------------
if [ "$TIMEOUT_S" -gt 0 ]; then
    RUN=(timeout --signal=TERM --kill-after=5 "${TIMEOUT_S}s" "${RUN[@]}")
fi

# --- run and sample memory -------------------------------------------------
start_s=$(date +%s)
peak_bytes=0
oom_kills=0
run_pid=""

sample_peak() {
    local cur=0
    if [ -n "$CGROUP_DIR" ] && [ -r "$CGROUP_DIR/memory.current" ]; then
        cur=$(cat "$CGROUP_DIR/memory.current" 2>/dev/null || echo 0)
        if [ -r "$CGROUP_DIR/memory.events" ]; then
            local n
            n=$(awk '$1 == "oom_kill" { print $2 }' "$CGROUP_DIR/memory.events" 2>/dev/null || true)
            if [ -n "$n" ] && [ "$n" -gt "$oom_kills" ]; then oom_kills="$n"; fi
        fi
    elif [ -n "$run_pid" ]; then
        # Sum the RSS of the process tree (fallback mode).
        cur=$(ps -eo rss=,pid=,ppid= 2>/dev/null | awk -v root="$run_pid" '
            { rss[$2]=$1; ppid[$2]=$3 }
            END {
                sum=0; queue[1]=root; n=1
                while (n>0) {
                    p=queue[n]; n--
                    if (p in rss) sum += rss[p]
                    for (q in ppid) if (ppid[q] == p) { n++; queue[n]=q }
                }
                print sum * 1024
            }')
    fi
    [ -n "$cur" ] || cur=0
    if [ "$cur" -gt "$peak_bytes" ]; then peak_bytes="$cur"; fi
}

rc_file="$OUTDIR/.exit_code"
: >"$rc_file"
set +e
# Run the command from a subshell that records its status. The subshell
# itself survives a SIGKILL of the child, so bash's "Killed" job message goes
# to a side file instead of the guard's output and we still get the real code.
(
    "${RUN[@]}" >"$LOG" 2>&1
    printf '%s\n' "$?" >"$rc_file"
) 2>"$OUTDIR/.subshell.err" &
run_pid=$!

if [ "$MODE" = "cgroup" ]; then
    # Wait briefly for the transient scope to show up in the cgroup tree.
    for _ in {1..100}; do
        cg=$(systemctl --user show -p ControlGroup --value "$UNIT" 2>/dev/null || true)
        if [ -n "$cg" ] && [ "$cg" != "/" ]; then
            CGROUP_DIR="/sys/fs/cgroup$cg"
            [ -d "$CGROUP_DIR" ] && break
        fi
        CGROUP_DIR=""
        # A short command may finish before the scope shows up.
        kill -0 "$run_pid" 2>/dev/null || break
        sleep 0.02
    done
fi

while kill -0 "$run_pid" 2>/dev/null; do
    sample_peak
    sleep 0.2
done
wait "$run_pid" 2>/dev/null || true
rc=$(cat "$rc_file" 2>/dev/null || echo 1)
case "$rc" in ''|*[!0-9]*) rc=1 ;; esac
sample_peak
set -e

end_s=$(date +%s)
duration=$(( end_s - start_s ))
peak_mb=$(( peak_bytes / 1024 / 1024 ))

fatal_re='Segmentation fault|AddressSanitizer|LeakSanitizer|Assertion|Aborted|double free|use-after-free|stack overflow|out of memory|Cannot allocate memory|X connection to .* broken|BadAlloc|X Error of failed request|Lua CB Error|LUI Bootstrap Error|LUI load error|Failed to bootstrap|Lua syntax error|Syntax Error|Execution error|Compile-Time Lua error'
fatal=0
if grep -qiE "$fatal_re" "$LOG" 2>/dev/null; then
    fatal=1
fi

status="ok"
if [ "$rc" -ne 0 ]; then status="failed"; fi
if [ "$fatal" = 1 ]; then status="failed"; fi

{
    printf 'status=%s\n' "$status"
    printf 'exit_code=%s\n' "$rc"
    printf 'duration_s=%s\n' "$duration"
    printf 'mode=%s\n' "$MODE"
    printf 'memory_limit_mb=%s\n' "$MEM_MB"
    printf 'memory_high_mb=%s\n' "$HIGH_MB"
    printf 'peak_memory_mb=%s\n' "$peak_mb"
    printf 'oom_kills=%s\n' "$oom_kills"
    printf 'timeout_s=%s\n' "$TIMEOUT_S"
    printf 'log=%s\n' "$LOG"
    printf 'command=%q ' "${COMMAND[@]}"; printf '\n'
} >"$META"

if [ "$USE_GDB" = 1 ] && grep -q '^===== backtrace =====$' "$LOG"; then
    awk '/^===== backtrace =====$/{found=1} found' "$LOG" >"$GDB_LOG"
fi

echo "guard: status=$status exit=$rc duration=${duration}s peak=${peak_mb}MB mode=$MODE"
echo "guard: artifacts: $OUTDIR"

if [ "$status" != "ok" ]; then
    if [ "$rc" -eq 137 ] || [ "$oom_kills" -gt 0 ]; then
        echo "guard: process was killed at the memory limit (likely OOM/runaway allocation)" >&2
    fi
    if [ "$rc" -eq 124 ]; then
        echo "guard: process hit the ${TIMEOUT_S}s wall-clock limit" >&2
    fi
    if [ "$fatal" = 1 ]; then
        echo "guard: fatal pattern found in $LOG" >&2
    fi
    if [ "$QUIET" != 1 ]; then
        echo "--- last 40 log lines ($LOG) ---" >&2
        tail -40 "$LOG" >&2 || true
    fi
fi

if [ "$rc" -ne 0 ]; then
    exit "$rc"
fi
[ "$fatal" = 1 ] && exit 1
exit 0
