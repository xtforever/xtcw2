% ------------------------------------------------------------------------------------
% -- to demonstrate, we can define some constants or functions 
% -- which can be used later in the precompiler sections 
% ------------------------------------------------------------------------------------
%%LUA
 num_edit_widgets= "8" 
%%END


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


%%LUA
function on_create_gui(a)
   label_widget = xtcreate( "NewLabel", "WLabel", "*icnBox", "label", a, "gridx", "3", "weightx", "100", "fill","Both")
end
%%END

*gb2.WcClass: gridbox
*gb2.WcChildren: sq3 t1 menu icnBox
*gb2.WcCallback: LUA( on_create_gui("hello world") )

*icnBox.WcClass: HBox
*icnBox.gridy: 0
*icnBox.WcChildren: icn1 icn2

*icn1.WcClass: IconSVG
*icn1.gridx: 0
*icn1.filename: alert.svg
*icn1.forced_width: 1cm
*icn1.forced_height: 1cm


*icn2.WcClass: IconSVG
*icn2.gridx: 1
*icn2.filename: question.svg




%%------------- container for toolbar box and edit box ---
*sq3.WcClass: gridbox
*sq3.gridy: 4
*sq3.weightx: 1
*sq3.weighty: 1
*sq3.fill: Both
*sq3.WcChildren: tb sq

*sq3.tb.gridy: 0
*sq3.tb.weighty : 0

*sq3.sq.gridy: 1
*sq3.tb.weighty: 100
*sq3.tb.weightx: 100


%%------------- toolbar box ----------------
*tb.WcClass: HBox

*tb.left.WcClass: Wbutton
*tb.left.label:left
*tb.reset.WcClass: Wbutton
*tb.reset.label:reset
*tb.right.WcClass: Wbutton
*tb.right.label:right
*tb.WcChildren: left reset right



%%------------- edit box ----------------
*sq.WcClass: gridbox
*sq.gridy: 4
*sq.weightx: 1
*sq.weighty: 1
*sq.fill: Both


*sq.line1.WcClass: WeditMV
*sq.line1.foreground: RED
*sq.line1.cursorColor: BLUE
*sq.line1.gridy:1
*sq.line1.weightx: 1
*sq.line1.fill: Width
*sq.line1.translations: #override <Key>Tab: Focus(*sq.line2)
*sq.WcChildren: line1
*sq.line2.WcClass: WeditMV
*sq.line2.foreground: RED
*sq.line2.cursorColor: BLUE
*sq.line2.gridy:2
*sq.line2.weightx: 1
*sq.line2.fill: Width
*sq.line2.translations: #override <Key>Tab: Focus(*sq.line3)
*sq.WcChildren: line1 line2
*sq.line3.WcClass: WeditMV
*sq.line3.foreground: RED
*sq.line3.cursorColor: BLUE
*sq.line3.gridy:3
*sq.line3.weightx: 1
*sq.line3.fill: Width
*sq.line3.translations: #override <Key>Tab: Focus(*sq.line4)
*sq.WcChildren: line1 line2 line3
*sq.line4.WcClass: WeditMV
*sq.line4.foreground: RED
*sq.line4.cursorColor: BLUE
*sq.line4.gridy:4
*sq.line4.weightx: 1
*sq.line4.fill: Width
*sq.line4.translations: #override <Key>Tab: Focus(*sq.line5)
*sq.WcChildren: line1 line2 line3 line4
*sq.line5.WcClass: WeditMV
*sq.line5.foreground: RED
*sq.line5.cursorColor: BLUE
*sq.line5.gridy:5
*sq.line5.weightx: 1
*sq.line5.fill: Width
*sq.WcChildren: line1 line2 line3 line4 line5
*sq.line6.WcClass: WeditMV
*sq.line6.foreground: RED
*sq.line6.cursorColor: BLUE
*sq.line6.gridy:6
*sq.line6.weightx: 1
*sq.line6.fill: Width
*sq.WcChildren: line1 line2 line3 line4 line5 line6
*sq.line7.WcClass: WeditMV
*sq.line7.foreground: RED
*sq.line7.cursorColor: BLUE
*sq.line7.gridy:7
*sq.line7.weightx: 1
*sq.line7.fill: Width
*sq.WcChildren: line1 line2 line3 line4 line5 line6 line7
*sq.line8.WcClass: WeditMV
*sq.line8.foreground: RED
*sq.line8.cursorColor: BLUE
*sq.line8.gridy:8
*sq.line8.weightx: 1
*sq.line8.fill: Width
*sq.WcChildren: line1 line2 line3 line4 line5 line6 line7 line8


% ------------------------------------------------------------------------------------
% -- to demonstrate, we can export values from the pre-compiler to the runtime code --
% ------------------------------------------------------------------------------------
%%LUA
 edit_widgets= " line1 line2 line3 line4 line5 line6 line7 line8" 
%%END
%%LUA
 edit_container= "*sq" 
%%END


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
