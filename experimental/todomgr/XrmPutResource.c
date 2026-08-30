#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xmu/SysUtil.h>


int main(int argc, char **argv)
{
    if( argc != 3 ) return 1;
    char *p = argv[1];
    char *v = argv[2];
    Display *dsp = XOpenDisplay("");
    if(!dsp) return 2;
    XrmDatabase db = XrmGetDatabase(dsp);
    if(!db) XGetDefault(dsp, "", "" );
    db = XrmGetDatabase(dsp);

    XrmPutStringResource(&db, p, v );

    XrmValue value;
    char *type_ret=0;
    XrmGetResource( db, p, "String", &type_ret, &value);
    puts( value.addr );

    
    char *new = "cwri.testpfad:17";
    int mode = PropModeReplace;
    size_t max = ((unsigned) XMaxRequestSize(dpy) << 2) - 28;
    if( max > strlen(new)) max=strlen(new)+1;

    XChangeProperty(dpy, RootWindow(dpy,0), XA_RESOURCE_MANAGER, XA_STRING, 8, mode, new, (int) max);
    
    XCloseDisplay(dsp);
    return 0;
}
