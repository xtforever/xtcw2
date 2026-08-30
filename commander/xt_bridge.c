#include "lua_bridge.h"
#include "luaxt.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/ConstrainP.h>
#include <WcCreate.h>
#include <WcCreateP.h>
#include "mls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Widget name registry for Lua-created widgets */
static int widget_registry = 0;  /* List of (name, widget) pairs */

/* Register a widget by name */
static void register_widget(const char *name, Widget w) {
    if (!widget_registry) {
        widget_registry = m_create(10, sizeof(char*));
    }
    char *name_dup = strdup(name);
    m_put(widget_registry, &name_dup);
    m_put(widget_registry, &w);
}

/* Find widget by name in registry */
static Widget find_widget_in_registry(const char *name) {
    if (!widget_registry) return NULL;
    for (int i = 0; i < m_len(widget_registry); i += 2) {
        char **stored_name = (char**)mls(widget_registry, i);
        if (stored_name && *stored_name && strcmp(*stored_name, name) == 0) {
            Widget *w = (Widget*)mls(widget_registry, i + 1);
            return w ? *w : NULL;
        }
    }
    return NULL;
}

const char *luastring(lua_State *L, int i) {
    return (const char*)lua_tostring(L, i);
}

void luatable_to_strings(lua_State *L, int index, int max, int ret_str) {
    char *s;
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        s = (char*)luastring(L, -2);
        m_put(ret_str, &s);
        s = (char*)luastring(L, -1);
        m_put(ret_str, &s);
        lua_pop(L, 1);
    }
}

int app_arg_lua(lua_State *L, int index, int max, int ret_str) {
    if (index > max) return 0;
    if (lua_istable(L, index)) {
        luatable_to_strings(L, index, max, ret_str);
    } else if (lua_isstring(L, index)) {
        char *s = (char*)luastring(L, index);
        m_put(ret_str, &s);
    }
    return index + 1;
}

static void app_arg_typed(int args, const char *key, lua_State *L, int val_idx) {
    XtTypedArg *arg = m_add(args);
    arg->name = (char*)key;
    if (lua_isboolean(L, val_idx)) {
        arg->type = XtRBoolean;
        arg->value = (XtArgVal)(long)lua_toboolean(L, val_idx);
        arg->size = sizeof(Boolean);
    } else if (lua_isnumber(L, val_idx)) {
        arg->type = XtRInt;
        arg->value = (XtArgVal)(long)lua_tointeger(L, val_idx);
        arg->size = sizeof(int);
    } else {
        const char *val = lua_tostring(L, val_idx);
        arg->type = XtRString;
        arg->value = (XtArgVal)val;
        arg->size = strlen(val) + 1;
    }
}

static void luatable_to_args(int args, lua_State *L, int table_index, int max) {
    lua_pushnil(L);
    while (lua_next(L, table_index) != 0) {
        app_arg_typed(args, lua_tostring(L, -2), L, -1);
        lua_pop(L, 1);
    }
}

static int luaarg_to_args(int args, lua_State *L, int index, int max) {
    if (lua_istable(L, index)) {
        luatable_to_args(args, L, index, max);
        return 1;
    }
    if (index < max && lua_isstring(L, index)) {
        app_arg_typed(args, lua_tostring(L, index), L, index + 1);
        return 2;
    }
    return 0;
}

Widget luaarg_to_widget(lua_State *L, int index) {
    Widget w;
    if (lua_isuserdata(L, index)) {
        w = lua_touserdata(L, index);
        if (w == NULL) {
            fprintf(stderr, "first arg is empty userdata, not a widget\n");
        }
        return w;
    }
    const char *s = luastring(L, index);
    if (s == NULL) {
        fprintf(stderr, "arg %d must be string or userdata\n", index);
        return NULL;
    }
    /* First check our registry for Lua-created widgets */
    w = find_widget_in_registry(s);
    if (w != NULL) {
        return w;
    }
    /* Fall back to Wc lookup */
    w = luaxt_nametowidget((char*)s);
    if (w == NULL) {
        fprintf(stderr, "widget '%s' not found\n", s);
    }
    return w;
}

