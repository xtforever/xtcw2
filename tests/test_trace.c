#include <stdio.h>
#include "mls.h"

int main() {
    m_init();
    // Use trace_level defined in mls.c
    extern int trace_level;
    trace_level = 2;
    
    TRACE(2, "LAYOUT handle %d, class %s, name %s, width %d, height %d, x %d, y %d, color %08x, content %s", 
          1, "TestWidget", "mytest", 100, 50, 0, 0, 0xFFFFFFFF, "Hello world");
    
    m_destruct();
    printf("Trace test completed\n");
    return 0;
}
