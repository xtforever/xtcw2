# MLS Library Reference

The MLS (Memory-managed List System) provides handle-based dynamic arrays and string management in C.

## Initialization

```c
#include "mls.h"
#include "m_tool.h"

int main() {
    m_init();           // Initialize the MLS pool
    conststr_init();    // Initialize constant string table

    // ... your code ...

    conststr_free();    // Cleanup string table
    m_destruct();       // Final cleanup of the MLS pool
    return 0;
}
```

**Critical**: Always call `m_init()` at program start and `m_destruct()` at exit. When compiled with `-DMLS_DEBUG`, header `mls.h` redefines `m_init()` to `_m_init()` which adds tracking. The library and application **must** use the same `MLS_DEBUG` setting.

## Lists

### Creation

```c
int my_list = m_create(10, sizeof(double));  // initial_max=10, element_width=8
```

### Adding Elements

```c
double val = 3.14;
m_put(my_list, &val);  // append a copy
```

### Accessing Elements

```c
double *ptr = (double*)mls(my_list, 0);  // returns void*
printf("Value: %f\n", *ptr);
```

### Metadata

```c
int count = m_len(my_list);      // number of elements
int width = m_width(my_list);   // sizeof(element)
```

### Iteration

```c
int p;
double *val;
m_foreach(my_list, p, val) {
    printf("Index %d: %f\n", p, *val);
}
```

### Fixed-Size Arrays

```c
int array = m_create(100, sizeof(int));
m_setlen(array, 100);  // indices 0-99 now valid
int *item = (int*)mls(array, 50);
*item = 42;
```

### Freeing

```c
m_free(my_list);           // release list memory
m_free_list(my_list);     // recursively free if elements are handles (width==sizeof(int))
```

## String Management

### Creation and Formatting

```c
int s = s_printf(0, 0, "Hello %s", "World");  // handle=0 creates new
printf("%s\n", m_str(s));                       // get const char*
s_printf(s, -1, "!");                           // -1 = append
```

### Appending

```c
s_app(handle, "str1", "str2", NULL);  // append multiple C-strings, must end with NULL
```

### Duplicating

```c
int copy = s_strdup_c("original string");  // create MLS string from C-string
```

### Warning: Constant Strings

Do NOT use constant string functions (`s_cstr`, `s_mstr`, `cs_printf`). Constant strings look like normal handles but cause errors if freed. They must never be passed to `m_free()`. To avoid confusion, prefer regular string functions.

## Debugging

### Compile with -DMLS_DEBUG

Enables:
- **Boundary checking**: `mls()` verifies index is in bounds
- **Use-after-free detection**: Detects freed list handles
- **Leak detection**: `m_destruct()` prints warnings for unfreed lists

```bash
# Debug build
gcc -DMLS_DEBUG -I./utils -o my_app main.c utils/mls.c utils/m_tool.c

# Production build (no debug overhead)
gcc -O3 -I./utils -o my_app main.c utils/mls.c utils/m_tool.c
```

### Leak Detection Output

When `m_destruct()` finds unfreed lists, it prints:
```
WARNING: mls.c Line: 1163 Function: _m_destruct
List 3 still allocated. Source: task_manager_init() in task_manager.c:53
```

This is expected for global lists (e.g., Lua callback queue, task manager lists) that are never freed during normal application lifetime.