static void GetAllResourcesForChild(Widget parent, WidgetClass child_class, XtResourceList *res_list, Cardinal *number) {
    XtInitializeWidgetClass(child_class);
    XtGetResourceList(child_class, res_list, number);
    if (parent && XtIsConstraint(parent)) {
        WidgetClass parent_class = XtClass(parent);
        XtInitializeWidgetClass(parent_class);
        XtResourceList constraint = NULL;
        Cardinal num_constraint = 0;
        XtGetConstraintResourceList(parent_class, &constraint, &num_constraint);
        if (num_constraint > 0) {
            *res_list = (XtResourceList)XtRealloc((char*)*res_list,
                (Cardinal)((*number + num_constraint) * sizeof(XtResource)));
            XtResourceList res = *res_list + *number;
            for (Cardinal temp = num_constraint; temp != 0; temp--)
                *res++ = *constraint++;
            *number += num_constraint;
        }
        /* Do NOT free constraint — XtGetConstraintResourceList returns
           a pointer into the class record, not an XtMalloc'd copy */
    }
}

static int TypedArgToArg(Widget widget, XtTypedArgList typed_arg, ArgList arg_return,
                          XtResourceList resources, Cardinal num_resources, ArgList memory_return) {
    String to_type = NULL;
    XrmValue from_val, to_val;
    if (widget == NULL) {
        return 0;
    }
    for (; num_resources--; resources++) {
        if (strcmp(typed_arg->name, resources->resource_name) == 0) {
            to_type = resources->resource_type;
            break;
        }
    }
    if (to_type == NULL) {
        /* Not found in child or parent constraint resources.
           Try passing as a raw string arg — XtCreateWidget may handle
           constraint resources itself if the parent is a Constraint widget. */
        arg_return->name = typed_arg->name;
        memory_return->value = (XtArgVal)NULL;
        if (strcmp(typed_arg->type, XtRString) == 0) {
            arg_return->value = (XtArgVal)typed_arg->value;
        } else if (strcmp(typed_arg->type, XtRInt) == 0) {
            arg_return->value = (XtArgVal)typed_arg->value;
        } else if (strcmp(typed_arg->type, XtRBoolean) == 0) {
            arg_return->value = (XtArgVal)typed_arg->value;
        } else {
            /* For unknown types, try passing as string and let Xt convert */
            arg_return->value = (XtArgVal)typed_arg->value;
        }
        return 1;
    }
    to_val.addr = NULL;
    from_val.size = typed_arg->size;
    if ((strcmp(typed_arg->type, XtRString) == 0) || ((unsigned)typed_arg->size > sizeof(XtArgVal))) {
        from_val.addr = (XPointer)typed_arg->value;
    } else {
        from_val.addr = (XPointer)&typed_arg->value;
    }
    XtConvertAndStore(widget, typed_arg->type, &from_val, to_type, &to_val);
    if (to_val.addr == NULL) {
        fprintf(stderr, "Type conversion failed\n");
        return 0;
    }
    arg_return->name = typed_arg->name;
    memory_return->value = (XtArgVal)NULL;
    if (strcmp(to_type, XtRString) == 0) {
        arg_return->value = (XtArgVal)to_val.addr;
    } else {
        if (to_val.size == sizeof(long))
            arg_return->value = (XtArgVal)*(long*)to_val.addr;
        else if (to_val.size == sizeof(int))
            arg_return->value = (XtArgVal)*(int*)to_val.addr;
        else if (to_val.size == sizeof(short))
            arg_return->value = (XtArgVal)*(short*)to_val.addr;
        else if (to_val.size == sizeof(char))
            arg_return->value = (XtArgVal)*(char*)to_val.addr;
        else if (to_val.size == sizeof(XtArgVal))
            arg_return->value = *(XtArgVal*)to_val.addr;
        else if (to_val.size > sizeof(XtArgVal)) {
            arg_return->value = (XtArgVal)XtMalloc(to_val.size);
            memory_return->value = (XtArgVal)memcpy((void*)arg_return->value, to_val.addr, to_val.size);
        }
    }
    return 1;
}

void TypedArgListToArgListWithClass(Widget parent, WidgetClass child_class, XtTypedArgList typed_arg_list, int max_count,
                            ArgList *args_return, Cardinal *num_args_return) {
    int count;
    ArgList args = (ArgList)NULL;
    XtResourceList resources = (XtResourceList)NULL;
    Cardinal num_resources = 0;
    if ((max_count == 0) || (parent == NULL)) {
        *num_args_return = 0;
        *args_return = (ArgList)NULL;
        return;
    }
    GetAllResourcesForChild(parent, child_class, &resources, &num_resources);
    args = (ArgList)XtMalloc((unsigned)(max_count * 2 * sizeof(Arg)));
    for (count = max_count * 2; --count >= 0;) args[count].value = (XtArgVal)NULL;
    Cardinal success_count = 0;
    for (int i = 0; i < max_count; i++) {
        if (TypedArgToArg(parent, &typed_arg_list[i], &args[success_count], resources, num_resources, &args[max_count + success_count])) {
            success_count++;
        }
    }
    XtFree((XtPointer)resources);
    *num_args_return = success_count;
    *args_return = (ArgList)args;
}

