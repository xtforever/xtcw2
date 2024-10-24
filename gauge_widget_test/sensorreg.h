#ifndef SENSORREG_H
#define SENSORREG_H

#include <X11/Intrinsic.h>

void sensor_register_c(Widget w, const char *s);
void sensor_register(Widget w, int name );

extern int SENSOR_LIST;
struct sensor_reg {
	int name; /* type conststr */
	Widget w;
	void (*gather)(struct sensor_reg*);
	void (*format)(struct sensor_reg*);
};


#endif
