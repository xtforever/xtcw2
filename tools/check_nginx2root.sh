#!/usr/bin/env bash
# check_nginx2root.sh - read-only nginx2root exposure check for local machines.
#
# The actual checks live in check_nginx2root_remote.sh next to this file. This
# controller pipes that script to each target over SSH (or runs it locally with
# --local). Nothing is changed on the targets.
#
# CVEs covered:
#   CVE-2026-123007  HTTP/2 send-window overflow (RCE)
#   CVE-2026-123634  resolver + request-derived proxy_pass + mirror (UAF)
#   CVE-2026-123633  HTTP/2 + upstream (UAF)
#   CVE-2026-121810  HTTP/3/QUIC + upstream (UAF)
#   CVE-2026-113425  mail proxy with proxy on + auth_http (UAF)
# plus the Linux SCTP LPE that can be chained after nginx worker RCE.
#
# Usage:
#   sudo ./tools/check_nginx2root.sh --local
#   ./tools/check_nginx2root.sh root@host1 root@host2
#   ./tools/check_nginx2root.sh -u root -p 22 -i ~/.ssh/id_ed25519 host1 host2
#   ./tools/check_nginx2root.sh --external -u root host1
#   ./tools/check_nginx2root.sh --forensics root@host1
#
# Remote requirements: root SSH (key-based by default) and /bin/sh.
#
# Exit codes: 0 OK, 2 EXPOSED, 3 INCONCLUSIVE, 4 VULNERABLE CODE PRESENT,
#             1 usage error. SSH failures are reported as INCONCLUSIVE.

set -uo pipefail

SCRIPT_PATH=$(readlink -f -- "${BASH_SOURCE[0]}" 2>/dev/null || printf '%s' "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd -- "$(dirname -- "$SCRIPT_PATH")" && pwd)
REMOTE_SCRIPT="$SCRIPT_DIR/check_nginx2root_remote.sh"

SSH_USER=${SSH_USER:-root}
SSH_PORT=
SSH_KEY=
EXTERNAL=0
FORENSICS=0
LOCAL=0
ASK_PASS=0
SSH_EXTRA_OPTS=()
HOSTS=()

usage() {
    cat <<'EOF'
Read-only nginx2root exposure check.

Usage:
  check_nginx2root.sh [options] host [host ...]
  check_nginx2root.sh --local

Options:
  -u, --user USER       SSH user (default: root)
  -p, --port PORT       SSH port
  -i, --identity FILE   SSH private key
  -o, --ssh-option OPT  extra ssh -o option (repeatable)
      --ask-pass        allow SSH password prompts (disables BatchMode)
  -e, --external        also test ALPN h2 from this machine against the target
                        (connects to the host part of the target)
  -f, --forensics       include read-only compromise indicators
  -l, --local           check the machine running this script (no SSH)
  -h, --help            show this help

Hosts may be given as "host" or "user@host". If "user@host" is used, -u is
ignored for that host. Remote checks require root.

The remote check script (check_nginx2root_remote.sh) must be in the same
directory as this script.

SSH uses StrictHostKeyChecking=accept-new by default. Pre-populate known_hosts
or pass -o StrictHostKeyChecking=yes if you require strict checking.

Exit codes:
  0  no affected/exposed nginx found
  2  at least one nginx is vulnerable and exposed
  3  at least one result is inconclusive (including SSH failures)
  4  vulnerable code present, but no known attack path enabled
  1  usage error
EOF
}

while (($#)); do
    case "$1" in
        -u|--user)
            [ $# -ge 2 ] || { echo "missing argument for $1" >&2; exit 1; }
            SSH_USER=$2; shift 2 ;;
        -p|--port)
            [ $# -ge 2 ] || { echo "missing argument for $1" >&2; exit 1; }
            SSH_PORT=$2; shift 2 ;;
        -i|--identity)
            [ $# -ge 2 ] || { echo "missing argument for $1" >&2; exit 1; }
            SSH_KEY=$2; shift 2 ;;
        -o|--ssh-option)
            [ $# -ge 2 ] || { echo "missing argument for $1" >&2; exit 1; }
            SSH_EXTRA_OPTS+=("$2"); shift 2 ;;
        --ask-pass) ASK_PASS=1; shift ;;
        -e|--external) EXTERNAL=1; shift ;;
        -f|--forensics) FORENSICS=1; shift ;;
        -l|--local) LOCAL=1; shift ;;
        -h|--help) usage; exit 0 ;;
        --) shift; while (($#)); do HOSTS+=("$1"); shift; done ;;
        -*) echo "unknown option: $1" >&2; usage >&2; exit 1 ;;
        *) HOSTS+=("$1"); shift ;;
    esac
done

