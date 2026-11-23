/* gridbox_compat.h - public API retained from the plain-C Gridbox header.
 *
 * wbuild emits public resource macros for @PUBLIC resources, but constraint
 * resources live in @private-constraints (wbuild's @CONSTRAINTS generation is
 * not usable here).  This header keeps the historical XtN, XtC and XtR names
 * and the FillType API visible to existing callers.
 */
#ifndef XTCW_GRIDBOX_COMPAT_H
#define XTCW_GRIDBOX_COMPAT_H

#include <X11/Constraint.h>
#include <X11/StringDefs.h>

#ifndef XtNgridx
#define XtNgridx "gridx"
#endif
#ifndef XtNgridy
#define XtNgridy "gridy"
#endif
#ifndef XtNgridWidth
#define XtNgridWidth "gridWidth"
#endif
#ifndef XtNgridHeight
#define XtNgridHeight "gridHeight"
#endif
#ifndef XtNfill
#define XtNfill "fill"
#endif
#ifndef XtNmargin
#define XtNmargin "margin"
#endif
#ifndef XtNweightx
#define XtNweightx "weightx"
#endif
#ifndef XtNweighty
#define XtNweighty "weighty"
#endif
#ifndef XtCFill
#define XtCFill "Fill"
#endif
#ifndef XtCWeight
#define XtCWeight "Weight"
#endif
#ifndef XtRFillType
#define XtRFillType "FillType"
#endif
#ifndef XtNgravity
#define XtNgravity "gravity"
#endif
#ifndef XtCGravity
#define XtCGravity "Gravity"
#endif

#ifndef XTCW_FILLTYPE_DEFINED
#define XTCW_FILLTYPE_DEFINED
typedef unsigned int FillType;
#endif

#define FillNone 0
#define FillWidth 1
#define FillHeight 2
#define FillBoth (FillWidth|FillHeight)

#endif /* XTCW_GRIDBOX_COMPAT_H */
