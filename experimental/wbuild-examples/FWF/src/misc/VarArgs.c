#include <X11/Intrinsic.h>
#include <Xfwf/VarArgs.h>


/* This struct is taken from mit/lib/Xt/VarargsI.h */

typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg;

XtVarArgsList  XfwfCreateArgsList(var)
     va_list	var;
{
  XtTypedArg	*avlist;
  int	size=4;
  int	count=0;
  String	attr;

  avlist = (XtTypedArg *)XtMalloc(size*sizeof(XtTypedArg));

  while ( (attr=va_arg(var, String)) ) {
    XtTypedArg	*curr;
    curr = & avlist[count];
    if (0 == strcmp(attr, XtVaTypedArg)) {
      curr->name = va_arg(var, String);
      curr->type = va_arg(var, String);
      curr->value = va_arg(var, XtArgVal);
      curr->size = va_arg(var, int);
    } else {
      curr->name = attr;
      curr->type = 0;
      curr->value = va_arg(var, XtArgVal);
    }
    count++;
    if (count >=size) {
      avlist = (XtTypedArg *)XtRealloc((XtPointer)avlist, sizeof(XtTypedArg)*(size*=2));
    }
  }

  avlist[count].name = 0;

  return (XtVarArgsList)avlist;
}
