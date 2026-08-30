# RadioGrp.w -- RadioGroup widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0 for FWF 4.0

@class XfwfRadioGroup (XfwfGroup)  @file=RadioGrp

@ The RadioGroup widget is a simple specialization of the Group
widget. It has an extra resource, |labels|, which holds the labels of
the toggle buttons that are created automatically. For the common case
that a Group widget contains only radio buttons, the RadioGroup widget
is much more convenient. It is also much less flexible. E.g., it is
very difficult to change resources of the radio buttons, when the
defaults are not satisfactory.

In particular, the Toggle widgets are created with the following
hardcoded resources: |shrinkToFit = True|, |border_width = 0|,
|frameWidth = 0|. The names of the Toggle widgets are equal to their
labels.

@public

@ The |labels| resource is an array of strings. Each string will
become the label of a toggle button. The last member of the array must
be a |NULL| pointer.

There is a converter that allows the list of strings to be entered as
a single string, with any character as separator.

	@var StringArray labels = NULL


@methods

@ In the |initialize| method, the utility function |create_toggles| is
called, which creates toggle widgets for each label in the |labels|
resource.

@proc initialize
{
    $labels = newStringArray($labels);
    create_toggles($);
}

@ The |set_values| method makes a private copy of the |labels|
resource and calls |create_labels|. Since the RadioGroup doesn't draw
anything itself, the return value is always |False|.

@proc set_values
{
    if ($old$labels != $labels) {
	freeStringArray($old$labels);
	$labels = newStringArray($labels);
	create_toggles($);
    }
    return False;
}

@utilities

@ The function |create_toggles| first destroys all existing children
and then creates new ones. For each string in the |labels| resource
there will be an XfwfToggle button.

@proc create_toggles($)
{
    Cardinal i;
    StringArray s;

    for (i = 0; i < $num_children; i++) 
	XtDestroyWidget($children[i]);

    if ($labels == NULL) return;

    for (s = $labels, i = 0; s[i] != NULL; i++) {
	(void) XtVaCreateManagedWidget
	    (s[i], xfwfToggleWidgetClass, $, XtNlabel, s[i], XtNshrinkToFit,
	     True, XtNborderWidth, 0, XtNframeWidth, 0, NULL);
    }
}

@imports

@incl <Xfwf/Toggle.h>

@exports

@incl <Xfwf/Converters.h>
