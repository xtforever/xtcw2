#!/usr/local/bin/lua
print("hallo")

filename='luarunner.adp'
outfilead='luarunner.ad'
outfilelua='ex.lua'

local ofad=io.open( outfilead, "w" )
local oflua=io.open( outfilelua, "w" )
local rfile = io.open( filename, "r")
local start = 0

for line in rfile:lines() do
    if line:match("%%LUA") then
       start=1
    elseif start == 1 and line:match("%%END") then
       start=0
    elseif start==1 then
       oflua:write( line .. "\n" )
    elseif start==0 then
       ofad:write( line .. "\n" )
    end
end