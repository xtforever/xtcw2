/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_WMENUPOPUPP_H
#define XTCW_WMENUPOPUPP_H
#include <xtcw/WlabelP.h>
#include <xtcw/WmenuPopup.h>
_XFUNCPROTOBEGIN

typedef struct {
/* methods */
/* class variables */
int dummy;
} WmenuPopupClassPart;

typedef struct _WmenuPopupClassRec {
CoreClassPart core_class;
WheelClassPart wheel_class;
WlabelClassPart wlabel_class;
WmenuPopupClassPart wmenuPopup_class;
} WmenuPopupClassRec;

typedef struct {
/* resources */
XtCallbackList  combo_cb;
StringMArray  lst;
/* private state */
Widget  shell;
Widget  listbox;
int  entry_mode;
int  filter;
int  f_lst;
int  k_lst;
int  lines_height;
int  selection;
} WmenuPopupPart;

typedef struct _WmenuPopupRec {
CorePart core;
WheelPart wheel;
WlabelPart wlabel;
WmenuPopupPart wmenuPopup;
} WmenuPopupRec;

externalref WmenuPopupClassRec wmenuPopupClassRec;

_XFUNCPROTOEND
#endif /* XTCW_WMENUPOPUPP_H */
