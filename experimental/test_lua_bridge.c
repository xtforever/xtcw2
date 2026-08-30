#include "lua_bridge.h"
#include "mls.h"
#include <stdio.h>
#include <stdlib.h>

XtAppContext LUAXT_APP;
Widget TopLevel;

int main(int argc, char **argv) {
    printf("Testing lua_bridge_init...\n");
    
    m_init();
    
    XtAppContext app;
    TopLevel = XtAppInitialize(&app, "TestApp", NULL, 0, &argc, argv, NULL, NULL, 0);
    LUAXT_APP = app;
    
    lua_bridge_init(app);
    
    if (L_GLOBAL == NULL) {
        fprintf(stderr, "FAIL: L_GLOBAL is NULL after init\n");
        return 1;
    }
    
    printf("PASS: L_GLOBAL initialized\n");
    
    lua_bridge_destroy();
    XtDestroyWidget(TopLevel);
    
    return 0;
}
