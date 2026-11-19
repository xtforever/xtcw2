#!/bin/sh
# nginx2root remote check. Read-only. Intended to run as root.
# This file is piped to the target by check_nginx2root.sh, but can also be
# copied to a machine and run directly:  sh check_nginx2root_remote.sh
set -u

FORENSICS=0
[ "${1:-}" = "--forensics" ] && FORENSICS=1

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export PATH
LC_ALL=C
export LC_ALL

FIXED_VERSION=1.31.9
RESULTS=""
WORST=0

section() { printf '\n===== %s =====\n' "$*"; }
info() { printf '%s\n' "$*"; }
warn() { printf 'WARN: %s\n' "$*"; }

if [ "$(id -u 2>/dev/null || echo unknown)" != "0" ]; then
    warn "not running as root; nginx -T and process checks may be incomplete"
fi

ver_ge() {
    # exit 0 if $1 >= $2 (numeric dot-separated version)
    awk -v v1="$1" -v v2="$2" 'BEGIN {
        n=split(v1,a,"."); m=split(v2,b,".");
        for (i=1; i<=n || i<=m; i++) {
            x=(i<=n ? a[i] : 0)+0;
            y=(i<=m ? b[i] : 0)+0;
            if (x>y) exit 0;
            if (x<y) exit 1;
        }
        exit 0;
    }'
}

record_result() {
    status=$1
    label=$2
    reason=$3
    printf 'RESULT|%s|%s|%s\n' "$status" "$label" "$reason"
    RESULTS="${RESULTS}${status}|${label}|${reason}
"
    case "$status" in
        EXPOSED) [ "$WORST" -lt 3 ] && WORST=3 ;;
        UNKNOWN) [ "$WORST" -lt 2 ] && WORST=2 ;;
        WARN)    [ "$WORST" -lt 1 ] && WORST=1 ;;
    esac
    return 0
}

MODE=host
CID=
ENGINE=
NGINX_BIN=${NGINX_BIN:-}
LABEL=host

run_cmd() {
    # Bound external commands so an unhealthy container cannot hang the check.
    if command -v timeout >/dev/null 2>&1; then
        timeout 20 "$@"
    else
        "$@"
    fi
}

ngx() {
    if [ "$MODE" = host ]; then
        run_cmd "$NGINX_BIN" "$@"
    else
        run_cmd "$ENGINE" exec "$CID" nginx "$@"
    fi
}

host_package_state() {
    section "host package state"
    if command -v dpkg-query >/dev/null 2>&1; then
        dpkg-query -W -f='${Package} ${Version} ${Status}\n' \
            nginx nginx-core nginx-common nginx-full nginx-light 2>/dev/null || true
    elif command -v rpm >/dev/null 2>&1; then
        rpm -q nginx 2>/dev/null || true
    elif command -v apk >/dev/null 2>&1; then
        apk info -v 2>/dev/null | grep -E '^nginx' || true
    else
        info "no supported package manager found"
    fi
    info "note: distro packages may backport fixes without changing the nginx version."
    info "      compare the full package version with your vendor security advisory."
}

host_nginx_master_pid() {
    # Find an nginx master that is not running inside a container.
    local pids pid cg
    if command -v pgrep >/dev/null 2>&1; then
        pids=$(pgrep -x nginx 2>/dev/null || true)
    else
        pids=$(ps -eo pid=,comm= 2>/dev/null | awk '$2=="nginx"{print $1}' || true)
    fi
    for pid in $pids; do
        cg=$(cat "/proc/$pid/cgroup" 2>/dev/null || true)
        case "$cg" in
            *docker*|*kubepods*|*libpod*|*containerd*|*lxc*|*nspawn*) continue ;;
        esac
        if tr '\000' ' ' < "/proc/$pid/cmdline" 2>/dev/null | grep -q 'nginx: master'; then
            printf '%s\n' "$pid"
            return 0
        fi
    done
    return 1
}