int xtcreate_lua(lua_State *L) {
    int num_args = lua_gettop(L);
    if (num_args < 3) return 0;
    for (int i = 1; i <= num_args; i++) {
        const char *s = luastring(L, i);
        if (s == NULL) {
            fprintf(stderr, "argument %d must be a string\n", i);
            goto fail;
        }
    }
    const char *name = luastring(L, 1);
    const char *class_name = luastring(L, 2);
    Widget parent = luaarg_to_widget(L, 3);
    if (parent == NULL) {
        goto fail;
    }
    XrmQuark class_quark = WcStringToQuark((char*)class_name);
    XtAppContext app = XtWidgetToApplicationContext(parent);
    WidgetClass class = WcMapClassFind(app, (intptr_t)class_quark);
    if (class == NULL) {
        goto fail;
    }
    int pairs = (num_args - 3) / 2;
    int arg_list = (pairs > 0) ? m_create(pairs, sizeof(XtTypedArg)) : 0;
    int i = 4;
    int p_count = pairs;
    while (p_count-- > 0) {
        XtTypedArg *arg = m_add(arg_list);
        arg->name = (char*)luastring(L, i++);
        const char *s = luastring(L, i++);
        arg->type = XtRString;
        arg->value = (XtArgVal)s;
        arg->size = strlen(s) + 1;
    }
    Widget w;
    ArgList args = NULL;
    Cardinal num_converted = 0;
    int memory_count = 0;
    if (arg_list && m_len(arg_list) > 0) {
        ArgList conv_args;
        Cardinal conv_num;
        TypedArgListToArgListWithClass(parent, class, m_buf(arg_list), m_len(arg_list), &conv_args, &conv_num);
        if (conv_num > 0) {
            args = conv_args;
            num_converted = conv_num;
            memory_count = conv_num;
        } else if (conv_args) {
            XtFree((char*)conv_args);
        }
    }
    w = XtCreateWidget((char*)name, class, parent, args, num_converted);
    if (args) {
        ArgList mem_args = args + memory_count;
        for (Cardinal i = 0; i < memory_count; i++) {
            if (mem_args[i].value != 0)
                XtFree((char*)mem_args[i].value);
        }
        XtFree((char*)args);
    }
    if (w == NULL) {
        if (arg_list) m_free(arg_list);
        goto fail;
    }
    if (arg_list) m_free(arg_list);
    register_widget(name, w);
    XtManageChild(w);
    lua_pushlightuserdata(L, w);
    return 1;
fail:
    lua_pushnil(L);
    return 0;
}

int xtsetvalue_lua(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    if (!w) return 0;
    int nargs = lua_gettop(L);
    int pairs = (nargs - 1) / 2;
    if (pairs < 1) return 0;
    int targ_list = m_create(pairs, sizeof(XtTypedArg));
    int argi = 2;
    while (argi <= nargs) {
        int cnt = luaarg_to_args(targ_list, L, argi, nargs);
        if (cnt == 0) {
            fprintf(stderr, "cannot parse argument %d\n", argi);
            lua_pushnil(L);
            goto cleanup;
        }
        argi += cnt;
    }
    ArgList args;
    Cardinal max_args;
    Widget parent = XtParent(w);
    TypedArgListToArgListWithClass(parent, XtClass(w), m_buf(targ_list), m_len(targ_list), &args, &max_args);
    XtSetValues(w, args, max_args);
cleanup:
    m_free(targ_list);
    return 0;
}

int xtgetvalue_lua(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    const char *res = luastring(L, 2);
    if (!w || !res) return 0;
    String res_type = WcGetResourceType(w, (char*)res);
    if (res_type == NULL) {
        lua_pushnil(L);
        return 1;
    }
    if (strcmp(res_type, XtRInt) == 0 ||
        strcmp(res_type, XtRShort) == 0 ||
        strcmp(res_type, XtRDimension) == 0 ||
        strcmp(res_type, XtRPosition) == 0 || strcmp(res_type, XtRCardinal) == 0) {
        int val;
        XtVaGetValues(w, res, &val, NULL);
        lua_pushinteger(L, val);
    } else if (strcmp(res_type, XtRBoolean) == 0) {
        Boolean val;
        XtVaGetValues(w, res, &val, NULL);
        lua_pushboolean(L, val);
    } else {
        char *val = NULL;
        XtVaGetValues(w, res, &val, NULL);
        if (val) lua_pushstring(L, val);
        else lua_pushnil(L);
    }
    return 1;
}

