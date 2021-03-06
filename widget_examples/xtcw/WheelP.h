/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_WHEELP_H
#define XTCW_WHEELP_H
#include <X11/CoreP.h>
#include <xtcw/Wheel.h>
_XFUNCPROTOBEGIN
typedef int  (*exec_command_Proc)(
#if NeedFunctionPrototypes
Widget,int ,int 
#endif
);
#define XtInherit_exec_command ((exec_command_Proc) _XtInherit)
typedef int  (*sig_recv_Proc)(
#if NeedFunctionPrototypes
Widget,Widget ,int ,void *,void *
#endif
);
#define XtInherit_sig_recv ((sig_recv_Proc) _XtInherit)

typedef struct {
/* methods */
exec_command_Proc exec_command;
sig_recv_Proc sig_recv;
/* class variables */
} WheelClassPart;

typedef struct _WheelClassRec {
CoreClassPart core_class;
WheelClassPart wheel_class;
} WheelClassRec;

typedef struct {
/* resources */
XftFont * xftFont;
XtCallbackList  callback;
Pixel  bg_norm;
Pixel  bg_sel;
Pixel  bg_hi;
Pixel  fg_norm;
Pixel  fg_sel;
Pixel  fg_hi;
int  user_data;
String  focus_group;
int  state;
Boolean  register_focus_group;
/* private state */
Pixel  pixel[6];
XftColor  xft_col[6];
GC  gc[3];
} WheelPart;

typedef struct _WheelRec {
CorePart core;
WheelPart wheel;
} WheelRec;

externalref WheelClassRec wheelClassRec;

_XFUNCPROTOEND
#endif /* XTCW_WHEELP_H */
