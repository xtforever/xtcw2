#include "m_table.h"
#include "mls.h"
#include "m_tool.h"
#include <string.h>
#include <stdio.h> // For TRACE


// --- Internal Data Structures ---

// A single entry in the table
typedef struct m_table_entry {
    int key;          // If is_str_key, this is a handle to the string. Else, it's the raw int key.
    int value;        // Handle to actual data (string, list, table) or a raw int.
    mls_table_type_t type; // Type of the 'value'.
    mls_table_type_t key_type; // Type of the 'key' (only relevant if is_str_key = 1).
    unsigned char is_str_key; // 1 if key is a string handle, 0 if int.
} m_table_entry_t;


// --- Internal Helpers ---

// Custom free handler for the table itself
static void m_table_free_handler(int table_h) {
    m_table_entry_t *entry;
    int p;
    TRACE(1, "m_table_free_handler called for table %d", table_h);
    m_foreach(table_h, p, entry) {
        // Free the value if it's a handle type that table owns and is NOT a constant string
        if ((entry->type == MLS_TABLE_TYPE_STRING ||
             entry->type == MLS_TABLE_TYPE_LIST ||
             entry->type == MLS_TABLE_TYPE_TABLE ||
             entry->type == MLS_TABLE_TYPE_CUSTOM_HANDLE) && entry->value > 0) {
            m_free(entry->value);
        }
        // Free the key if it's a string handle AND it's a dynamic string
        if (entry->is_str_key && entry->key > 0 && entry->key_type == MLS_TABLE_TYPE_STRING) {
            m_free(entry->key); 
        }
    }
    // Do NOT call m_free_simple(table_h); here. The m_free function will do that.
}

// Global variable for the custom free handler ID
static int MFREE_TABLE_ENTRIES_HDLR = 0;


// --- Table Management ---

int m_table_create() {
    if (MFREE_TABLE_ENTRIES_HDLR == 0) {
        MFREE_TABLE_ENTRIES_HDLR = m_reg_freefn(0, m_table_free_handler);
    }
    // A table is an MLS list of m_table_entry_t structs
    return m_alloc(0, sizeof(m_table_entry_t), MFREE_TABLE_ENTRIES_HDLR);
}

void m_table_free(int table_h) {
    m_free(table_h);
}

// --- Internal Lookup Helpers ---
// Returns a pointer to the entry, or NULL if not found
static m_table_entry_t* m_table_find_int_key_entry(int table_h, int key_idx) {
    m_table_entry_t *entry;
    int p;
    m_foreach(table_h, p, entry) {
        if (!entry->is_str_key && entry->key == key_idx) {
            return entry;
        }
    }
    return NULL;
}

static m_table_entry_t* m_table_find_str_key_entry(int table_h, int key_str_h) {
    m_table_entry_t *entry;
    int p;
    m_foreach(table_h, p, entry) {
        if (entry->is_str_key && entry->key == key_str_h) {
            return entry;
        }
    }
    return NULL;
}

static m_table_entry_t* m_table_find_cstr_key_entry(int table_h, const char *key_cstr) {
    m_table_entry_t *entry;
    int p;
    m_foreach(table_h, p, entry) {
        if (entry->is_str_key && strcmp(m_str(entry->key), key_cstr) == 0) {
            return entry;
        }
    }
    return NULL;
}


// --- Generic Setters (Old API, renamed for clarity) ---

void m_table_set_int_key(int table_h, int key_idx, int value, mls_table_type_t type) {
    if (table_h <= 0) { ERR("Invalid table handle %d", table_h); return; }

    m_table_entry_t *entry = m_table_find_int_key_entry(table_h, key_idx);
    if (entry) {
        // Entry exists, free old value if it was a handle type and not a constant string
        if ((entry->type == MLS_TABLE_TYPE_STRING || entry->type == MLS_TABLE_TYPE_LIST || entry->type == MLS_TABLE_TYPE_TABLE || entry->type == MLS_TABLE_TYPE_CUSTOM_HANDLE) && entry->value > 0) {
            m_free(entry->value);
        }
        // Update existing entry
        entry->value = value;
        entry->type = type;
    } else {
        // Add new entry
        m_table_entry_t new_entry = {
            .key = key_idx,
            .value = value,
            .type = type,
            .is_str_key = 0,
            .key_type = MLS_TABLE_TYPE_UNKNOWN // Not applicable for int key
        };
        m_put(table_h, &new_entry);
    }
}

