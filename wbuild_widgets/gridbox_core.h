/* gridbox_core.h - exported implementation API for the ported Gridbox widget.
 *
 * The .widget wrapper owns the Xt class record, resources and method slots.
 * This header exposes the existing plain-C implementation so the thin wrapper
 * can delegate to it.  The original plainc_widgets/Gridbox.c is untouched.
 */
#ifndef XTCW_GRIDBOX_CORE_H
#define XTCW_GRIDBOX_CORE_H

#include <X11/Intrinsic.h>
#include <X11/Xregion.h>

void gridbox_core_class_initialize(void);

void gridbox_core_resources(XtResourceList *res, Cardinal *n);
void gridbox_core_constraint_resources(XtResourceList *res, Cardinal *n);

void gridbox_core_initialize(Widget request, Widget neww,
                             ArgList args, Cardinal *num_args);
void gridbox_core_destroy(Widget w);
void gridbox_core_expose(Widget w, XEvent *event, Region region);
void gridbox_core_resize(Widget w);
Boolean gridbox_core_set_values(Widget current, Widget request, Widget neww,
                                ArgList args, Cardinal *num_args);
XtGeometryResult gridbox_core_query_geometry(Widget w,
                                             XtWidgetGeometry *request,
                                             XtWidgetGeometry *reply);
void gridbox_core_change_managed(Widget w);
XtGeometryResult gridbox_core_geometry_manager(Widget w,
                                               XtWidgetGeometry *request,
                                               XtWidgetGeometry *reply);
void gridbox_core_constraint_initialize(Widget request, Widget neww,
                                        ArgList args, Cardinal *num_args);
Boolean gridbox_core_constraint_set_values(Widget current, Widget request,
                                           Widget neww, ArgList args,
                                           Cardinal *num_args);

#endif /* XTCW_GRIDBOX_CORE_H */
