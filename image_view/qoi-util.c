#include "qoi-util.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"


void* qoi_load_image(  char *fn, int *w, int *h ) 
{
    qoi_desc desc;
    void *rgba_pixels = qoi_read(fn, &desc, 4);
    *w = desc.width;
    *h = desc.height;
    return rgba_pixels;
}
