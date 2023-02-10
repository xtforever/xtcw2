zoom_percent = 100
function zoomin()
  if zoom_percent < 400  then zoom_percent = zoom_percent+ 10 end
  xtsetvalue( "*zoom", "label", zoom_percent )
end

function zoomout()
  if zoom_percent > 20 then zoom_percent = zoom_percent- 10 end
  xtsetvalue( "*zoom", "label", zoom_percent )
end


x=0
cb=""
repeat

        fin = luaxt.luaxt_processevent()

        repeat
          cb = luaxt.luaxt_pullcallback()
          if cb ~= "" then

	     class_data = luaxt.luaxt_pullcallback()

             print("eval=" .. cb .. " class=" ..  class_data )

	     f = load( cb )
             if f then f() end
          end
        until cb==""

until   fin ~= 0

print ("- lua has stopped -")