check_instance() {
    label=$1
    LABEL=$label
    section "nginx instance: $label"

    if [ "$MODE" = container ]; then
        if ! run_cmd "$ENGINE" exec "$CID" nginx -v >/dev/null 2>&1; then
            info "no nginx binary in container"
            return 0
        fi
    elif [ -z "$NGINX_BIN" ] || [ ! -x "$NGINX_BIN" ]; then
        info "no nginx binary"
        return 0
    fi

    v_out=$(ngx -v 2>&1 || true)
    v_full=$(ngx -V 2>&1 || true)
    cfg=$(ngx -T 2>/dev/null)
    cfg_rc=$?
    cfg_err=""
    cfg_err_msg=""
    if [ "$cfg_rc" -ne 0 ]; then
        cfg_err=yes
        cfg_err_msg=$(ngx -T 2>&1 >/dev/null || true)
    fi

    info "binary: $v_out"
    version=$(printf '%s\n' "$v_out" | sed -n 's#.*nginx/\([0-9][0-9.]*\).*#\1#p' | head -n 1)
    [ -z "$version" ] && version=$(printf '%s\n' "$v_out" | sed -n 's#.*openresty/\([0-9][0-9.]*\).*#\1#p' | head -n 1)
    info "version: ${version:-unknown}"

    h2_module=absent
    h3_module=absent
    printf '%s\n' "$v_full" | grep -q -- '--with-http_v2_module' && h2_module=present
    printf '%s\n' "$v_full" | grep -q -- '--with-http_v3_module' && h3_module=present
    printf '%s\n' "$cfg" | sed 's/#.*//' | grep -qE 'load_module[[:space:]]+.*ngx_http_v2_module' && h2_module=present
    printf '%s\n' "$cfg" | sed 's/#.*//' | grep -qE 'load_module[[:space:]]+.*ngx_http_v3_module' && h3_module=present
    info "http_v2_module: $h2_module"
    info "http_v3_module: $h3_module"

    running=no
    if [ "${NGINX_RUNNING_OVERRIDE:-}" = "yes" ]; then
        running=yes
    elif [ "${NGINX_RUNNING_OVERRIDE:-}" = "no" ]; then
        running=no
    elif [ "$MODE" = host ]; then
        # Ignore nginx processes that belong to containers; they are checked
        # separately and would otherwise make a stopped host nginx look active.
        if host_nginx_master_pid >/dev/null 2>&1; then
            running=yes
        fi
    else
        if run_cmd "$ENGINE" top "$CID" 2>/dev/null | grep -q '[n]ginx'; then
            running=yes
        fi
        if [ "$running" = no ] && run_cmd "$ENGINE" exec "$CID" sh -c 'pidof nginx >/dev/null 2>&1' 2>/dev/null; then
            running=yes
        fi
    fi
    info "running: $running"

    if [ "$running" = yes ]; then
        info "note: version/config above are read from the on-disk binary/config."
        info "      restart nginx after an upgrade so workers use the new executable."
        if [ "$MODE" = host ]; then
            master_pid=$(host_nginx_master_pid 2>/dev/null || true)
            if [ -n "$master_pid" ]; then
                info "running master pid: $master_pid"
                exe=$(readlink "/proc/$master_pid/exe" 2>/dev/null || true)
                [ -n "$exe" ] && info "running executable: $exe"
            fi
        elif [ "$MODE" = container ]; then
            exe=$(run_cmd "$ENGINE" exec "$CID" sh -c 'for p in $(pidof nginx); do x=$(readlink /proc/$p/exe 2>/dev/null) && { echo "$x"; break; }; done' 2>/dev/null || true)
            [ -n "$exe" ] && info "running executable: $exe"
        fi
    fi

    if [ "$MODE" = host ]; then
        info "path: $NGINX_BIN"
    fi

    if [ "$cfg_err" = yes ]; then
        warn "nginx -T failed; configuration checks may be incomplete"
        [ -n "$cfg_err_msg" ] && printf '%s\n' "$cfg_err_msg"
        printf '%s\n' "$cfg" | head -n 20
    fi

    clean=$(printf '%s\n' "$cfg" | sed 's/#.*//')

    h2_enabled=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*listen[[:space:]].*[[:space:]]http2([[:space:]]|;)|^[[:space:]]*http2[[:space:]]+on[[:space:]]*;' || true)
    h3_quic=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*listen[[:space:]].*[[:space:]]quic([[:space:]]|;)' || true)
    h3_on=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*http3[[:space:]]+on[[:space:]]*;' || true)
    resolver_lines=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*resolver[[:space:]]' || true)
    mirror_lines=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*mirror[[:space:]]' || true)
    proxy_var_lines=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*proxy_pass[[:space:]]+[^;]*\$' || true)
    mail_block=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*mail[[:space:]]*\{' || true)
    mail_proxy_on=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*proxy[[:space:]]+on[[:space:]]*;' || true)
    mail_auth=$(printf '%s\n' "$clean" | grep -nE '^[[:space:]]*auth_http[[:space:]]' || true)
    mail_proxy=no
    [ -n "$mail_block" ] && [ -n "$mail_proxy_on" ] && [ -n "$mail_auth" ] && mail_proxy=yes

    section "configuration signals"
    if [ -n "$h2_enabled" ]; then
        info "HTTP/2 ENABLED:"
        printf '%s\n' "$h2_enabled"
    else
        info "HTTP/2: not enabled"
    fi
    if [ -n "$h3_quic" ]; then
        info "HTTP/3/QUIC ENABLED (QUIC listener):"
        printf '%s\n' "$h3_quic"
    else
        info "HTTP/3/QUIC: no QUIC listener"
    fi
    [ -n "$h3_on" ] && { info "http3 on:"; printf '%s\n' "$h3_on"; }
    if [ -n "$resolver_lines" ]; then
        info "resolver:"
        printf '%s\n' "$resolver_lines"
    else
        info "resolver: none"
    fi
    if [ -n "$mirror_lines" ]; then
        info "mirror:"
        printf '%s\n' "$mirror_lines"
    else
        info "mirror: none"
    fi
    if [ -n "$proxy_var_lines" ]; then
        info "request-derived proxy_pass:"
        printf '%s\n' "$proxy_var_lines"
    else
        info "request-derived proxy_pass: none"
    fi
    info "mail proxy (proxy on + auth_http): $mail_proxy"

    tls_ports=$(printf '%s\n' "$clean" | awk '
        /^[[:space:]]*listen[[:space:]]/ {
            line=$0;
            sub(/#.*/,"",line);
            n=split(line,t,"[ \t;]+");
            ssl=0; quic=0; port="";
            for (i=1;i<=n;i++) {
                if (t[i]=="ssl") ssl=1;
                if (t[i]=="quic") quic=1;
                if (t[i] ~ /^[0-9]+$/) port=t[i];
                else if (t[i] ~ /:[0-9]+$/) { p=t[i]; sub(/.*:/,"",p); port=p }
            }
            if (ssl && !quic && port!="") print port;
        }' | sort -n -u)

    sni=$(printf '%s\n' "$clean" | sed -n 's/^[[:space:]]*server_name[[:space:]][[:space:]]*\([^;]*\);.*/\1/p' \
        | tr ' ' '\n' | grep -vE '^(_|""|~|\*|\.|)$' | head -n 1 || true)

    if [ -n "$tls_ports" ]; then
        section "TLS listeners / ALPN (loopback)"
        for port in $tls_ports; do
            info "TLS port $port${sni:+ (SNI: $sni)}"
            alpn=unknown
            if command -v openssl >/dev/null 2>&1; then
                if [ -n "$sni" ]; then
                    set -- -servername "$sni"
                else
                    set --
                fi
                if command -v timeout >/dev/null 2>&1; then
                    out=$(timeout 8 openssl s_client -connect "127.0.0.1:$port" "$@" -alpn h2 </dev/null 2>&1 || true)
                else
                    out=$(openssl s_client -connect "127.0.0.1:$port" "$@" -alpn h2 </dev/null 2>&1 || true)
                fi
                if printf '%s' "$out" | grep -q 'ALPN protocol: h2'; then
                    alpn=h2
                elif printf '%s' "$out" | grep -q 'No ALPN negotiated'; then
                    alpn=no-h2
                elif printf '%s' "$out" | grep -qiE 'Connection refused|connect:errno|handshake failure'; then
                    alpn=failed
                fi
            else
                alpn=openssl-missing
            fi
            info "ALPN h2: $alpn"
            if [ "$alpn" = failed ]; then
                info "note: loopback connect failed; the listener may be bound to a specific address or IPv6-only"
            fi
            printf 'TLS_PORT|%s|%s|%s\n' "$label" "$port" "$sni"
            printf 'ALPN|%s|%s|%s\n' "$label" "$port" "$alpn"
        done
    else
        info "no TLS listener found in config"
    fi

    status=
    reason=
    if [ -n "$version" ] && ver_ge "$version" "$FIXED_VERSION"; then
        status=OK
        reason="version $version >= $FIXED_VERSION (verify build/backport for distro packages)"
    elif [ "$cfg_err" = yes ]; then
        status=UNKNOWN
        reason="nginx -T failed; configuration could not be fully verified"
    elif [ -z "$version" ]; then
        status=UNKNOWN
        reason="could not determine nginx version"
    elif [ "$mail_proxy" = yes ] && ver_ge "$version" "0.5.15"; then
        if [ "$running" = yes ]; then
            status=EXPOSED
            reason="mail proxy with proxy on + auth_http (CVE-2026-113425)"
        else
            status=WARN
            reason="mail proxy with proxy on + auth_http configured, but instance is not running (CVE-2026-113425)"
        fi
    elif ! ver_ge "$version" "1.9.5"; then
        status=OK
        reason="version $version < 1.9.5 and no affected mail proxy config"
    elif [ -n "$resolver_lines" ] && [ -n "$mirror_lines" ] && [ -n "$proxy_var_lines" ] && ver_ge "$version" "1.13.4"; then
        if [ "$running" = yes ]; then
            status=EXPOSED
            reason="resolver + request-derived proxy_pass + mirror (CVE-2026-123634)"
        else
            status=WARN
            reason="resolver + request-derived proxy_pass + mirror configured, but instance is not running (CVE-2026-123634)"
        fi
    elif [ -n "$h2_enabled" ] && [ "$h2_module" = present ]; then
        if [ "$running" = yes ]; then
            status=EXPOSED
            reason="HTTP/2 enabled (CVE-2026-123007/123633)"
        else
            status=WARN
            reason="HTTP/2 configured, but instance is not running (CVE-2026-123007/123633)"
        fi
    elif [ -n "$h3_quic" ] && [ "$h3_module" = present ] && ver_ge "$version" "1.25.0"; then
        if [ "$running" = yes ]; then
            status=EXPOSED
            reason="HTTP/3/QUIC listener enabled (CVE-2026-121810)"
        else
            status=WARN
            reason="HTTP/3/QUIC listener configured, but instance is not running (CVE-2026-121810)"
        fi
    elif [ -n "$h2_enabled" ] && [ "$h2_module" = absent ]; then
        status=UNKNOWN
        reason="HTTP/2 configured but module not reported by nginx -V"
    elif [ -n "$h3_quic" ] && [ "$h3_module" = absent ]; then
        status=UNKNOWN
        reason="HTTP/3 configured but module not reported by nginx -V"
    else
        status=WARN
        reason="affected version, no known attack path enabled; patch recommended"
    fi

    record_result "$status" "$label" "$reason"
    info "verdict: $status - $reason"
}

check_engine() {
    engine=$1
    command -v "$engine" >/dev/null 2>&1 || return 0

    section "$engine containers"
    list=$(run_cmd "$engine" ps --format '{{.ID}}|{{.Names}}|{{.Image}}' 2>/dev/null || true)
    if [ -z "$list" ]; then
        info "no running containers, or no permission"
        return 0
    fi

    while IFS='|' read -r id name image; do
        [ -z "$id" ] && continue
        info "container: $name ($image) id=$id"
        if run_cmd "$engine" exec "$id" nginx -v >/dev/null 2>&1; then
            MODE=container
            CID=$id
            ENGINE=$engine
            check_instance "$engine:$name"
        else
            info "no nginx in container"
        fi
    done <<EOF
$list
EOF

    MODE=host
    CID=
    ENGINE=
}

section "host nginx"
if [ -n "$NGINX_BIN" ]; then
    info "using NGINX_BIN override: $NGINX_BIN"
else
    NGINX_BIN=$(command -v nginx 2>/dev/null || true)
    if [ -z "$NGINX_BIN" ]; then
        for p in /usr/sbin/nginx /usr/local/nginx/sbin/nginx /opt/nginx/sbin/nginx; do
            if [ -x "$p" ]; then NGINX_BIN=$p; break; fi
        done
    fi
fi

host_package_state

if [ -n "$NGINX_BIN" ] && [ -x "$NGINX_BIN" ]; then
    MODE=host
    check_instance host
else
    info "no host nginx binary found"
    if ps -eo comm= 2>/dev/null | grep -qx 'nginx'; then
        warn "nginx processes are running but no host binary was found; check the container runtime"
    fi
fi

if [ "${SKIP_CONTAINERS:-0}" != "1" ]; then
    check_engine docker
    check_engine podman
else
    section "containers"
    info "skipped (SKIP_CONTAINERS=1)"
fi

section "UDP/443 listeners"
if command -v ss >/dev/null 2>&1; then
    udp443=$(ss -lunp 'sport = :443' 2>/dev/null || true)
    if printf '%s\n' "$udp443" | grep -q ':443'; then
        printf '%s\n' "$udp443"
    else
        info "none"
    fi
else
    info "ss not available"
fi

section "kernel / SCTP"
kernel=$(uname -r 2>/dev/null || true)
info "kernel: ${kernel:-unknown}"
if command -v lsmod >/dev/null 2>&1; then
    if lsmod 2>/dev/null | awk '$1=="sctp"{found=1} END{exit !found}'; then
        info "sctp module: loaded"
    else
        info "sctp module: not loaded"
    fi
fi
if command -v modinfo >/dev/null 2>&1; then
    mod=$(modinfo -n sctp 2>/dev/null || true)
    [ -n "$mod" ] && info "sctp module available: $mod"
fi
sctp_rule=no
for f in /etc/modprobe.d/*.conf; do
    [ -f "$f" ] || continue
    if grep -qE '^[[:space:]]*install[[:space:]]+sctp[[:space:]]+/bin/false' "$f" 2>/dev/null; then
        sctp_rule=yes
    fi
done
info "modprobe mitigation (install sctp /bin/false): $sctp_rule"
info "note: the SCTP LPE is only reachable after a successful nginx worker RCE."
info "      blocking SCTP is defense-in-depth if you do not use it."

if [ "$FORENSICS" = 1 ]; then
    section "forensics (read-only)"
    info "nginx error logs:"
    info "note: container nginx logs are not included; use docker/podman logs for those."
    found_log=no
    for f in /var/log/nginx/error.log /var/log/nginx/*error*.log; do
        [ -f "$f" ] || continue
        found_log=yes
        info "-- $f"
        grep -Ei 'worker process.*exited|signal 11|segfault|core dumped' "$f" 2>/dev/null | tail -n 20 || true
    done
    [ "$found_log" = no ] && info "no nginx error log found"

    if command -v coredumpctl >/dev/null 2>&1; then
        info "coredumpctl list nginx:"
        coredumpctl list nginx 2>/dev/null | tail -n 20 || true
    fi
    if command -v auditctl >/dev/null 2>&1; then
        info "audit rule nginx_worker_exec:"
        auditctl -l 2>/dev/null | grep nginx_worker_exec || info "not installed"
    fi
    if command -v ausearch >/dev/null 2>&1; then
        info "recent nginx worker execve events:"
        ausearch -k nginx_worker_exec -ts recent -i 2>/dev/null | tail -n 50 || true
    fi
    if command -v ss >/dev/null 2>&1; then
        info "established connections on :443 (top 20):"
        ss -tinp state established '( sport = :443 )' 2>/dev/null | head -n 20 || true
    fi
fi

section "summary"
if [ -z "$RESULTS" ]; then
    info "no nginx instances found"
else
    printf '%s' "$RESULTS" | while IFS='|' read -r status label reason; do
        [ -z "$status" ] && continue
        printf '%-8s %-30s %s\n' "$status" "$label" "$reason"
    done
fi

case "$WORST" in
    3)
        info "overall: EXPOSED - immediate action required"
        exit 2
        ;;
    2)
        info "overall: INCONCLUSIVE - manual review required"
        exit 3
        ;;
    1)
        info "overall: VULNERABLE CODE PRESENT - no known attack path enabled; patch recommended"
        exit 4
        ;;
    *)
        info "overall: OK - no affected/exposed nginx found"
        exit 0
        ;;
esac
