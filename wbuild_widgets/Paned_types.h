#ifndef PANED_TYPES_H
#define PANED_TYPES_H

typedef enum { UpLeftPane = 'U', LowRightPane = 'L', ThisBorderOnly = 'T', AnyPane = 'A' } Direction;

typedef struct _PaneStack {
    struct _PaneStack *next;
    void *pane;
    int start_size;
} PaneStack;

#endif
