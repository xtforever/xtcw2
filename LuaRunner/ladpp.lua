#!/usr/bin/env lua

-- FILE: ladpp.lua
-- lua application-defaults preprozessor
--
-- read a adp-file from stdin, prozess the embedded lua code and output the result to stdout

for line in io:lines() do
    if start==0 and line:match("<%?LUA") then
        lines=""
	start=2
    elseif start == 2 and line:match("^%?>$") then
       start=0
       f = load(lines)
       if f then f() end
       lines=""
    elseif start==2 then
       lines = lines .. line .. "\n"
    elseif start==0 then
       io:write( line .. "\n" )
    end	
end
