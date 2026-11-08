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

# Always bring the build up to date.  The wbuild-generated sources and the
# static archives have no automatic dependency tracking, so editing a
# .widget does not by itself rebuild libxtcw.a -- and a stale libxtcw.a means
# a stale commander_runner (which can crash in ways already fixed in the
# sources).  An incremental `make all` regenerates changed widgets, rebuilds
# the libraries and relinks the runner.
#
# Set START_SKIP_BUILD=1 to skip this (used by tests / when already built).
if [ "${START_SKIP_BUILD:-0}" = "1" ]; then
    echo -e "${GREEN}Skipping build (START_SKIP_BUILD=1).${NC}"
else
    echo -e "${YELLOW}Building/updating (make all)...${NC}"
    if make all; then
        echo -e "${GREEN}Build up to date.${NC}"
    else
        echo ""
        echo -e "${RED}Build failed!${NC}"
        exit 1
    fi
fi

echo ""
echo -e "${GREEN}Ready to run demos!${NC}"
echo ""
echo "Usage examples:"
echo "  cd commander && ./commander_runner -Luafile ../demos/demo_wlabel_svg.lua"
echo "  cd commander && ./commander_runner -Luafile /tmp/demo_label.lua"
echo ""
echo "Or run directly with full path:"
echo "  ./commander/commander_runner -Luafile ./demos/demo_wlabel_svg.lua"