void m_table_set_str_key(int table_h, int key_str_h, int value, mls_table_type_t type) {
    if (table_h <= 0) { ERR("Invalid table handle %d", table_h); return; }
    if (key_str_h <= 0) { ERR("Invalid key string handle %d", key_str_h); return; }

    // First, find if an entry with this key's *content* exists
    m_table_entry_t *entry = m_table_find_str_key_entry(table_h, key_str_h); 
    if (entry) {
        // Entry exists, free old value if it was a handle type and not a constant string
        if ((entry->type == MLS_TABLE_TYPE_STRING || entry->type == MLS_TABLE_TYPE_LIST || entry->type == MLS_TABLE_TYPE_TABLE || entry->type == MLS_TABLE_TYPE_CUSTOM_HANDLE) && entry->value > 0) {
            m_free(entry->value);
        }
        // Free old key handle (only if it was a dynamic string that the table owns, and it's being replaced)
        if (entry->key_type == MLS_TABLE_TYPE_STRING && entry->key > 0 && entry->key != key_str_h) {
            m_free(entry->key);
        }
        // Update existing entry
        entry->key = key_str_h; // Key ownership transfers here for new key_str_h.
        entry->key_type = (type == MLS_TABLE_TYPE_CONST_STRING) ? MLS_TABLE_TYPE_CONST_STRING : MLS_TABLE_TYPE_STRING; // Assume string handles are dynamic unless explicitly constant
        entry->value = value;
        entry->type = type;
    } else {
        // Add new entry, key_str_h ownership transfers here.
        m_table_entry_t new_entry = {
            .key = key_str_h, // Store the string handle
            .value = value,
            .type = type,
            .is_str_key = 1,
            .key_type = (type == MLS_TABLE_TYPE_CONST_STRING) ? MLS_TABLE_TYPE_CONST_STRING : MLS_TABLE_TYPE_STRING
        };
        m_put(table_h, &new_entry);
    }
}

void m_table_set_cstr_key(int table_h, const char *key_cstr, int value, mls_table_type_t type) {
    if (table_h <= 0) { ERR("Invalid table handle %d", table_h); return; }
    if (!key_cstr) { ERR("Null key_cstr", key_cstr); return; }

    m_table_entry_t *entry = m_table_find_cstr_key_entry(table_h, key_cstr);
    if (entry) {
        // Key exists, update its value.
        // Free old value if it was a handle type and not a constant string
        if ((entry->type == MLS_TABLE_TYPE_STRING || entry->type == MLS_TABLE_TYPE_LIST || entry->type == MLS_TABLE_TYPE_TABLE || entry->type == MLS_TABLE_TYPE_CUSTOM_HANDLE) && entry->value > 0) {
            m_free(entry->value);
        }
        entry->value = value;
        entry->type = type;
    } else {
        // Key does not exist, create a new entry.
        // Duplicate the C-string into an MLS string, ownership transfers to table.
        int new_key_h = s_strdup_c(key_cstr); 
        m_table_entry_t new_entry = {
            .key = new_key_h, 
            .value = value,
            .type = type,
            .is_str_key = 1,
            .key_type = MLS_TABLE_TYPE_STRING // keys from s_strdup_c are always dynamic
        };
        m_put(table_h, &new_entry);
    }
}


// --- New Simplified Setters ---

// By int key
void m_table_set_int_val_by_int(int table_h, int key_idx, int raw_int_value) {
    m_table_set_int_key(table_h, key_idx, raw_int_value, MLS_TABLE_TYPE_INT);
}
void m_table_set_string_by_int(int table_h, int key_idx, const char *raw_c_string) {
    int str_h = s_strdup_c(raw_c_string);
    m_table_set_int_key(table_h, key_idx, str_h, MLS_TABLE_TYPE_STRING);
}
void m_table_set_const_string_by_int(int table_h, int key_idx, const char *raw_c_string) {
    int str_h = s_cstr(raw_c_string);
    m_table_set_int_key(table_h, key_idx, str_h, MLS_TABLE_TYPE_CONST_STRING);
}
void m_table_set_list_by_int(int table_h, int key_idx, int list_h) {
    m_table_set_int_key(table_h, key_idx, list_h, MLS_TABLE_TYPE_LIST);
}
void m_table_set_table_by_int(int table_h, int key_idx, int nested_table_h) {
    m_table_set_int_key(table_h, key_idx, nested_table_h, MLS_TABLE_TYPE_TABLE);
}
void m_table_set_handle_by_int(int table_h, int key_idx, int value_h, mls_table_type_t type) {
    m_table_set_int_key(table_h, key_idx, value_h, type);
}


