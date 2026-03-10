#ifndef M_TABLE_H
#define M_TABLE_H

#include "mls.h"
#include "m_tool.h" // For s_cstr

// Enum for types stored in the table for introspection.
typedef enum {
    MLS_TABLE_TYPE_UNKNOWN = 0, // Default or uninitialized
    MLS_TABLE_TYPE_INT,         // Value is an actual integer (stored directly in table, not a handle)
    MLS_TABLE_TYPE_STRING,      // Value is an MLS string handle (s_printf, s_strdup_c)
    MLS_TABLE_TYPE_CONST_STRING, // Value is a constant MLS string handle (s_cstr), should NOT be individually freed
    MLS_TABLE_TYPE_LIST,        // Value is an MLS generic list handle (m_alloc(..., MFREE/MFREE_EACH))
    MLS_TABLE_TYPE_TABLE,       // Value is another M_TABLE handle
    MLS_TABLE_TYPE_CUSTOM_HANDLE // Value is a handle but m_table doesn't know its specific type
} mls_table_type_t;

// --- Table Management ---
// Creates a new empty table handle.
// Returns a table handle (int) or 0 on error.
int m_table_create();

// Frees the table and its contents recursively.
// Cleans up all string handles, list handles, and nested table handles.
void m_table_free(int table_h);

// --- Setting Values ---

// Sets a raw integer value by integer key.
void m_table_set_int_val_by_int(int table_h, int key_idx, int raw_int_value);
// Sets a raw C-string value by integer key (creates a dynamic MLS string).
void m_table_set_string_by_int(int table_h, int key_idx, const char *raw_c_string);
// Sets a constant C-string value by integer key (uses s_cstr).
void m_table_set_const_string_by_int(int table_h, int key_idx, const char *raw_c_string);
// Sets an MLS list handle by integer key.
void m_table_set_list_by_int(int table_h, int key_idx, int list_h);
// Sets another MLS table handle by integer key.
void m_table_set_table_by_int(int table_h, int key_idx, int nested_table_h);
// Generic setter for int keys, for custom handles or types.
void m_table_set_handle_by_int(int table_h, int key_idx, int value_h, mls_table_type_t type);


// Sets a raw integer value by C-string key.
void m_table_set_int_val_by_cstr(int table_h, const char *key_cstr, int raw_int_value);
// Sets a raw C-string value by C-string key (creates a dynamic MLS string).
void m_table_set_string_by_cstr(int table_h, const char *key_cstr, const char *raw_c_string);
// Sets a constant C-string value by C-string key (uses s_cstr).
void m_table_set_const_string_by_cstr(int table_h, const char *key_cstr, const char *raw_c_string);
// Sets an MLS list handle by C-string key.
void m_table_set_list_by_cstr(int table_h, const char *key_cstr, int list_h);
// Sets another MLS table handle by C-string key.
void m_table_set_table_by_cstr(int table_h, const char *key_cstr, int nested_table_h);
// Generic setter for cstr keys, for custom handles or types.
void m_table_set_handle_by_cstr(int table_h, const char *key_cstr, int value_h, mls_table_type_t type);


// Sets a raw integer value by MLS string handle key.
void m_table_set_int_val_by_str(int table_h, int key_str_h, int raw_int_value);
// Sets a raw C-string value by MLS string handle key (creates a dynamic MLS string).
void m_table_set_string_by_str(int table_h, int key_str_h, const char *raw_c_string);
// Sets a constant C-string value by MLS string handle key (uses s_cstr).
void m_table_set_const_string_by_str(int table_h, int key_str_h, const char *raw_c_string);
// Sets an MLS list handle by MLS string handle key.
void m_table_set_list_by_str(int table_h, int key_str_h, int list_h);
// Sets another MLS table handle by MLS string handle key.
void m_table_set_table_by_str(int table_h, int key_str_h, int nested_table_h);
// Generic setter for str handle keys, for custom handles or types.
void m_table_set_handle_by_str(int table_h, int key_str_h, int value_h, mls_table_type_t type);


// Old generic setters (still available for advanced/flexible usage)
// 'value' is the handle to the actual data (string, list, table) or a raw int.
// 'type' specifies the type of the value for introspection.
void m_table_set_int_key(int table_h, int key_idx, int value, mls_table_type_t type);
void m_table_set_str_key(int table_h, int key_str_h, int value, mls_table_type_t type);
void m_table_set_cstr_key(int table_h, const char *key_cstr, int value, mls_table_type_t type);

// --- Getting Values ---

// Gets the value (handle or raw int) by integer key.
// Returns 0 if key not found or error.
int m_table_get_int(int table_h, int key_idx);

// Gets the value (handle or raw int) by string key handle.
// Returns 0 if key not found or error.
int m_table_get_str(int table_h, int key_str_h);

// Convenience function to get a value by C-string key.
// Returns 0 if key not found or error.
int m_table_get_cstr(int table_h, const char *key_cstr);

// --- Introspection ---

// Gets the type of the value stored at an integer key.
mls_table_type_t m_table_get_type_int(int table_h, int key_idx);

// Gets the type of the value stored at a string key handle.
mls_table_type_t m_table_get_type_str(int table_h, int key_str_h);

// Convenience function to get the type of the value stored at a C-string key.
mls_table_type_t m_table_get_type_cstr(int table_h, const char *key_cstr);


#endif // M_TABLE_H
