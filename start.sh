#!/bin/bash
# XTCW Development Startup Script
# Checks for required libraries and builds them if necessary

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== XTCW Development Startup ==="
echo ""

# Check for required libraries
LIBS=(
    "build/lib/libxtcw.a"
    "build/lib/libwcl.a"
    "build/lib/libutils.a"
    "build/lib/libretex.a"
    "build/lib/libplainc.a"
)

MISSING_LIBS=0
for lib in "${LIBS[@]}"; do
    if [ ! -f "$lib" ]; then
        echo -e "${YELLOW}Missing library: $lib${NC}"
        MISSING_LIBS=1
    fi
done

# Check for commander_runner
if [ ! -f "experimental/commander_runner" ]; then
    echo -e "${YELLOW}Missing binary: experimental/commander_runner${NC}"
    MISSING_LIBS=1
fi

# Build if necessary
if [ $MISSING_LIBS -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}Building required libraries...${NC}"
    echo ""
    
    # Run the main makefile
    if make all; then
        echo ""
        echo -e "${GREEN}Build completed successfully!${NC}"
    else
        echo ""
        echo -e "${RED}Build failed!${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}All libraries present.${NC}"
fi

# Build commander_runner if missing or older than its prerequisites
NEEDS_RUNNER_BUILD=0
if [ ! -f "experimental/commander_runner" ]; then
    NEEDS_RUNNER_BUILD=1
else
    # Rebuild when the libraries or runner sources are newer than the binary
    for prereq in build/lib/libxtcw.a build/lib/libwcl.a build/lib/libutils.a \
                  build/lib/libretex.a build/lib/libplainc.a \
                  experimental/commander_runner.c experimental/xt_bridge.c \
                  experimental/lua_bridge.c experimental/task_manager.c \
                  experimental/lua_task_bindings.c experimental/file_ops.c; do
        if [ -e "$prereq" ] && [ "$prereq" -nt "experimental/commander_runner" ]; then
            echo -e "${YELLOW}Rebuilding commander_runner: $prereq is newer${NC}"
            NEEDS_RUNNER_BUILD=1
            break
        fi
    done
fi

if [ $NEEDS_RUNNER_BUILD -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}Building commander_runner...${NC}"
    cd experimental
    make commander_runner
    cd ..
fi

echo ""
echo -e "${GREEN}Ready to run demos!${NC}"
echo ""
echo "Usage examples:"
echo "  cd experimental && ./commander_runner -Luafile ../demos/demo_wlabel_svg.lua"
echo "  cd experimental && ./commander_runner -Luafile /tmp/demo_label.lua"
echo ""
echo "Or run directly with full path:"
echo "  ./experimental/commander_runner -Luafile ./demos/demo_wlabel_svg.lua"
