print ("- lua has started -")


x=0
repeat

        fin = luaxt.luaxt_processevent()

        repeat
          str = luaxt.luaxt_pullcallback()
          if str ~= "" then
             print("you want me to evaluate this? " .. str )
             f = load( str )
             if f then f() end
          end
        until str==""

until   fin ~= 0

print ("- lua has stopped -")

