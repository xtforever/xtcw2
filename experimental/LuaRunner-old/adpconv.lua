#!/usr/bin/env lua

filename=arg[1] or ""
outfilead=arg[2] or ""
outfilelua=arg[3] or ""
if not filename:match( ".adp$" ) then
   print("expecting file.adp as first arg")
   os.exit(1)
end
if not outfilead:match( ".ad.?$" ) then
   print("expecting file.ad as 2nd arg")
   os.exit(1)
end
if not outfilelua:match( ".lua$" ) then
   print("expecting file.lua as third arg")
   os.exit(1)
end


local ofad=io.open( outfilead, "w" )
local oflua=io.open( outfilelua, "w" )
local rfile = io.open( filename, "r")
local start = 0
local start_macro=0


local oldprint = print
print = function( ... )
   ofad:write(...)
   ofad:write("\n")
end
echo = function( ... )
   ofad:write(...)
end

lines=""

for line in rfile:lines() do
    if start==0 and line:match("%%LUA") then
       start=1
    elseif start == 1 and line:match("%%END") then
       start=0
    elseif start==1 then
       oflua:write( line .. "\n" )
    elseif start==0 and line:match("<%?LUA") then
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
       ofad:write( line .. "\n" )
    end
	
end
