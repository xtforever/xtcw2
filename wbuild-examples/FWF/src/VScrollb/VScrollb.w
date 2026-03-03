# VScrollbar.w -- Vertical scrollbar widget
# Author: Bert Bos <bert@let.rug.nl>
# Date: 18 Nov '92
# Copyright: see README file
# Version: 1.0

@class XfwfVScrollbar (XfwfScrollbar)  @file=VScrollb

@ The |XfwfVScrollbar| widget is exactly the same as its superclass,
|XfwfScrollbar|, except for the fact that it has default translations
for keyboard events. It would be a simple matter to add these
translations in a resource file or in an application's source, but
having them by default is quite convenient.


@translations

@ The actions are already defined by the superclass, but they were by
default not used.

@trans Ctrl<Key>Prior: Scroll("top")
@trans Ctrl<Key>Next: Scroll("bottom")

@trans <Key>Up: Scroll("up")
@trans <Key>Down: Scroll("down")

@ Are |PageUp| and |PageDown| always available? Or only in OSF?
@ trans <Key>PageUp: pageForward()
@ trans <Key>PageDown: pageForward()

@trans <Key>Prior: Scroll("pageUp")
@trans <Key>Next: Scroll("pageDown")

