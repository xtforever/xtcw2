#!/usr/bin/env bash
# cookbook/scripts/build.sh — build xtcw2 + the cookbook app in one mode.
#
#   ./build.sh debug        # -g -O0 -DMLS_DEBUG -fsanitize=address
#   ./build.sh production   # -O3, no MLS_DEBUG, no AddressSanitizer
#
# Always builds from the repository root so that the toolkit libraries and
# commander_runner agree on MLS_DEBUG (mixing them crashes at runtime). A full
# `make distclean` runs first, because the Makefiles do not track flag changes.
set -euo pipefail

MODE="${1:-}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

case "$MODE" in
    debug)      MAKE_MODE=() ;;
    production) MAKE_MODE=(debug_enable=0) ;;
    *)
        echo "usage: $0 <debug|production>" >&2
        exit 2
        ;;
esac

cd "$ROOT"
echo "[build] make distclean"
make distclean
echo "[build] make commander ${MAKE_MODE[*]:-}"
make commander "${MAKE_MODE[@]}"
echo "[build] ok: $ROOT/commander/commander_runner ($MODE)"
