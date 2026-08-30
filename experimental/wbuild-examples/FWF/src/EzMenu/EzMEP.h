/*
 * $XConsortium: SmeP.h,v 1.4 89/12/11 15:20:22 kit Exp $
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
 * SmeP.h - Private Header file for Sme object.
 *
 * This is the private header file for the Athena EzME object.
 * This object is intended to be used with the simple menu widget.  
 *
 * Date:    April 3, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#ifndef _XawEzMEP_h
#define _XawEzMEP_h

/***********************************************************************
 *
 * EzME Widget Private Data
 *
 ***********************************************************************/

#include <X11/RectObjP.h>
#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SmeBSBP.h>
#include <Xfwf/EzME.h>

/************************************************************
 *
 * New fields for the EzME widget class record.
 *
 ************************************************************/

typedef struct _XfwfEzMEClassPart {
  void (*highlight)();
  void (*unhighlight)();
  void (*notify)();	
  XtPointer extension;
} XfwfEzMEClassPart;

/* Full class record declaration */
typedef struct _XfwfEzMEClassRec {
    RectObjClassPart    rect_class;
    SmeClassPart	sme_class;
    SmeBSBClassPart  sme_bsb_class;
    XfwfEzMEClassPart	ezme_class;
} XfwfEzMEClassRec;

extern XfwfEzMEClassRec xfwfEzmeClassRec;

/* New fields for the EzME widget record */
typedef struct {
    /* resources */
    char *action;
    Widget toPopUp;
} XfwfEzMEPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XfwfEzMERec {
  ObjectPart     object;
  RectObjPart    rectangle;
  SmePart	 sme;
  SmeBSBPart     sme_bsb;
  XfwfEzMEPart	 ezme;
} XfwfEzMERec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/
#endif /* _XawEzMEP_h */
