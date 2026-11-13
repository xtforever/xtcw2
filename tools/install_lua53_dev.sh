#!/bin/bash
# Install the Lua 5.3 development files into build/lua5.3 without sudo.
#
# The runtime (liblua5.3-0) is usually installed, but the -dev package may be
# missing. This script downloads the Ubuntu/Debian package, extracts it into the
# build tree, fixes the pkg-config file so it is relocatable, and points the
# linker at the installed runtime library.
#
# Usage:
#   tools/install_lua53_dev.sh
#   make commander
#
# The script is intentionally local: it does not touch /usr and needs no root.

set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
PREFIX="$ROOT/build/lua5.3"
MULTIARCH="$(gcc -dumpmachine)"
LIBDIR="$PREFIX/usr/lib/$MULTIARCH"
PCDIR="$LIBDIR/pkgconfig"

if ! command -v apt-get >/dev/null 2>&1 || ! command -v dpkg-deb >/dev/null 2>&1; then
    echo "error: apt-get and dpkg-deb are required." >&2
    exit 1
fi

mkdir -p "$PREFIX"
cd "$PREFIX"

if ! ls liblua5.3-dev_*.deb >/dev/null 2>&1; then
    echo "Downloading liblua5.3-dev..."
    apt-get download liblua5.3-dev
fi

for deb in liblua5.3-dev_*.deb; do
    [ -f "$deb" ] || continue
    echo "Extracting $deb..."
    dpkg-deb -x "$deb" "$PREFIX"
done

# The package's pkg-config file hard-codes prefix=/usr. Make it relocatable so
# pkg-config reports the headers and library in this build prefix.
cat > "$PCDIR/lua5.3.pc" <<EOF
prefix=\${pcfiledir}/../../..
exec_prefix=\${prefix}
libdir=\${prefix}/lib/$MULTIARCH
includedir=\${prefix}/include

Name: Lua
Description: Lua language engine
Version: 5.3.6
Libs: -L\${libdir} -llua5.3
Libs.private: -lm -ldl
Cflags: -I\${includedir}/lua5.3 -I\${includedir}/$MULTIARCH
EOF

# The -dev package only ships the unversioned symlink; the actual runtime
# library comes from liblua5.3-0. Point the local symlink at the installed one.
RUNTIME=""
for candidate in \
    "/usr/lib/$MULTIARCH/liblua5.3.so.0" \
    "/lib/$MULTIARCH/liblua5.3.so.0"; do
    if [ -e "$candidate" ]; then
        RUNTIME="$candidate"
        break
    fi
done

if [ -z "$RUNTIME" ]; then
    echo "error: liblua5.3.so.0 not found; install the liblua5.3-0 runtime." >&2
    exit 1
fi

ln -sf "$RUNTIME" "$LIBDIR/liblua5.3.so"

echo "Lua 5.3 development files installed in:"
echo "  $PREFIX"
echo ""
echo "Verify with:"
echo "  PKG_CONFIG_PATH=$PCDIR pkg-config --cflags --libs lua5.3"
echo "  make commander"
