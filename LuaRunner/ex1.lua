print ("- lua has started -")

label_widget = xtcreate( "NewLabel", "WLabel", "*gb1", "gridy", "0", "label", "hello-world" )


tab = { xlabel='hello' , gridy='4' }



xtsetvalue( label_widget, tab, "label", "hello" )



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

