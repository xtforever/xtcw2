#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>

int main() {
        Display *dis;
        dis = XOpenDisplay(NULL);
        KeyCode modcode = 0; //init value
        int i;
        for (i=0;i<5;i++) {
                modcode = XKeysymToKeycode(dis, XStringToKeysym("a"));
                XTestFakeKeyEvent(dis, modcode, False, 0);
                XFlush(dis);
                sleep(1);
                XTestFakeKeyEvent(dis, modcode, True, 0);
                XFlush(dis);
                XTestFakeKeyEvent(dis, modcode, False, 0);
                XFlush(dis);
        }
        return 0;
}
