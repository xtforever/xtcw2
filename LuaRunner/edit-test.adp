

%%
%% set default colors
%%
*allowShellResize: true

*foreground: #1cf0f1
*background: #ffffff
*Gridbox.background: Lightblue

*fg_norm: #ecf0f1
*bg_norm: #27ae60

*bg_sel: Red
*fg_sel: White

*bg_hi: Green
*fg_hi: Yellow




% disabled
*Wswitch.bg_norm: 	#bdc3c7
*Wswitch.fg_norm:	#ecf0f1
*Wswitch.sizex: 	30pt

% enabled
*Wswitch.bg_sel:	#34495e
*Wswitch.fg_sel: 	#1abc9c

% mouse-over
*Wswitch.bg_hi:		#2ecc71
*Wswitch.fg_hi: 	#95a5a6


% disabled
*Woption.bg_norm: 	#bdc3c7
*Woption.fg_norm:	#ecf0f1

% enabled
*Woption.bg_hi:		#34495e
*Woption.fg_hi: 	#1abc9c

% mouse-over
*Woption.bg_sel:	#2ecc71
*Woption.fg_sel: 	#95a5a6

%% light-green: #1abc9c
*Wedit.foreground:	#008c7c
*Wedit.cursorColor:	#95a5a6
*Wedit.fontName: Source Code Pro-10
%% *WeditMV.background: #661166
%% *Wedit.background: #ecf0f1


*Wbutton.xftFont:  DejaVu Sans Condensed-12
*xftFont: DejaVu Sans Condensed-12
*family:  DejaVu Sans Condensed

*Wbutton.borderWidth: 1
*Wbutton.borderColor: #0000C0

%% *Wbutton.height: 80
*borderWidth: 0
%%
%% -------------------


%%! ONLY FOR TESTING
*borderColor: Black
*borderWidth: 2
*background:LIGHTBLUE
*fill: Both
*weightx: 1
*weighty: 1

luarunner.WcChildren: gb2

*gb2.WcClass: gridbox
*gb2.WcChildren: sq t1 menu icnBox


*icnBox.WcClass: HBox
*icnBox.gridy: 0
*icnBox.WcChildren: icn1 icn2

*icn1.WcClass: IconSVG
*icn1.gridx: 0
*icn1.filename: alert.svg

*icn2.WcClass: IconSVG
*icn2.gridx: 1
*icn2.filename: question.svg




*sq.WcClass: gridbox
*sq.gridy: 4
*sq.weightx: 1
*sq.weighty: 1
*sq.fill: Both


<?LUA

child=""
for i=1,5 do
   child=child .. " line" .. i
   w="*sq.line" .. i
   print( w .. ".WcClass: Wedit" )
   print( w .. ".gridy:" .. i )
   print( w .. ".weightx: 1" )
   print( w .. ".fill: Width" )
   if i < 5 then
     print( w .. ".callback: Focus( *sq.line" .. (i+1) .. ")" )
     print( w .. ".translations: #override <Btn1Down>: set_cursor() Focus(*sq.line" .. i .. ")\\n<Key>Tab: Focus(*sq.line" .. (i+1) .. ")" )
     


   end
   print( "*sq.WcChildren:" .. child )
end

?>



*painter1.gridy: 1

*zoom.WcClass: Wedit
*zoom.label: 500
*zoom.fill: none
*zoom.gravity: center
*zoom.weighty: 0

*sc.gridy: 1
*sc.WcClass: ScrolledCanvas
*sc.canvas.callback: canvas_draw_cb    
*sc*foreground: BLUE
*sc.canvas.width: 300
*sc.canvas.height: 300
%% *sc.canvas.translations: <Btn1Down>: LUA("paint1")

*t1.WcClass: WButton
*t1.label: warn
*t1.fill: none
*t1.gravity: center
*t1.weighty: 0
*t1.gridy: 2
*t1.callback: MessageBox(WARN, "Hello World")

*menu.WcClass: WmenuPopup
*menu.gridy: 3
*menu.weighty: 0








*painter1.WcClass: Canvas
*painter1.xftFont: Sans-48
*painter1.weightx: 1000
*painter1.callback: canvas_draw_cb
*painter1.width: 500
*painter1.height: 500


*painter2.gridx: 1
*painter2.WcClass: VSlider
*painter2.foreground: Black
*painter2.weightx: 0
*painter2.weighty: 1000

*painter3.WcClass: HSlider
*painter3.gridy: 1
*painter3.foreground: Black
*painter3.weighty: 0
*painter3.weightx: 0

*term.gridy: 1
*term.weighty: 1
*term.WcClass: Radio2
*term.lst: one, two, three, four, five, six
*term.xftFont: Source Code Pro-8
*term.bg_norm: black
*term.bg_hi: blue
*term.bg_sel: black
*term.fg_norm: green
*term.fg_hi: green
*term.fg_sel: green
*term.var5: myvarname
*term.callback: LUA(k=var5[class_data];  print("hallo " .. class_data .. ' ' .. k[0] .. ' ' .. k[1] .. ' ' .. k[2] )  )



*b1.gridy: 2
*b1.WcClass: wbutton
*b1.label: Exit1
*b1.callback: wcquit()
*b1.weightx: 0
*b1.weighty: 0





*ot4.gridy: 3
*ot4.gridx: 0
*ot4.WcClass: Wsqlcombo
*ot4.label: Hello
*ot4.borderColor: Blue
*ot4.borderWidth: 8
*bt.fill: Both


%%LUA
function paint1()
        print("hallo x=" .. class_data['x'] )
        print("hallo x=" .. class_data.x )
end

%%END


%%LUA


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
%%END
