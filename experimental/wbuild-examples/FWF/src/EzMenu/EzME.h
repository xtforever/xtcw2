/*
 * $XConsortium: EzME.h,v 1.4 89/12/11 15:20:09 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * EzME.h - Public Header file for EzME object.
 *
 * This is the public header file for the Athena EzME object.
 * It is intended to be used with the simple menu widget.  
 *
 * Date:    April 3, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#ifndef _EzME_h
#define _EzME_h

#include <X11/RectObj.h>
#include <X11/Xaw/Sme.h>

/****************************************************************
 *
 * EzME Object
 *
 ****************************************************************/

typedef struct _XfwfEzMEClassRec*	XfwfEzMEObjectClass;
typedef struct _XfwfEzMERec*	        XfwfEzMEWidget;

#define XtNaction "action"
#define XtCAction "Action"

#ifndef XtNmenuName
#  define XtNmenuName "menuName"
#  define XtCMenuName "MenuName"
#endif

extern WidgetClass xfwfEzMEObjectClass;

#endif /* _EzME_h */
