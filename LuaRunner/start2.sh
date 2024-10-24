#!/bin/bash
set -x

p=edit-test

make
./ladpp.lua < $p.adp >pre.adp

./adpconv.lua pre.adp pre.ad pre.lua 

./luarunner -TraceLevel 1 -Luafile pre.lua -xrm "*WclResFiles:pre.ad"