int xtaction_lua(lua_State *L) {
    Widget w = NULL;
    int nargs = lua_gettop(L);
    int str = m_create(10, sizeof(char*));
    if (nargs < 2 || !(w = luaarg_to_widget(L, 1))) {
        fprintf(stderr, "Syntax Error: xtaction(widget,action,args,...)\n");
        goto fail;
    }
    const char *action = luastring(L, 2);
    int i = 3;
    while ((i = app_arg_lua(L, i, nargs, str)));
    XtCallActionProc(w, action, NULL, m_buf(str), m_len(str));
fail:
    m_free(str);
    return 0;
}

int xtmanage_lua(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    if (w) XtManageChild(w);
    return 0;
}

int xtunmanage_lua(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    if (w) XtUnmanageChild(w);
    return 0;
}

int xtdestroy_lua(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    if (w) XtDestroyWidget(w);
    return 0;
}

int mls_create_lua(lua_State *L) {
    int size = luaL_checkinteger(L, 1);
    int width = luaL_checkinteger(L, 2);
    lua_pushinteger(L, m_create(size, width));
    return 1;
}

int mls_put_string_lua(lua_State *L) {
    int handle = luaL_checkinteger(L, 1);
    const char *s = luaL_checkstring(L, 2);
    char *dup = strdup(s);
    m_put(handle, &dup);
    return 0;
}

int mls_clear_lua(lua_State *L) {
    int handle = luaL_checkinteger(L, 1);
    m_clear(handle);
    return 0;
}

int mls_len_lua(lua_State *L) {
    int handle = luaL_checkinteger(L, 1);
    lua_pushinteger(L, m_len(handle));
    return 1;
}

int mls_get_string_lua(lua_State *L) {
    int handle = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index >= m_len(handle)) return 0;
    char **s = (char **)mls(handle, index);
    if (s && *s) lua_pushstring(L, *s);
    else lua_pushnil(L);
    return 1;
}

static void timeout_callback(XtPointer client_data, XtIntervalId *id) {
    typedef struct { lua_State *L; int ref; } timeout_data_t;
    timeout_data_t *td = (timeout_data_t *)client_data;
    lua_State *L = td->L;
    int ref = td->ref;
    free(td);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "xtapptimeout callback error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

int xtapptimeout_lua(lua_State *L) {
    extern XtAppContext LUAXT_APP;
    int ms = luaL_checkinteger(L, 1);
    if (lua_type(L, 2) != LUA_TFUNCTION) {
        fprintf(stderr, "xtapptimeout: second arg must be function\n");
        lua_pushnil(L);
        return 1;
    }
    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    typedef struct { lua_State *L; int ref; } timeout_data_t;
    timeout_data_t *td = malloc(sizeof(timeout_data_t));
    td->L = L;
    td->ref = ref;
    XtAppAddTimeOut(LUAXT_APP, ms, (XtTimerCallbackProc)timeout_callback, (XtPointer)td);
    lua_pushinteger(L, ref);
    return 1;
}

int xtgeometry_lua(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    if (!w) {
        lua_pushnil(L);
        return 1;
    }
    Position x = 0, y = 0;
    Dimension width = 0, height = 0;
    XtVaGetValues(w,
        XtNx, &x,
        XtNy, &y,
        XtNwidth, &width,
        XtNheight, &height,
        NULL);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "x");
    lua_pushinteger(L, (lua_Integer)x);
    lua_rawset(L, -3);
    lua_pushstring(L, "y");
    lua_pushinteger(L, (lua_Integer)y);
    lua_rawset(L, -3);
    lua_pushstring(L, "width");
    lua_pushinteger(L, (lua_Integer)width);
    lua_rawset(L, -3);
    lua_pushstring(L, "height");
    lua_pushinteger(L, (lua_Integer)height);
    lua_rawset(L, -3);
    return 1;
}

static int x11_error_count = 0;

static int x11_error_handler(Display *display, XErrorEvent *event) {
    x11_error_count++;
    char buf[256];
    XGetErrorText(display, event->error_code, buf, sizeof(buf));
    fprintf(stderr, "X11 Error: %s (request=%d, minor=%d, resource=%lu)\n",
        buf, event->request_code, event->minor_code, (unsigned long)event->resourceid);
    return 0;
}

int xterror_count_lua(lua_State *L) {
    int nargs = lua_gettop(L);
    if (nargs >= 1 && lua_isboolean(L, 1) && lua_toboolean(L, 1)) {
        x11_error_count = 0;
        XSetErrorHandler(x11_error_handler);
    }
    lua_pushinteger(L, (lua_Integer)x11_error_count);
    return 1;
}
