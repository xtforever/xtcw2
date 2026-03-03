#ifndef WPANED_TYPES_H
#define WPANED_TYPES_H

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/ConstrainP.h>

typedef enum { UpLeftPane = 'U', LowRightPane = 'L', ThisBorderOnly = 'T', AnyPane = 'A' } Direction;

typedef struct _PaneStack {
    struct _PaneStack *next;
    void *pane;
    int start_size;
} PaneStack;

/* Resource names */
#define XtNallowResize "allowResize"
#define XtCAllowResize "AllowResize"
#define XtNmin "min"
#define XtCMin "Min"
#define XtNmax "max"
#define XtCMax "Max"
#define XtNpreferredPaneSize "preferredPaneSize"
#define XtCPreferredPaneSize "PreferredPaneSize"
#define XtNresizeToPreferred "resizeToPreferred"
#define XtCResizeToPreferred "ResizeToPreferred"
#define XtNskipAdjust "skipAdjust"
#define XtCSkipAdjust "SkipAdjust"
#define XtNshowGrip "showGrip"
#define XtCShowGrip "ShowGrip"

/* Macros */
#define CPane(w) (&((WPanedConstraintRec*)((w)->core.constraints))->wPaned)
#define HasGrip(w) (CPane(w)->grip != NULL)
#define IsPane(w) ((w)->core.widget_class != gripWidgetClass)
#define PaneIndex(w) (CPane(w)->position)
#define IsVert(pw) (((WPanedWidget)(pw))->wPaned.orientation != 0)
#define PaneSize(w, vert) ((vert) ? (w)->core.height : (w)->core.width)
#define GetRequestInfo(geo, vert) ((vert) ? (geo)->height : (geo)->width)

#define ForAllPanes(pw, childP) for ((childP) = ((CompositeWidget)(pw))->composite.children; (childP) < ((CompositeWidget)(pw))->composite.children + ((WPanedWidget)(pw))->wPaned.num_panes; (childP)++)
#define ForAllChildren(pw, childP) for ((childP) = ((CompositeWidget)(pw))->composite.children; (childP) < ((CompositeWidget)(pw))->composite.children + ((CompositeWidget)(pw))->composite.num_children; (childP)++)
#define SatisfiesRule1(pane, shrink) (((shrink) && ((pane)->size != (pane)->min)) || (!(shrink) && ((pane)->size != (pane)->max)))
#define SatisfiesRule2(pane) (!(pane)->skipAdjust || (pane)->paned_adjusted_me)
#define SatisfiesRule3(pane, shrink) ((pane)->paned_adjusted_me && (((shrink) && ((int)(pane)->wp_size <= (pane)->size)) || (!(shrink) && ((int)(pane)->wp_size >= (pane)->size))))

#ifndef Max
#define Max(a,b) ((a)>(b)?(a):(b))
#endif

#define AssignMax(a,b) if ((a)>(b)) (a)=(b)
#define AssignMin(a,b) if ((a)<(b)) (a)=(b)

#endif
