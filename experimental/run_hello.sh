#!/bin/bash
# Start script for Hello LUI Application

# Ensure we are in the project root
ROOT_DIR=$(dirname "$(readlink -f "$0")")
cd "$ROOT_DIR"

# Set up the search path for Lua modules
export LUA_PATH="./lui/?.lua;;"

# Set X resources and launch luarunner
XENVIRONMENT=lui/hello.ad ./build/bin/luarunner -Luafile lui/hello_lui.lua -geometry 1200x500
