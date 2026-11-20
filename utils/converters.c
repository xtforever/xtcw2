#include "converters.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/Misc.h>
#include <X11/Xmu/SysUtil.h>
#include <X11/Xaw/ToggleP.h>
#include <X11/Xaw/XawInit.h>
#include "converters-xft.h"
#include "strarray.h"
#include "mls.h"





/* String    to   Distance

   Xt Resource Converter to specify a number relative to screen size
   and screen resolution

   String: <Number><Unit>

   Number: see strtol() function

   Unit:  "pt", "mm", "cm", "in", "px"

   Valid Numbers:
   -30
   0x102
   0555
   100pt
   10cm
   200mm
   100px

  convert Pt to Int
    72pt == 1 inch

  convert mm to Int
    254mm == 1 inch

  convert Px to Int
     10000px == Root Screen Width
     100px   == 1% Root Screen Width

   

   example:
     172Pt ==   (DPI*172)/72
     -172Pt == -(DPI*172)/72


   To get current DPI:
     XGetDefault( dpy, "Xft", "dpi" );

   To get screen Width:
     XGetGeometry( dpy,DefaultRootWindow(dpy), ... )
*/

/* Round a double to the nearest int (half away from zero). */
static int dist_round(double v)
{
    return (int)(v >= 0.0 ? v + 0.5 : v - 0.5);
}

double converters_dpi_from_display(Display *dpy)
{
    char *s;
    long dpi;

    if (!dpy)
        return 96.0;
    s = XGetDefault(dpy, "Xft", "dpi");
    if (!s)
        return 96.0;
    dpi = strtol(s, NULL, 0);
    if (dpi < 1 || dpi > 100000)
        return 96.0;
    return (double)dpi;
}

double converters_dpi(Widget w)
{
    return converters_dpi_from_display(w ? XtDisplay(w) : NULL);
}

double converters_pt2px(Widget w, double pt)
{
    return pt * converters_dpi(w) / 72.0;
}

double converters_px2pt(Widget w, double px)
{
    return px * 72.0 / converters_dpi(w);
}

/* Parse a "Distance" specification ("10.5pt", "3mm", "1in", "20px" or a
 * bare pixel count) into whole pixels.  Returns 1 on success. */
int converters_distance_from_display(Display *dpy, const char *str, int *out)
{
    double num, px, dpi;
    char *endptr;
    int type = 0;
    int cnt;
    int len;
    unsigned width = 1024;

    if (!str || !out)
        return 0;
    len = (int)strlen(str);
    if (len <= 0)
        return 0;

    if (len > 2) {
        if (toupper((unsigned char)str[len - 2]) == 'P' &&
            toupper((unsigned char)str[len - 1]) == 'T')
            type = 1;
        else if (toupper((unsigned char)str[len - 2]) == 'C' &&
                 toupper((unsigned char)str[len - 1]) == 'M')
            type = 2;
        else if (toupper((unsigned char)str[len - 2]) == 'M' &&
                 toupper((unsigned char)str[len - 1]) == 'M')
            type = 3;
        else if (toupper((unsigned char)str[len - 2]) == 'P' &&
                 toupper((unsigned char)str[len - 1]) == 'X')
            type = 4;
        else if (toupper((unsigned char)str[len - 2]) == 'I' &&
                 toupper((unsigned char)str[len - 1]) == 'N')
            type = 5;
    }

    /* Integer-looking values keep strtol's base-0 syntax (0x.., 0..);
       decimal values are accepted for physical units (e.g. 10.5pt). */
    if (strchr(str, '.'))
        num = strtod(str, &endptr);
    else
        num = (double)strtol(str, &endptr, 0);
    cnt = (int)(endptr - str);

    switch (type) {
    case 0:
        if (*endptr != 0 && *endptr != ' ')
            return 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        if (cnt != len - 2)
            return 0;
        break;
    default:
        return 0;
    }

    dpi = converters_dpi_from_display(dpy);
    switch (type) {
    case 0:
        px = num;
        break;
    case 1:
        px = num * dpi / 72.0;
        break;
    case 2:
        px = num * dpi * 100.0 / 254.0;
        break;
    case 3:
        px = num * dpi * 10.0 / 254.0;
        break;
    case 4:
        if (dpy) {
            Window root;
            int x, y;
            unsigned height, borderw, depth;
            XGetGeometry(dpy, DefaultRootWindow(dpy), &root,
                         &x, &y, &width, &height, &borderw, &depth);
            if (width == 0)
                width = 1024;
        }
        px = num * width / 10000.0;
        break;
    case 5:
        px = num * dpi;
        break;
    default:
        return 0;
    }

    *out = dist_round(px);
    return 1;
}

