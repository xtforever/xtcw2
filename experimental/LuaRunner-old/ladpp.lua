#!/usr/bin/env lua
-- read a adp-file from stdin, prozess the embedded lua code and output the result to stdout
-- FILE: ladpp.lua
-- NAME: lua xt-application-defaults preprozessor


print = function( ... )
   io.write(...)
   io.write("\n")
end
echo = function( ... )
   io.write(...)
end
perror = function( ... )
   io.stderr:write( ... )
end

start=0
cnt=0
for line in io.lines() do
   cnt=cnt+1
    if start==0 and line:match("<%?LUA") then
        lines=""
	start=2
    elseif start == 2 and line:match("^%?>$") then
       start=0
       f = load(lines)
       if not f then
	  perror( "ERROR 1 loading lua code in line: " .. cnt .. "\n" )
	  print( lines )
       else
	  local status, err =  pcall(f)
	  if( not status ) then 
	     perror( "ERROR " .. err .. " in lua code line: " .. cnt .. "\n" )
	     perror( lines )
	  end
       end
       lines=""
    elseif start==2 then
       lines = lines .. line .. "\n"
    elseif start==0 then
       io.write( line .. "\n" )
    end	
end
