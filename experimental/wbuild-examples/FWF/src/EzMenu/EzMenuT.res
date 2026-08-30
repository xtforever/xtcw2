
Eztest.label.label:This is a label with a menu; hit button 3 now.
Eztest.label.translations:#replace\n\
	<Btn3Down>: XawPositionSimpleMenu(menu) MenuPopup(menu)


Eztest.menu.label: EzTest Menu
Eztest.menu.RowHeight: 16
Eztest.menu.menu:\
	(line)\
	(item "Meaningless Item" (action "does-not-exist()"))\
	(item "Another Meaningless Item" (action "does-not-exist()"))\
	(blank)\
	(menu "Submenu..." \
		(line)\
		(item "Multiple actions here" \
		   (action "does-not-exist() nor-does-this()"))\
		(item "Single Action" (action "does-not-exist()"))\
	)\
	(blank)\
	(line)\
	(blank)\
	(item "Last entry!" (action "does-not-exist()"))
