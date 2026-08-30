#ifndef OPTS_H
#define OPTS_H
#include <X11/Intrinsic.h>

int svar_strdup(char *s);
int append_mstring_array(int v,int offset, ... );


void set_default_active_widget(Widget w, int k_group ); 
int register_widget_group( Widget wid, char *group );
void register_widget( Widget wid, int key );
Widget change_active_widget( Widget new, int key );
Widget get_active_widget(char *group);


#endif
