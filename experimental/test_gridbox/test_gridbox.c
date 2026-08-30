/*
 * Minimal test: create a Gridbox, add two labels with gridx/gridy
 * constraint resources, verify the constraints were applied.
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/ConstrainP.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/XawInit.h>
#include "Gridbox.h"
#include "GridboxP.h"
#include "mls.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    XtAppContext app;
    Widget shell, gridbox, label1, label2;

    m_init();
    trace_level = 0;

    shell = XtOpenApplication(&app, "TestGridbox",
        NULL, 0, &argc, argv, NULL,
        sessionShellWidgetClass, NULL, 0);

    if (!shell) {
        fprintf(stderr, "FAIL: could not create shell widget\n");
        return 1;
    }

    /* Create Gridbox as child of shell */
    gridbox = XtCreateManagedWidget("grid", gridboxWidgetClass,
        shell, NULL, 0);
    if (!gridbox) {
        fprintf(stderr, "FAIL: could not create gridbox\n");
        return 1;
    }
    fprintf(stderr, "OK: gridbox created: class=%s\n",
        XtClass(gridbox)->core_class.class_name);

    fprintf(stderr, "XtIsConstraint(gridbox) = %d (flag 0x10 = %d)\n",
        XtIsConstraint(gridbox), !!(XtClass(gridbox)->core_class.class_inited & 0x10));

    /* Check constraint resource list via Xt API */
    XtResourceList constraint_res = NULL;
    Cardinal num_constraint = 0;
    WidgetClass parent_class = XtClass(gridbox);
    XtGetConstraintResourceList(parent_class, &constraint_res, &num_constraint);
    fprintf(stderr, "XtGetConstraintResourceList: num_constraint=%d\n", num_constraint);
    if (constraint_res) {
        for (Cardinal i = 0; i < num_constraint; i++) {
            fprintf(stderr, "  constraint[%d]: name=%s type=%s\n",
                i, constraint_res[i].resource_name, constraint_res[i].resource_type);
        }
        XtFree((char *)constraint_res);
    }

    /* Create label1 with gridx=0, gridy=0 */
    Arg args[4];
    Cardinal n;

    n = 0;
    XtSetArg(args[n], XtNlabel, "Label at 0,0"); n++;
    XtSetArg(args[n], XtNgridx, (XtArgVal)0); n++;
    XtSetArg(args[n], XtNgridy, (XtArgVal)0); n++;
    label1 = XtCreateManagedWidget("label1", labelWidgetClass,
        gridbox, args, n);

    /* Create label2 with gridx=1, gridy=0 */
    n = 0;
    XtSetArg(args[n], XtNlabel, "Label at 1,0"); n++;
    XtSetArg(args[n], XtNgridx, (XtArgVal)1); n++;
    XtSetArg(args[n], XtNgridy, (XtArgVal)0); n++;
    label2 = XtCreateManagedWidget("label2", labelWidgetClass,
        gridbox, args, n);

    XtRealizeWidget(shell);

    /* Read back constraint resources via XtVaGetValues */
    fprintf(stderr, "\n--- Reading back constraint resources via XtVaGetValues ---\n");
    Position gx1, gy1, gx2, gy2;
    Dimension gw1, gw2;
    XtVaGetValues(label1, XtNgridx, &gx1, XtNgridy, &gy1,
                  XtNgridWidth, &gw1, NULL);
    XtVaGetValues(label2, XtNgridx, &gx2, XtNgridy, &gy2,
                  XtNgridWidth, &gw2, NULL);
    fprintf(stderr, "label1: gridx=%d gridy=%d gridWidth=%d\n", gx1, gy1, gw1);
    fprintf(stderr, "label2: gridx=%d gridy=%d gridWidth=%d\n", gx2, gy2, gw2);

    /* Also read back via direct struct access */
    fprintf(stderr, "\n--- Reading via direct struct access ---\n");
    if (label1 && label1->core.constraints) {
        GridboxConstraints gc1 = (GridboxConstraints)label1->core.constraints;
        fprintf(stderr, "label1: gridx=%d gridy=%d gridWidth=%d gridHeight=%d weightx=%d weighty=%d fill=%d\n",
            gc1->gridbox.gridx, gc1->gridbox.gridy,
            gc1->gridbox.gridWidth, gc1->gridbox.gridHeight,
            gc1->gridbox.weightx, gc1->gridbox.weighty,
            gc1->gridbox.fill);
    } else {
        fprintf(stderr, "label1: NO constraint record!\n");
    }

    if (label2 && label2->core.constraints) {
        GridboxConstraints gc2 = (GridboxConstraints)label2->core.constraints;
        fprintf(stderr, "label2: gridx=%d gridy=%d gridWidth=%d gridHeight=%d weightx=%d weighty=%d fill=%d\n",
            gc2->gridbox.gridx, gc2->gridbox.gridy,
            gc2->gridbox.gridWidth, gc2->gridbox.gridHeight,
            gc2->gridbox.weightx, gc2->gridbox.weighty,
            gc2->gridbox.fill);
    } else {
        fprintf(stderr, "label2: NO constraint record!\n");
    }

    /* Check geometry (positions) */
    Position x1, y1, x2, y2;
    Dimension w1, h1, w2, h2;
    XtVaGetValues(label1, XtNx, &x1, XtNy, &y1, XtNwidth, &w1, XtNheight, &h1, NULL);
    XtVaGetValues(label2, XtNx, &x2, XtNy, &y2, XtNwidth, &w2, XtNheight, &h2, NULL);
    fprintf(stderr, "\nlabel1 geom: x=%d y=%d w=%d h=%d\n", x1, y1, w1, h1);
    fprintf(stderr, "label2 geom: x=%d y=%d w=%d h=%d\n", x2, y2, w2, h2);

    if (x2 > x1 && y2 == y1) {
        fprintf(stderr, "\nPASS: label2 is to the right of label1 (grid layout works)\n");
    } else if (x1 == x2 && y1 == y2) {
        fprintf(stderr, "\nFAIL: both labels at same position (constraint resources not applied)\n");
    } else {
        fprintf(stderr, "\nRESULT: label positions: (%d,%d) and (%d,%d)\n", x1, y1, x2, y2);
    }

    if (argc > 1 && strcmp(argv[1], "-keep") == 0) {
        fprintf(stderr, "Keeping window open (close to exit)...\n");
        XtAppMainLoop(app);
    }

    return 0;
}