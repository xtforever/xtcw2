#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "backend_xpixmap.h"
#include "renderer.h"
#include "node.h"
#include "builder.h"
#include "linebreak.h"
#include "token.h"
#include "mls.h"
#include "m_tool.h"

/* Workaround for redefinition error in conststr.h vs m_tool.h */
#define s_cstr s_cstr_hidden
#define s_mstr s_mstr_hidden
#include "conststr.h"
#undef s_cstr
#undef s_mstr

// Callback for builder to measure chars using backend
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
    
    // Create a window
    int win_w = 1200;
    int win_h = 800;
    Window win = XCreateSimpleWindow(dpy, root, 10, 10, win_w, win_h, 1, 
                                     BlackPixel(dpy, screen), WhitePixel(dpy, screen));
    
    XSelectInput(dpy, win, ExposureMask | KeyPressMask);
    XMapWindow(dpy, win);
    
    // Initialize re-tex
    m_init();
    conststr_init();

    // Setup Backend
    Backend *be = backend_xpixmap_create(dpy, win, win_w, win_h);
    if (!be) {
        fprintf(stderr, "Failed to create XPixmap backend\n");
        return 1;
    }
    
    // Use a Serif font, size 24pt
    be->set_font_face(be, "Serif", 0);
    be->set_font_size(be, 24.0);

    const char *text = 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
        "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
        "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
        "culpa qui officia deserunt mollit anim id est laborum.";

    printf("Tokenizing text...\n");
    int tokens = tokenize(text);
    
    printf("Building node list...\n");
    // Build with 24pt
    int face_handle = conststr_lookup_c("Serif");
    int hlist = build_hlist(tokens, measure_char, be, FROM_INT(24), "Serif", face_handle);
    
    // Add parfillskip
    Glue parfill = glue_create(0, FROM_INT(1000), ORDER_FIL, 0, ORDER_NORMAL);
    node_create_glue(hlist, parfill);
    
    printf("Breaking lines...\n");
    // Target width: 1000pt (leaving margins)
    Glue zero = glue_zero();
    int vbox_lines = line_break(hlist, FROM_INT(1000), zero, zero, 0, 0);
    
    // Wrap the lines in a VBOX
    int vbox_handle = m_create(1, sizeof(Node));
    node_create_vbox(vbox_handle, vbox_lines);
    Node *vbox = (Node*)mls(vbox_handle, 0);
    vbox->width = FROM_INT(1000);

    // Render to Pixmap
    printf("Rendering to Pixmap...\n");
    be->draw_rect(be, 0, 0, win_w, win_h, 0xFFFFFFFF); // Clear bg
    
    // Render at (50, 50)
    renderer_render(be, vbox_handle, 50.0, 50.0);

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
    be->destroy(be);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    conststr_free();
    m_destruct();

    return 0;
}
