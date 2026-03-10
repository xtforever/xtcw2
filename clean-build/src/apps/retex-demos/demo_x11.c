#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "backend_xpixmap.h"
#include "renderer.h"
#include "node.h"
#include "builder.h"
#include "token.h"
#include "mls.h"
#include "m_tool.h"

/* Workaround for redefinition error in conststr.h vs m_tool.h */
#define s_cstr s_cstr_hidden
#define s_mstr s_mstr_hidden
#include "conststr.h"
#undef s_cstr
#undef s_mstr

static CharMetricsScaled measure_char(void *ctx, int c, Scaled font_size, int style, const char *face) {
    Backend *be = (Backend*)ctx;
    be->set_font_face(be, face, style);
    be->set_font_size(be, TO_DOUBLE(font_size));
    CharMetrics m;
    be->get_char_metrics(be, c, &m);
    
    CharMetricsScaled ms;
    ms.width = FROM_DOUBLE(m.width);
    ms.height = FROM_DOUBLE(m.height);
    ms.depth = FROM_DOUBLE(m.depth);
    return ms;
}

int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    // Create a simple window (larger for 80pt text)
    int win_w = 1200;
    int win_h = 300;
    Window win = XCreateSimpleWindow(dpy, root, 10, 10, win_w, win_h, 1, 
                                     BlackPixel(dpy, screen), WhitePixel(dpy, screen));

    XSelectInput(dpy, win, ExposureMask | KeyPressMask);
    XMapWindow(dpy, win);

    // Backend creation needed early for measuring, but it needs window?
    // XPixmap backend creates a Pixmap. We can create it now.
    Backend *be = backend_xpixmap_create(dpy, win, win_w, win_h);
    if (!be) {
        fprintf(stderr, "Failed to create XPixmap backend\n");
        return 1;
    }
    be->set_font_size(be, 80.0);
    be->set_font_face(be, "Monospace", 0); // Consistent with demo_png

    // Initialize re-tex
    m_init();
    conststr_init();

    printf("Building node list...\n");
    int tokens = tokenize("Hello X11 World!");
    // Use 80pt font (8x standard 10pt)
    int face_handle = conststr_lookup_c("Monospace");
    int node_list = build_hlist(tokens, measure_char, be, FROM_INT(80), "Monospace", face_handle);

    int box_handle_list = m_create(1, sizeof(Node));
    node_create_hbox(box_handle_list, node_list);
    Node *box = (Node*)mls(box_handle_list, 0);
    box->width = FROM_INT(1000); 
    box->height = FROM_INT(100);

    // Render to Pixmap
    printf("Rendering to Pixmap...\n");
    be->draw_rect(be, 0, 0, win_w, win_h, 0xFFFFFFFF); // Clear bg
    be->draw_rect(be, 20, 200, 1100, 2, 0xFF0000FF);    // Blue baseline (lower down)
    renderer_render(be, box_handle_list, 20.0, 200.0);

    // Get the pixmap handle
    Pixmap px = backend_xpixmap_get_pixmap(be);

    // Event loop
    XEvent e;
    int running = 1;
    GC gc = XCreateGC(dpy, win, 0, NULL);

    printf("Window created. Press any key to exit.\n");

    while (running) {
        XNextEvent(dpy, &e);
        if (e.type == Expose) {
            // Copy Pixmap to Window
            XCopyArea(dpy, px, win, gc, 0, 0, win_w, win_h, 0, 0);
        }
        if (e.type == KeyPress) {
            running = 0;
        }
    }

    // Cleanup
    XFreeGC(dpy, gc);
    be->destroy(be); // This destroys the Pixmap too!
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    conststr_free();
    m_destruct();

    return 0;
}
