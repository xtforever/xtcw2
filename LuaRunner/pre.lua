 num_edit_widgets= "8" 
function on_create_gui(a)
   label_widget = xtcreate( "NewLabel", "WLabel", "*icnBox", "label", a, "gridx", "3", "weightx", "100", "fill","Both")
end
 edit_widgets= " line1 line2 line3 line4 line5 line6 line7 line8" 
 edit_container= "*sq" 
function paint1()
        print("hallo x=" .. class_data['x'] )
        print("hallo x=" .. class_data.x )
end



x=0
cb=""
repeat

        fin = luaxt.luaxt_processevent()

        repeat
          cb = luaxt.luaxt_pullcallback()
          if cb ~= "" then

  	      class_data = luaxt.luaxt_pullcallback()
          if class_data == "" then class_data="nil" end

          cb1 = "class_data=" ..  class_data .. "\n" .. cb
          print("eval:  " ..  cb1 )

 	      f = load( cb1 )
          if f then f() end
         end
        until cb==""

until   fin ~= 0

print ("- lua has stopped -")