// By cstr key
void m_table_set_int_val_by_cstr(int table_h, const char *key_cstr, int raw_int_value) {
    m_table_set_cstr_key(table_h, key_cstr, raw_int_value, MLS_TABLE_TYPE_INT);
}
void m_table_set_string_by_cstr(int table_h, const char *key_cstr, const char *raw_c_string) {
    int str_h = s_strdup_c(raw_c_string);
    m_table_set_cstr_key(table_h, key_cstr, str_h, MLS_TABLE_TYPE_STRING);
}
void m_table_set_const_string_by_cstr(int table_h, const char *key_cstr, const char *raw_c_string) {
    int str_h = s_cstr(raw_c_string);
    m_table_set_cstr_key(table_h, key_cstr, str_h, MLS_TABLE_TYPE_CONST_STRING);
}
void m_table_set_list_by_cstr(int table_h, const char *key_cstr, int list_h) {
    m_table_set_cstr_key(table_h, key_cstr, list_h, MLS_TABLE_TYPE_LIST);
}
void m_table_set_table_by_cstr(int table_h, const char *key_cstr, int nested_table_h) {
    m_table_set_cstr_key(table_h, key_cstr, nested_table_h, MLS_TABLE_TYPE_TABLE);
}
void m_table_set_handle_by_cstr(int table_h, const char *key_cstr, int value_h, mls_table_type_t type) {
    m_table_set_cstr_key(table_h, key_cstr, value_h, type);
}


// By str handle key
void m_table_set_int_val_by_str(int table_h, int key_str_h, int raw_int_value) {
    m_table_set_str_key(table_h, key_str_h, raw_int_value, MLS_TABLE_TYPE_INT);
}
void m_table_set_string_by_str(int table_h, int key_str_h, const char *raw_c_string) {
    int str_h = s_strdup_c(raw_c_string);
    m_table_set_str_key(table_h, key_str_h, str_h, MLS_TABLE_TYPE_STRING);
}
void m_table_set_const_string_by_str(int table_h, int key_str_h, const char *raw_c_string) {
    int str_h = s_cstr(raw_c_string);
    m_table_set_str_key(table_h, key_str_h, str_h, MLS_TABLE_TYPE_CONST_STRING);
}
void m_table_set_list_by_str(int table_h, int key_str_h, int list_h) {
    m_table_set_str_key(table_h, key_str_h, list_h, MLS_TABLE_TYPE_LIST);
}
void m_table_set_table_by_str(int table_h, int key_str_h, int nested_table_h) {
    m_table_set_str_key(table_h, key_str_h, nested_table_h, MLS_TABLE_TYPE_TABLE);
}
void m_table_set_handle_by_str(int table_h, int key_str_h, int value_h, mls_table_type_t type) {
    m_table_set_str_key(table_h, key_str_h, value_h, type);
}


// --- Getting Values ---

int m_table_get_int(int table_h, int key_idx) {
    if (table_h <= 0) { return 0; }
    m_table_entry_t *entry = m_table_find_int_key_entry(table_h, key_idx);
    return entry ? entry->value : 0;
}

int m_table_get_str(int table_h, int key_str_h) {
    if (table_h <= 0) { return 0; }
    m_table_entry_t *entry = m_table_find_str_key_entry(table_h, key_str_h);
    return entry ? entry->value : 0;
}

int m_table_get_cstr(int table_h, const char *key_cstr) {
    if (table_h <= 0) { return 0; }
    if (!key_cstr) { return 0; }
    
    m_table_entry_t *entry = m_table_find_cstr_key_entry(table_h, key_cstr);
    return entry ? entry->value : 0;
}

// --- Introspection ---

mls_table_type_t m_table_get_type_int(int table_h, int key_idx) {
    if (table_h <= 0) { return MLS_TABLE_TYPE_UNKNOWN; }
    m_table_entry_t *entry = m_table_find_int_key_entry(table_h, key_idx);
    return entry ? entry->type : MLS_TABLE_TYPE_UNKNOWN;
}

mls_table_type_t m_table_get_type_str(int table_h, int key_str_h) {
    if (table_h <= 0) { return MLS_TABLE_TYPE_UNKNOWN; }
    // We cannot reliably determine type of key_str_h itself without looking up MLS.
    // This function intends to find the type of the VALUE associated with key_str_h
    // So, we find the entry based on key_str_h.
    m_table_entry_t *entry = m_table_find_str_key_entry(table_h, key_str_h);
    return entry ? entry->type : MLS_TABLE_TYPE_UNKNOWN;
}

mls_table_type_t m_table_get_type_cstr(int table_h, const char *key_cstr) {
    if (table_h <= 0) { return MLS_TABLE_TYPE_UNKNOWN; }
    if (!key_cstr) { return MLS_TABLE_TYPE_UNKNOWN; }
    
    m_table_entry_t *entry = m_table_find_cstr_key_entry(table_h, key_cstr);
    return entry ? entry->type : MLS_TABLE_TYPE_UNKNOWN;
}
