#ifndef XSGAME_H
#define XSGAME_H

#include <X11/Intrinsic.h>

void xsg_init(Widget w);
void xsg_free(void);
void xsg_click(int x, int y);
void xsg_update();
void xsg_render(Drawable d);

#endif
