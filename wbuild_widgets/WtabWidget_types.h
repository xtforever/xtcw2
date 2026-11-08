#ifndef XTCW_WTABWIDGET_TYPES_H
#define XTCW_WTABWIDGET_TYPES_H

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/ConstrainP.h>

/* tabTitle constraint resource name / class */
#define XtNtabTitle "tabTitle"
#define XtCtabTitle "TabTitle"

/* Access the per-child constraint record of a WtabWidget child. */
#define CTab(w) (&((WtabWidgetConstraintRec*)((w)->core.constraints))->wtabWidget)

#endif /* XTCW_WTABWIDGET_TYPES_H */