int converters_distance_px(Widget w, const char *spec)
{
    int v = 0;
    if (!converters_distance_from_display(w ? XtDisplay(w) : NULL, spec, &v))
        return 0;
    return v;
}

/* ARGSUSED */
static  Boolean
_CvtStringToDistance(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    int px;

    if (!converters_distance_from_display(dpy, (String)fromVal->addr, &px)) {
        XtStringConversionWarning(fromVal->addr, XtRDistance);
        return False;
    }

    if (toVal->addr) {
        if (toVal->size < sizeof(int))
            return False;
        *((int *)toVal->addr) = px;
    } else {
        static int static_val;
        static_val = px;
        toVal->addr = (XPointer)&static_val;
    }
    toVal->size = sizeof(int);
    return True;
}

#define done(type, value)			\
{						\
  if (toVal->addr != NULL)			\
    {						\
      if (toVal->size < sizeof(type))		\
	{					\
	  toVal->size = sizeof(type);		\
	  return (False);			\
	}					\
      *(type *)(toVal->addr) = (value);		\
    }						\
  else						\
    {						\
      static type static_val;			\
						\
      static_val = (value);			\
      toVal->addr = (XPointer)&static_val;	\
    }						\
  toVal->size = sizeof(type);			\
  return (True);				\
}

static int repl_esc_char(int ch)
{
  char tab[] = { '\\', '\0', '\n', '\r', '\'', '"', '\x1a', '\t' };
  char rep[] = { '\\', '0', 'n', 'r', '\'', '"', 'Z', 't' };
  int i;
  for(i=0;i<sizeof rep; i++ )
    if( rep[i] == ch ) return tab[i];
  return ch;
}


/* ARGSUSED */
static  Boolean
_CvtStringToArrayChar(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    String      str = (String)fromVal->addr ;
    static int  cvtval;
    int cnt;
    char *endptr;

    if( str == 0 || strlen(str) == 0 ) {
	// error:
	XtStringConversionWarning(fromVal->addr, XtRDistance);
	return False ;
    }

    int escape=0;
    int m_array = m_create(strlen(str)+1,1);
    char ch;
    while( *str ) {
      ch = *str; str++;

      if( ch == '~' ) escape=1;
      else {
        if( escape ) { ch=repl_esc_char(ch); escape=0; }
        m_put( m_array, &ch );
      }
    }
    m_putc(m_array, 0);
    done( int, m_array );
}

/* ARGSUSED */
static  Boolean
_CvtStringToArrayInt(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    String      str = (String)fromVal->addr ;

    if( str == 0 || strlen(str) == 0 ) {
    error:
	XtStringConversionWarning(fromVal->addr, XtRArrayInt);
	return False ;
    }

    int m_array = m_create(10,sizeof(int));
    char *s=str,*e;
    int val;
    while(*s) {
      while(*s && !isdigit(*s) && *s != '-') s++;
      if(!*s) break;
      val = strtol( s, &e, 0 );
      if( e == s ) break;
      m_put( m_array, &val );
      s=e;
    }

    done( int, m_array );
}

/* ARGSUSED */
static  Boolean
_CvtStringToQVar(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    String      str = (String)fromVal->addr ;

    if( str == 0 || strlen(str) == 0 ) {
    error:
	XtStringConversionWarning(fromVal->addr, XtRArrayInt);
	return False ;
    }

    int qvar = XrmStringToQuark( str );

    done( int, qvar );
}



#define string_done(value)			\
{						\
  if (toVal->addr != NULL)			\
    {						\
      if (toVal->size < size)			\
	{					\
	  toVal->size = size;			\
	  return (False);			\
	}					\
      strcpy((char *)toVal->addr, (value));	\
    }						\
  else						\
    toVal->addr = (XPointer)(value);		\
  toVal->size = size;				\
  return (True);				\
}

