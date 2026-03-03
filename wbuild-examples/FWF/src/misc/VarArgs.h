#ifndef _VarargsI_h_ 
#define _VarargsI_h_ 

#include <X11/Xfuncproto.h>

#if NeedVarargsPrototypes
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

#include <X11/Intrinsic.h>


XtVarArgsList  XfwfCreateArgsList();
#endif
