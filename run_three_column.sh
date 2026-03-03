#!/bin/bash
# Start script for LUI Three Column Demo

# Ensure we are in the project root
ROOT_DIR=$(dirname "$(readlink -f "$0")")
cd "$ROOT_DIR"

# Set up the search path for Lua modules
export LUA_PATH="./lui/?.lua;;"

# Set X resources and launch luarunner
# We use the existing hello.ad for basic styles
XENVIRONMENT=lui/hello.ad ./clean-build/build/bin/luarunner -Luafile lui/three_column_demo.lua -geometry 650x550