/* int to Anesthetic */
/* ARGSUSED */
static  Boolean
_CvtIntToAnesthetic(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
  static char *Anesthetic_names[] = { "Ag ", "Hal", "Enf", "Iso", "Des", "Sev" };
  int anesthetic_number = *(int *)fromVal->addr ;

  if( anesthetic_number > XtNumber(Anesthetic_names) ||
      anesthetic_number < 0 ) anesthetic_number = 0;

  Cardinal size = 4;
  string_done( Anesthetic_names[anesthetic_number] );
}

/* ArrayInt to String */
/* ARGSUSED */
static  Boolean
_CvtArrayIntToString(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    int i,p;
    static int buf = 0;
    if( !buf ) buf = m_create(100,1);
    int m = *(int *)fromVal->addr;
    m_clear(buf);
    p = 0;
    for(i=0;i<m_len(m);i++) {
        s_printf( buf, p, "%d ", INT(m,i) );
        p = m_len(buf)-1;
    }
    Cardinal size = m_len(buf);
    string_done( m_buf(buf) );
}

/* ArrayInt to String */
/* ARGSUSED */
static  Boolean
_CvtArrayCharToString(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    Cardinal size;
    static char *s = "ArrayCharToString conversion error";
    int m = *(int *)fromVal->addr;
    if( m ) {
        size = m_len(m);
        string_done( m_buf(m) );
    }
    else {
        size = strlen(s)+1;
        string_done( s );
    }
}

/* Distance to String */
/* ARGSUSED */
static  Boolean
_CvtDistanceToString(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    Cardinal size;
    static char *s = "DistanceToString conversion error";
    static char buf[60];
    int m = *(int *)fromVal->addr;
    size = sprintf( buf, "%d", m ) +1;
    string_done( buf );
}


/* ARGSUSED */
static  Boolean
_CvtStringToStringMArray(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    String      str = (String)fromVal->addr ;

    if( str == 0 || strlen(str) == 0 ) {
    error:
	XtStringConversionWarning(fromVal->addr, XtRString);
	return False ;
    }

    int m_array = s_split(0,str, ',', 1 );
    done( int, m_array );
}


static inline char *m2s(int m, int p)
{
    if( m <= 0 ) { WARN("List %d not found", m); return ""; }
    if( p >= m_len(m) ) {
        WARN("Item %d in List %d Size=%d not found", p, m, m_len(m)); return "";
    }
    char **s = mls(m,p);
    return *s;
}

/* ARGSUSED */
static  Boolean
_CvtStringMArrayToString(dpy, args, num_args, fromVal, toVal, data)
    Display     *dpy ;
    XrmValuePtr args;           /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *data ;
{
    static int buf_m = 0;
    if( buf_m == 0 ) buf_m = m_create(100,1);
    int strlist_m = *(int*)fromVal->addr ;
    int i;

    if( strlist_m == 0 ) {
    error:
	XtStringConversionWarning(fromVal->addr, XtRStringMArray);
	return False ;
    }

    m_clear(buf_m);
    if( m_len(strlist_m) == 0 ) m_putc(buf_m, 0);
    else {
        s_app1(buf_m, m2s(strlist_m, 0));
        for(i=1;i<m_len(strlist_m);i++) {
            s_app( buf_m, ",", m2s(strlist_m,i),NULL );
        }
    }

    Cardinal size;
    size = m_len(buf_m);
    string_done(m_buf(buf_m));
}



void converters_init(void)
{
    static Boolean first_time = True;
    if (first_time == False)
	return;
    first_time = False;

    XawInitializeWidgetSet();

    XtSetTypeConverter(XtRString, XtRDistance, _CvtStringToDistance,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRString, XtRQVar, _CvtStringToQVar,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRDistance,XtRString, _CvtDistanceToString,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRString, XtRArrayChar, _CvtStringToArrayChar,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRArrayChar,XtRString,  _CvtArrayCharToString,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRString, XtRArrayInt, _CvtStringToArrayInt,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRArrayInt,XtRString, _CvtArrayIntToString,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRInt, XtRAnesthetic, _CvtIntToAnesthetic,
		       NULL, 0, XtCacheNone, NULL);

    XtSetTypeConverter(XtRString, XtRStringArray, cvtStringToStringArray,
                       NULL, 0, XtCacheNone, NULL);


    XtSetTypeConverter(XtRString, "StringMArray", _CvtStringToStringMArray,
                       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter("StringMArray",XtRString, _CvtStringMArrayToString,
                       NULL, 0, XtCacheNone, NULL);

    converters_xft_init();
}
