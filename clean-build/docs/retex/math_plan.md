# Analysis and Plan for Math Mode in re-tex

Based on the analysis of `tex.web` (specifically sections 34 and 35), here is the plan to implement a proper math engine for `re-tex`.

## Analysis of tex.web

TeX handles math by treating formulas not as simple lists of characters (like text), but as **Math Lists (mlists)** composed of **Noads**.

1.  **Data Structures (Section 34)**:
    *   **Noad**: The basic unit of a math list.
    *   **Types**: `Ord` (ordinary), `Op` (operator), `Bin` (binary op), `Rel` (relation), `Open` (left brace), `Close` (right brace), `Punct` (punctuation), `Inner` (delimited sub-formula).
    *   **Fields**: Each noad has:
        *   `nucleus`: The character or sub-mlist.
        *   `subscr`: Subscript field.
        *   `supscr`: Superscript field.
        *   `type`: The class (Ord, Bin, etc.).

2.  **Algorithms (Section 35)**:
    *   **mlist_to_hlist**: This is the core function. It converts the abstract `mlist` tree into a concrete `hlist` (boxes and glue) that can be rendered.
    *   **Styles**: It maintains a `style` (Display, Text, Script, ScriptScript, Cramped variants) which dictates font size and spacing.
    *   **Spacing**: It inserts glue between atoms based on their types (e.g., `Bin` gets medium space around it, `Rel` gets thick space).
    *   **Script Positioning**: It calculates precise vertical shifts for subscripts and superscripts based on font metrics (x-height, etc.).

## Plan for re-tex

Current `re-tex` treats math input linearly in `builder.c`. We need to introduce an intermediate "Math Parse" stage and a "Math Layout" stage.

### Phase 1: Data Structures (`math_structs.h`)

Define the `Noad` structure.

```c
typedef enum {
    NOAD_ORD, NOAD_OP, NOAD_BIN, NOAD_REL, 
    NOAD_OPEN, NOAD_CLOSE, NOAD_PUNCT, NOAD_INNER,
    NOAD_RADICAL, NOAD_FRACTION
} NoadType;

typedef struct Noad {
    NoadType type;
    int nucleus; // Handle to Node or sub-mlist
    int subscr;  // Handle to sub-mlist
    int supscr;  // Handle to sub-mlist
    // ... font family info ...
} Noad;
```

### Phase 2: Math Parsing (`math_parser.c`)

Modify `builder.c` (or create a new module) to parse math mode input into an `mlist` instead of a linear `hlist`.

*   **Tokenizer**: Reuse existing tokenizer (already has `_`, `^`, `$`).
*   **Tree Builder**:
    *   When `$` is encountered, switch to "Math Builder" mode.
    *   Accumulate tokens into `Noad`s.
    *   Handle `_` and `^` by attaching the following item to the previous `Noad`'s subscript/superscript field.
    *   Handle `{}` by creating sub-mlists.
    *   Auto-classify chars: `+` -> `Bin`, `=` -> `Rel`, `(` -> `Open`, `a` -> `Ord`.

### Phase 3: Math Layout (`math_layout.c`)

Implement `mlist_to_hlist`.

*   **Input**: `mlist` (from Phase 2), `style` (e.g., TEXT_STYLE).
*   **Output**: `hlist` (standard nodes ready for `line_break` or direct rendering).
*   **Logic**:
    1.  **Iterate**: Walk through the `mlist`.
    2.  **Recursion**: If `nucleus`, `subscr`, or `supscr` is an `mlist`, recursively call `mlist_to_hlist` with the appropriate style (e.g., subscript uses `SCRIPT_STYLE`).
    3.  **Spacing**: Check the type of the current noad and the next noad. Look up the spacing in a table (e.g., `Ord` followed by `Bin` -> `MedMuskip`). Insert `Glue` node.
    4.  **Scripts**: Construct a `VBox` (or manual vertical shifts) to position the nucleus, subscript, and superscript correctly relative to the baseline.

### Phase 4: Integration

*   Update `retex_layout` to detect math boundaries.
*   When a math list is finished, pass it to `mlist_to_hlist`.
*   Append the resulting `hlist` to the main paragraph.

## Next Steps

1.  Create `re-tex/src/math_structs.h` (Noad definitions).
2.  Create `re-tex/src/math_layout.c` (Skeleton of `mlist_to_hlist`).
3.  Update `re-tex/src/builder.c` to produce Noads in math mode.
