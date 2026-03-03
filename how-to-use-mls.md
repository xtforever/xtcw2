# MLS Library: Production Usage Guide

The `mls` (Memory-managed List System) provides a handle-based approach to dynamic arrays and string management in C. It focuses on memory safety, ease of use, and efficient string interning.

## 1. Initialization and Cleanup

Before using any `mls` functions, you must initialize the global state with `m_init()`. Similarly, you should clean up before your application exits with `m_destruct()`.

```c
#include "mls.h"
#include "m_tool.h"

int main() {
    m_init();           // Initialize the MLS pool
    conststr_init();    // Initialize the constant string table (interning)

    // ... your code ...

    conststr_free();    // Cleanup string table
    m_destruct();       // Final cleanup of the MLS pool
    return 0;
}
```

## 2. Managing Dynamic Lists

Lists in MLS are identified by an integer **handle**.

### Creating a List
`m_create(int initial_max, int element_width)` creates a new list.
* `initial_max`: Initial capacity.
* `element_width`: Size of one element (e.g., `sizeof(int)` or `sizeof(MyStruct)`).

```c
int my_list = m_create(10, sizeof(double));
```

### Adding Elements
`m_put(int handle, const void *data)` appends a copy of the data to the end of the list, growing it if necessary.

```c
double val = 3.14;
m_put(my_list, &val);
```

### Accessing Elements
`mls(int handle, int index)` returns a `void*` pointer to the element at the specified index. You must cast this to your data type.

```c
double *ptr = (double*)mls(my_list, 0);
printf("Value: %f\n", *ptr);
```

### Metadata
* `m_len(int handle)`: Returns the current number of elements in the list.
* `m_width(int handle)`: Returns the `element_width` specified during creation.

### Iteration
Use the `m_foreach` macro for safe and efficient iteration.

```c
int p;          // Index variable
double *val;    // Pointer to element
m_foreach(my_list, p, val) {
    printf("Index %d: %f\n", p, *val);
}
```

**Note on Handles**: If your list contains other MLS handles (e.g., `sizeof(int)` where each int is a handle), iterate using an `int *`:
```c
int p; int *h;
m_foreach(handle_list, p, h) {
    m_free(*h); // Example: freeing sub-lists
}
```

### Freeing a List
* `m_free(int handle)`: Releases the memory associated with the list.
* `m_free_list(int handle)`: Recursively frees elements if they are handles (only if `element_width == sizeof(int)`).

```c
m_free(my_list);
```

## 3. Random Access and Arrays

If you want to use a list as a fixed-size array or need to write to specific indices immediately, use `m_setlen`.

`m_setlen(int handle, int len)` sets the current length of the list and pre-allocates the memory.

```c
int array = m_create(100, sizeof(int));
m_setlen(array, 100); // Now indices 0-99 are valid for mls()

int *item = (int*)mls(array, 50);
*item = 42;
```


## 4. String Management

MLS treats strings as lists of `char` (width 1).

### Creation and Printing
- `s_printf(int handle, int offset, const char *format, ...)`: 
    - If `handle` is 0: creates a new formatted string.
    - If `handle` is not 0: overwrites/appends to the string referenced by `handle` beginning at `offset` (-1 to append).
- `m_str(int handle)`: Returns a standard `const char*` pointer to the string data. **NEVER** free this pointer directly.

```c
int s = s_printf(0, 0, "Hello %s", "World");
printf("%s\n", m_str(s));
s_printf(s, -1, "!"); // Append
```

### Appending and Conversion
- `s_app(int handle, const char *s, ...)`: Appends multiple C-strings to an MLS string. Must end with `NULL`.
- `s_strdup_c(const char *s)`: Creates a new MLS string handle from a C-string. 


### Interning (Constant Strings)
- do not use constant string functions s_cstr, s_mstr, cs_printf
- constant strings look like normal string handles but create errors if freed
- constant strings must never be used with m_free (except in conststr_free where all strings are freed)
- to avoid confusion better not use constant strings

## 5. Debugging and Safety

### MLS_DEBUG
When compiling for development, add `-DMLS_DEBUG` to your `CFLAGS`. This enables:
- **Boundary checking**: `mls()` will verify the index is within bounds.
- **UAF Protection**: Detects "Use After Free" errors.
- **Leak Detection**: `m_destruct()` will print warnings if lists were not freed.

### Production
For production builds, omit `-DMLS_DEBUG` and use `-O3`. The `mls` functions will then expand to high-performance versions with minimal overhead.

```bash
# Debug build (example)
gcc -DMLS_DEBUG -I./utils -o my_app main.c utils/mls.c utils/m_tool.c

# Production build
gcc -O3 -I./utils -o my_app main.c utils/mls.c utils/m_tool.c
```
