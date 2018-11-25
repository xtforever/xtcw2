/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_COMMON_H
#define XTCW_COMMON_H
#include <X11/Composite.h>
_XFUNCPROTOBEGIN
typedef int  Alignment;

#define XfwfCenter 0 


#define XfwfLeft 1 


#define XfwfRight 2 


#define XfwfTop 4 


#define XfwfBottom 8 


#define XfwfTopLeft (XfwfTop +XfwfLeft )


#define XfwfTopRight (XfwfTop +XfwfRight )


#define XfwfBottomLeft (XfwfBottom +XfwfLeft )


#define XfwfBottomRight (XfwfBottom +XfwfRight )


typedef enum {
	    TraverseLeft, TraverseRight, TraverseUp, TraverseDown,
	    TraverseNext, TraversePrev, TraverseHome, TraverseNextTop } TraversalDirection;

typedef Pixel  Color;

void XfwfCallComputeInside(
#if NeedFunctionPrototypes
Widget,Position *,Position *,Dimension *,Dimension *
#endif
);
Dimension  XfwfCallFrameWidth(
#if NeedFunctionPrototypes
Widget
#endif
);
#include "XCC.h"
#ifndef XtNuseXCC
#define XtNuseXCC "useXCC"
#endif
#ifndef XtCUseXCC
#define XtCUseXCC "UseXCC"
#endif
#ifndef XtRBoolean
#define XtRBoolean "Boolean"
#endif

#ifndef XtNusePrivateColormap
#define XtNusePrivateColormap "usePrivateColormap"
#endif
#ifndef XtCUsePrivateColormap
#define XtCUsePrivateColormap "UsePrivateColormap"
#endif
#ifndef XtRBoolean
#define XtRBoolean "Boolean"
#endif

#ifndef XtNuseStandardColormaps
#define XtNuseStandardColormaps "useStandardColormaps"
#endif
#ifndef XtCUseStandardColormaps
#define XtCUseStandardColormaps "UseStandardColormaps"
#endif
#ifndef XtRBoolean
#define XtRBoolean "Boolean"
#endif

#ifndef XtNstandardColormap
#define XtNstandardColormap "standardColormap"
#endif
#ifndef XtCStandardColormap
#define XtCStandardColormap "StandardColormap"
#endif
#ifndef XtRAtom
#define XtRAtom "Atom"
#endif

#ifndef XtNxcc
#define XtNxcc "xcc"
#endif
#ifndef XtCXCc
#define XtCXCc "XCc"
#endif
#ifndef XtRXCC
#define XtRXCC "XCC"
#endif

#ifndef XtNtraversalOn
#define XtNtraversalOn "traversalOn"
#endif
#ifndef XtCTraversalOn
#define XtCTraversalOn "TraversalOn"
#endif
#ifndef XtRBoolean
#define XtRBoolean "Boolean"
#endif

#ifndef XtNhighlightThickness
#define XtNhighlightThickness "highlightThickness"
#endif
#ifndef XtCHighlightThickness
#define XtCHighlightThickness "HighlightThickness"
#endif
#ifndef XtRDimension
#define XtRDimension "Dimension"
#endif

#ifndef XtNhighlightColor
#define XtNhighlightColor "highlightColor"
#endif
#ifndef XtCHighlightColor
#define XtCHighlightColor "HighlightColor"
#endif
#ifndef XtRColor
#define XtRColor "Color"
#endif

#ifndef XtNbackground
#define XtNbackground "background"
#endif
#ifndef XtCBackground
#define XtCBackground "Background"
#endif
#ifndef XtRColor
#define XtRColor "Color"
#endif

#ifndef XtNhighlightPixmap
#define XtNhighlightPixmap "highlightPixmap"
#endif
#ifndef XtCHighlightPixmap
#define XtCHighlightPixmap "HighlightPixmap"
#endif
#ifndef XtRPixmap
#define XtRPixmap "Pixmap"
#endif

#ifndef XtNnextTop
#define XtNnextTop "nextTop"
#endif
#ifndef XtCNextTop
#define XtCNextTop "NextTop"
#endif
#ifndef XtRCallback
#define XtRCallback "Callback"
#endif

#ifndef XtNuserData
#define XtNuserData "userData"
#endif
#ifndef XtCUserData
#define XtCUserData "UserData"
#endif
#ifndef XtRPointer
#define XtRPointer "Pointer"
#endif

typedef struct _CommonClassRec *CommonWidgetClass;
typedef struct _CommonRec *CommonWidget;
externalref WidgetClass commonWidgetClass;
_XFUNCPROTOEND
#endif /* XTCW_COMMON_H */
