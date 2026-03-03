# OptButton -- Option button widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0 for FWF V4.0

@class XfwfOptionButton (XfwfPullDown)  @file=OptButton

@ The option button is very much like a PullDown button, except that
the label of the button is automatically set to the label of the last
selected menu item.

There are two ways to specify the menu: with the |popup|
resource, or with the |menu| resource.
When the |popup| resource is used, the option button cannot update
its own label, so it becomes the responsibility of the application
that created the pop up widget to set the label.

For more explanation of |menu| and |popup|, see the XfwfPullDown
class.



@PUBLIC

@ The frame width is again set to a default of 2 pixels.

	@var frameWidth = 2



@METHODS

@ The |initialize| method adds a callback to the menu, but only if it
has been created in the PullDown widget.

@proc initialize
{
    if ($own_popup)
	XtAddCallback($popup, XtNactivate, set_label_cb, $);
}

@ Similarly, the |set_values| methods adds the same callback if a new
menu has been created in the |set_values| method of XfwfPullDown.

@proc set_values
{
    if ($old$popup != $popup && $own_popup)
	XtAddCallback($popup, XtNactivate, set_label_cb, $);
    return False;
}


@TRANSLATIONS

@ Since wbuild cannot yet inherit translations, they must be
given again.

	@trans <Btn1Down>: prepare() open_menu()
	@trans Button1<Enter>: prepare() open_menu()
	@trans <Key>Return: prepare() open_menu()
	@trans <Btn1Up>: popdown_maybe()


@UTILITIES

@ The |set_label_cb| routine is called when an option from the options
menu is selected. It will set the label of the option button.

@proc set_label_cb(Widget menu, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    if ($label && strcmp($label, data->label) == 0) return;
    XtVaSetValues($, XtNlabel, data->label, NULL);
}



@IMPORTS

@incl <stdio.h>
@incl <Xfwf/TextMenu.h>
