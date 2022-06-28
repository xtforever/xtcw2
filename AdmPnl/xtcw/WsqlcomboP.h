/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_WSQLCOMBOP_H
#define XTCW_WSQLCOMBOP_H
#include <xtcw/WcomboP.h>
#include <xtcw/Wsqlcombo.h>
_XFUNCPROTOBEGIN

typedef struct {
/* methods */
/* class variables */
int dummy;
} WsqlcomboClassPart;

typedef struct _WsqlcomboClassRec {
CoreClassPart core_class;
WheelClassPart wheel_class;
WlabelClassPart wlabel_class;
WcomboClassPart wcombo_class;
WsqlcomboClassPart wsqlcombo_class;
} WsqlcomboClassRec;

typedef struct {
/* resources */
XtCallbackList  sqlcombo_cb;
String  sql_query_str;
String  sql_disp_field;
String  sql_key_field;
ArrayChar  sql_result;
String  sql_host;
String  sql_user;
String  sql_pwd;
String  sql_db;
XtCallbackList  onEnter;
/* private state */
int  sqlres;
int  disp_lst;
int  key_lst;
int  var5_res;
} WsqlcomboPart;

typedef struct _WsqlcomboRec {
CorePart core;
WheelPart wheel;
WlabelPart wlabel;
WcomboPart wcombo;
WsqlcomboPart wsqlcombo;
} WsqlcomboRec;

externalref WsqlcomboClassRec wsqlcomboClassRec;

_XFUNCPROTOEND
#endif /* XTCW_WSQLCOMBOP_H */
