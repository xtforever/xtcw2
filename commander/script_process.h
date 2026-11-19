#ifndef SCRIPT_PROCESS_H
#define SCRIPT_PROCESS_H

#include <lua.h>

void script_process_register(lua_State *L);
void script_process_cleanup(void);

#endif
