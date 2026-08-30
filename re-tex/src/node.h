#ifndef NODE_H
#define NODE_H

#include "scaled.h"
#include "glue.h"

typedef enum {
    NODE_CHAR,
    NODE_HBOX,
    NODE_VBOX,
    NODE_GLUE,
    NODE_PENALTY,
    NODE_KERN,
    NODE_RULE,
    NODE_SVG,
    NODE_PROPERTY
} NodeType;

typedef enum {
    PROP_HANG_INDENT,
    PROP_HANG_AFTER
} PropType;

/**
 * @brief Base Node structure.
 * Nodes will be stored in memc lists.
 */
typedef struct {
    NodeType type;
    Scaled width;
    Scaled height;
    Scaled depth;
    Scaled shift; // Vertical shift (baseline offset)
    int list; // Handle to a memc list of child nodes (for boxes)
    int data; // Character code or other specific data
    float vertical_scale; // Non-uniform vertical scale (for delimiters)
    Glue glue_spec; // For NODE_GLUE
    double glue_set; // Ratio for boxes
    int glue_order; // Order of glue being adjusted
    Scaled content_width; // Natural content width (before glue stretch), for boxes
    
    // Font info
    float font_size;
    int font_style; // bitmask: 1:Bold, 2:Italic, 4:SmallCaps
    int font_face_handle; // Handle to interned string
    int source_offset; // Byte offset in original string
    int reverse_mode; // 1 = fill char box with foreground color, draw char in white
} Node;

// Node construction
void node_create_char(int list_handle, int char_code, Scaled w, Scaled h, Scaled d, float fsize, int fstyle, int fhandle, int reverse_mode);
void node_create_hbox(int list_handle, int child_list);
void node_create_vbox(int list_handle, int child_list);
void node_create_glue(int list_handle, Glue spec, int reverse_mode);
void node_create_kern(int list_handle, Scaled amount, int reverse_mode);
void node_create_rule(int list_handle, Scaled w, Scaled h, Scaled d, int reverse_mode);
void node_create_svg(int list_handle, const char *filename, Scaled w, Scaled h, int reverse_mode);
void node_create_property(int list_handle, PropType type, int value, int reverse_mode);
int node_list_to_vbox(int all_lines, double font_size_pt);

/**
 * @brief Packages a list of nodes into an HBox of a specific target width,
 * calculating glue_set to justify the content.
 * 
 * @param child_list List of child nodes
 * @param target_width Desired width of the box
 * @return The HBox node struct
 */
Node node_pack_hbox(int child_list, Scaled target_width);

/**
 * @brief Recursively frees a node and its children.
 * 
 * @param node_handle Handle to the list of 1 Node to free.
 */
void node_free_tree(int node_handle);

/**
 * @brief Recursively frees all nodes in a list and the list itself.
 * 
 * @param list_handle Handle to the list of Nodes.
 */
void node_list_free(int list_handle);

// Serialization for headless testing
void node_dump(int node_handle, int indent, int output_buf);

#endif // NODE_H