if [ ! -f "$REMOTE_SCRIPT" ]; then
    echo "error: remote check script not found: $REMOTE_SCRIPT" >&2
    exit 1
fi

if ((LOCAL == 1)); then
    if ((${#HOSTS[@]} > 0)); then
        echo "note: --local ignores host arguments; use --external with a host to test from here" >&2
    fi
    HOSTS=("localhost")
elif ((${#HOSTS[@]} == 0)); then
    usage >&2
    exit 1
fi

external_alpn() {
    local host=$1 port=$2 sni=$3
    local connect out
    case "$host" in
        *:*) connect="[$host]:$port" ;;
        *) connect="$host:$port" ;;
    esac

    if ! command -v openssl >/dev/null 2>&1; then
        printf 'openssl-missing'
        return 0
    fi

    local -a args=(-connect "$connect")
    [ -n "$sni" ] && args+=(-servername "$sni")

    if command -v timeout >/dev/null 2>&1; then
        out=$(timeout 8 openssl s_client "${args[@]}" -alpn h2 </dev/null 2>&1 || true)
    else
        out=$(openssl s_client "${args[@]}" -alpn h2 </dev/null 2>&1 || true)
    fi

    if printf '%s' "$out" | grep -q 'ALPN protocol: h2'; then
        printf 'h2'
    elif printf '%s' "$out" | grep -q 'No ALPN negotiated'; then
        printf 'no-h2'
    elif printf '%s' "$out" | grep -qiE 'Connection refused|connect:errno|handshake failure'; then
        printf 'failed'
    else
        printf 'unknown'
    fi
}

overall=0
declare -a SUMMARY=()

run_target() {
    local raw_host=$1
    local target host_only out rc remote_flags severity

    if ((LOCAL == 1)); then
        target=localhost
        host_only=127.0.0.1
    else
        target=$raw_host
        case "$target" in
            *@*) ;;
            *) target="${SSH_USER}@${raw_host}" ;;
        esac
        host_only=${raw_host#*@}
    fi

    remote_flags=""
    ((FORENSICS == 1)) && remote_flags="--forensics"

    printf '\n############################################################\n'
    printf '# Target: %s\n' "$target"
    printf '############################################################\n'

    if ((LOCAL == 1)); then
        out=$(sh "$REMOTE_SCRIPT" $remote_flags)
        rc=$?
    else
        local -a ssh_cmd=(ssh)
        for opt in "${SSH_EXTRA_OPTS[@]}"; do
            ssh_cmd+=(-o "$opt")
        done
        ssh_cmd+=(-o ConnectTimeout=10 -o StrictHostKeyChecking=accept-new)
        ((ASK_PASS == 0)) && ssh_cmd+=(-o BatchMode=yes)
        [ -n "$SSH_PORT" ] && ssh_cmd+=(-p "$SSH_PORT")
        [ -n "$SSH_KEY" ] && ssh_cmd+=(-i "$SSH_KEY")

        out=$("${ssh_cmd[@]}" "$target" "sh -s -- $remote_flags" < "$REMOTE_SCRIPT")
        rc=$?
    fi

    printf '%s\n' "$out"

    if ((EXTERNAL == 1)); then
        printf '\n===== external ALPN (from this machine) =====\n'
        printf '%s\n' "$out" | while IFS='|' read -r tag label port sni; do
            [ "$tag" = "TLS_PORT" ] || continue
            local res
            res=$(external_alpn "$host_only" "$port" "$sni")
            printf 'EXTERNAL %-24s %s:%s -> %s\n' "$label" "$host_only" "$port" "$res"
        done
    fi

    case "$rc" in
        0) severity=0 ;;
        4) severity=1 ;;
        3) severity=2 ;;
        2) severity=3 ;;
        *)
            printf 'ERROR: remote check failed (ssh/exit code %s)\n' "$rc" >&2
            severity=2
            ;;
    esac

    SUMMARY+=("$target rc=$rc severity=$severity")
    ((severity > overall)) && overall=$severity
}

for h in "${HOSTS[@]}"; do
    run_target "$h"
done

printf '\n############################################################\n'
printf '# Overall\n'
printf '############################################################\n'
for line in "${SUMMARY[@]}"; do
    printf '%s\n' "$line"
done

case "$overall" in
    3)
        printf 'Overall: EXPOSED - immediate action required\n'
        exit 2
        ;;
    2)
        printf 'Overall: INCONCLUSIVE - manual review required\n'
        exit 3
        ;;
    1)
        printf 'Overall: VULNERABLE CODE PRESENT - no known attack path enabled; patch recommended\n'
        exit 4
        ;;
    *)
        printf 'Overall: OK - no affected/exposed nginx found\n'
        exit 0
        ;;
esac
