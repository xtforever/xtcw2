#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/RadioGrp.h>
#include <Xfwf/Button.h>

static XtAppContext app_context;
static Widget toplevel, rowcol, radiogroup1, radiogroup2, radiogroup3,
    quit_button; 

static String fallback_resources[] = {
    "ToggleT.rowcol.width: 800",
    "ToggleT.rowcol.height: 300",
    "ToggleT*borderWidth: 0",
    "ToggleT*background: aquamarine",

    "ToggleT.rowcol.innerOffset: 4",

    "*XfwfRadioGroup.foreground: yellow",
    "*XfwfRadioGroup.font: *-helvetica-bold-r-*--14-*-*-*-*-*-*-*",

    /******************* radiogroup1 *************************************
     *
     * The next 3 resources keep the group a fixed width, but the
     * height is calculated from the total height of the children.
     *
     *********************************************************************/
    "ToggleT*radiogroup1.location: 20 20 260 250",
    "ToggleT*radiogroup1.columns: 0",
    "ToggleT*radiogroup1.shrinkToFit: True",

    "ToggleT*radiogroup1.outerOffset: 5",	/* Together, these give */
    "ToggleT*radiogroup1.innerOffset: 5",	/*   a 14 pixel high frame */
    "ToggleT*radiogroup1.frameWidth: 4",	/*   for the label */
    "ToggleT*radiogroup1.label: Fruits:",

    "ToggleT*radiogroup1*onIcon: filleddiamond", /* Recommended for */
    "ToggleT*radiogroup1*offIcon: emptydiamond", /*   radiobuttons */
    "ToggleT*radiogroup1*alignment: left",

    /******************* radiogroup2 *************************************
     *
     * The second radio group differs from the first in two respects:
     * it allows any number of buttons to be `on', and
     * the labels are specified as a resource, instead of a StringArray.
     *
     *********************************************************************/
    "ToggleT*radiogroup2.location: 280 20 260 250",
    "ToggleT*radiogroup2.columns: 0",
    "ToggleT*radiogroup2.shrinkToFit: True",

    "ToggleT*radiogroup2.outerOffset: 5",
    "ToggleT*radiogroup2.innerOffset: 5",
    "ToggleT*radiogroup2.frameWidth: 4",
    "ToggleT*radiogroup2.label: Streets:",
    "ToggleT*radiogroup2.labels: |Astraat|Brugstraat|Grote Markt|Herestraat",
    "ToggleT*radiogroup2.selectionStyle: multi",

    /* "ToggleT*radiogroup2*onIcon: filledsquare", */
    /* "ToggleT*radiogroup2*offIcon: emptysquare", */
    "ToggleT*radiogroup2*alignment: left",

    /******************* radiogroup3 *************************************
     *
     * The third radio group enforces that at all times exactly one of
     * the toggles is `on'. The labels are again given as a resource,
     * this time with `*' as a delimiter.
     *
     *********************************************************************/
    "ToggleT*radiogroup3.location: 560 20 260 250",
    "ToggleT*radiogroup3.columns: 0",
    "ToggleT*radiogroup3.shrinkToFit: True",

    "ToggleT*radiogroup3.outerOffset: 5",
    "ToggleT*radiogroup3.innerOffset: 5",
    "ToggleT*radiogroup3.frameWidth: 4",
    "ToggleT*radiogroup3.label: Names:",
    "ToggleT*radiogroup3.labels: *John*Harry*Bill*Richard*Fred*Patrick*Jack",
    "ToggleT*radiogroup3.selectionStyle: one",
    "ToggleT*radiogroup3.selection: 0",

    "ToggleT*radiogroup3*onIcon: filleddiamond", /* Recommended for */
    "ToggleT*radiogroup3*offIcon: emptydiamond", /*   radiobuttons */
    "ToggleT*radiogroup3*alignment: left",

    /******************* quit button *************************************/

    "ToggleT*quit.location: 1.0-65 1.0-35 60 35", /* Lower right corner */
    "ToggleT*quit.label: Quit",

    NULL
};

/*
 * The labels for the first radio group are given as a StringArray
 */
static String items[] = {
    "orange",
    "apple",
    "melon",
    "grapefruit",
    "strawberry",
    "cherry",
    "banana",
    NULL
};

static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    long choice1, choice2, choice3;
    String s;
    StringArray a2, a3;
    int i;

    /*
     * Getting the labels from the 2nd and 3rd radiogroups is a bit
     * more difficult than from the 1st, since they were specified as
     * resources. They must be retrieved from the widgets themselves.
     *
     * The second radiogroup has selectionStyle `multiple', so the
     * returned choice2 is actually a set of selections.
     */
    XtVaGetValues(radiogroup1, XtNselection, &choice1, NULL);
    XtVaGetValues(radiogroup2, XtNselection, &choice2, XtNlabels, &a2, NULL);
    XtVaGetValues(radiogroup3, XtNselection, &choice3, XtNlabels, &a3, NULL);
    printf("The final selections were:\n");

    printf("\tgroup1: %s\n", choice1 >= 0 ? items[choice1] : "(none)");

    printf("\tgroup2:");
    if (choice2 == 0)
	printf(" (none)");
    else
	for (i = 0; i < 8 * sizeof(choice2); i++)
	    if (choice2 & (1L << i)) printf(" %s", a2[i]);
    printf("\n");

    printf("\tgroup3: %s\n", choice3 >= 0 ? a3[choice3] : "(none)");

    exit(0);
}


int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize
	(&app_context, "ToggleT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    rowcol = XtVaCreateManagedWidget
	("rowcol", xfwfRowColWidgetClass, toplevel, NULL);
    radiogroup1 = XtVaCreateManagedWidget
	("radiogroup1", xfwfRadioGroupWidgetClass, rowcol,
	 XtNlabels, items, NULL);
    radiogroup2 = XtVaCreateManagedWidget
	("radiogroup2", xfwfRadioGroupWidgetClass, rowcol, NULL);
    radiogroup3 = XtVaCreateManagedWidget
	("radiogroup3", xfwfRadioGroupWidgetClass, rowcol, NULL);
    quit_button = XtVaCreateManagedWidget
	("quit", xfwfButtonWidgetClass, rowcol, NULL);
    XtAddCallback(quit_button, XtNactivate, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
