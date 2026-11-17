#!/usr/bin/env bash
# Install build and test dependencies for xtcw2 on Debian/Ubuntu.
#
# Usage:
#   tools/install_dev_deps.sh
#   tools/install_dev_deps.sh --no-submodules
#   tools/install_dev_deps.sh --check
#   tools/install_dev_deps.sh --pip
#
# The script uses sudo when not run as root. It installs the packages needed by
# `make all`, the tests/ scripts, commander/run_demo_tests.sh and the cookbook
# smoke tests (Xvfb, screenshot tooling, Python PIL/numpy).
#
# Note: experimental/AdmPnl additionally needs libmysqlclient-dev; it is not
# part of the default build and is not installed here.

set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
cd "$ROOT"

NO_SUBMODULES=0
CHECK_ONLY=0
INSTALL_PIP=0
for arg in "$@"; do
    case "$arg" in
        --no-submodules) NO_SUBMODULES=1 ;;
        --check) CHECK_ONLY=1 ;;
        --pip) INSTALL_PIP=1 ;;
        -h|--help)
            sed -n '2,15p' "$0"
            exit 0
            ;;
        *)
            echo "error: unknown argument: $arg" >&2
            exit 2
            ;;
    esac
done

PACKAGES=(
    # compiler / build system
    build-essential
    pkg-config
    bison
    flex
    swig
    git
    gdb
    # X11 / Xt / Xaw
    libx11-dev
    libxt-dev
    libxaw7-dev
    libxmu-dev
    libxft-dev
    libfontconfig1-dev
    libxrender-dev
    libxpm-dev
    libxext-dev
    libxtst-dev
    # Cairo backend
    libcairo2-dev
    # Lua 5.3 + LPeg parser
    liblua5.3-dev
    lua5.3
    lua-lpeg
    # headless tests / screenshots
    xvfb
    x11-utils
    xdotool
    scrot
    python3
    python3-pil
    python3-numpy
    # fonts used by the default theme
    fonts-dejavu-core
    # setarch for the cookbook docker-compose debug profile
    util-linux
)

check_deps() {
    local missing=()
    local cmd pkg status py python_missing=0

    for cmd in gcc make pkg-config bison flex swig lua5.3 Xvfb xvfb-run \
               xwininfo scrot python3; do
        command -v "$cmd" >/dev/null 2>&1 || missing+=("command: $cmd")
    done

    py="$(command -v python3 2>/dev/null || true)"
    if [ -z "$py" ]; then
        missing+=("command: python3")
    else
        for pkg in PIL numpy; do
            if ! "$py" -c "import $pkg" >/dev/null 2>&1; then
                missing+=("python module: $pkg (active python3: $py)")
                python_missing=1
            fi
        done
    fi

    if command -v dpkg-query >/dev/null 2>&1; then
        for pkg in "${PACKAGES[@]}"; do
            status="$(dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null || true)"
            [ "$status" = "install ok installed" ] || missing+=("package: $pkg")
        done
    fi

    if [ -f "$ROOT/.gitmodules" ] && [ ! -f "$ROOT/LuaRunner/nanosvg/src/nanosvg.h" ]; then
        missing+=("submodule: LuaRunner/nanosvg")
    fi

    if [ "${#missing[@]}" -ne 0 ]; then
        echo "Missing dependencies:" >&2
        printf '  - %s\n' "${missing[@]}" >&2
        if [ "$python_missing" -eq 1 ]; then
            echo >&2
            echo "note: python3-pil/python3-numpy from apt target /usr/bin/python3." >&2
            echo "      Your active python3 is: $py" >&2
            echo "      For a pyenv/conda interpreter, install them there:" >&2
            echo "        $py -m pip install --user pillow numpy" >&2
        fi
        return 1
    fi

    echo "All checked dependencies are present (python3: $py)."
}
if [ "$CHECK_ONLY" -eq 1 ]; then
    check_deps
    exit 0
fi

if ! command -v apt-get >/dev/null 2>&1; then
    cat >&2 <<'MSG'
error: this script supports Debian/Ubuntu (apt-get) only.

Install the equivalent packages with your distribution's package manager:
  build: gcc make pkg-config bison flex swig git gdb
  X11:   libx11-dev libxt-dev libxaw7-dev libxmu-dev libxft-dev
         libfontconfig1-dev libxrender-dev libxpm-dev libxext-dev libxtst-dev
  Cairo: libcairo2-dev
  Lua:   liblua5.3-dev lua5.3 lua-lpeg
  tests: xvfb x11-utils xdotool scrot python3 python3-pil python3-numpy
  fonts: fonts-dejavu-core
MSG
    exit 1
fi

if [ "$(id -u)" -eq 0 ]; then
    SUDO=()
else
    if ! command -v sudo >/dev/null 2>&1; then
        echo "error: sudo is required when not running as root." >&2
        exit 1
    fi
    SUDO=(sudo)
fi

echo "==> Installing xtcw2 build/test dependencies (apt)"
"${SUDO[@]}" apt-get update
DEBIAN_FRONTEND=noninteractive "${SUDO[@]}" apt-get install -y --no-install-recommends "${PACKAGES[@]}"

if [ "$NO_SUBMODULES" -eq 0 ] && [ -f "$ROOT/.gitmodules" ]; then
    echo "==> Initialising git submodules"
    git -C "$ROOT" submodule update --init --recursive
fi

if [ "$INSTALL_PIP" -eq 1 ]; then
    py="$(command -v python3 2>/dev/null || true)"
    if [ -n "$py" ] && "$py" -m pip --version >/dev/null 2>&1; then
        echo "==> Installing Python test modules for $py"
        if ! "$py" -m pip install --user pillow numpy; then
            echo "warning: pip install failed; install pillow/numpy manually for $py." >&2
        fi
    else
        echo "warning: cannot run '$py -m pip'; install pillow/numpy manually." >&2
    fi
fi

echo "==> Checking required tools"
check_deps

echo
echo "Dependencies installed."
echo "Next steps:"
echo "  make all"
echo "  tests/test_run_widget.sh"
echo "  (cd commander && ./run_demo_tests.sh)"
echo "  (cd cookbook && make test)"
