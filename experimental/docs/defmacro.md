# `defmacro` Documentation

## Definition
`defmacro` is a special form in LUI used to define reusable UI components or layout patterns. Macros are expanded at compile-time (before the UI is built), allowing you to generate complex AST structures from simple, high-level declarations.

Macros can encapsulate common widget property combinations, handle repetitive layouts, or even use embedded Lua logic to procedurally generate UI elements.

## Syntax
```lisp
(defmacro macro-name (arg1 arg2 ...)
  (body-node-1)
  (body-node-2)
  ...
)
```

- **`macro-name`**: The name used to invoke the macro.
- **`(arg1 arg2 ...)`**: A list of symbols representing the macro's parameters.
- **`body-nodes`**: One or more LUI expressions. When the macro is expanded, these nodes are returned. If multiple nodes are present, they are "spliced" into the parent list.

## Examples

### 1. Simple Property Abstraction
A macro that encapsulates common grid positioning and weighting properties:
```lisp
(defmacro g1(x y) 
  :gridx x :gridy y :weightx 1 :weighty 1
)

(label :id "msg" (g1 0 0) :label "Hello World")
```

### 2. Complex Widget Patterns
A macro that defines a standard button layout with an ID prefix:
```lisp
(defmacro std-btn(n x y label cb) 
  (button :id (.. "btn_" n) 
          :gridx x :gridy y 
          :label label 
          :on-click cb)
)

(grid
  (std-btn "save" 0 0 "Save" "LUA(do_save)")
  (std-btn "quit" 1 0 "Quit" "LUA(os.exit)")
)
```

## Use Cases
- **Standardizing Layouts**: Ensure consistent padding, margins, or grid weights across multiple widgets.
- **Component Libraries**: Create reusable UI "components" (e.g., a labeled input field, a toolbar item).
- **Procedural Generation**: Use with `(lua ...)` blocks to generate grids or lists based on dynamic data.
- **Code Reduction**: Minimize boilerplate in large UI definitions.

## Lua Integration
Inside a macro's body, you can use `(lua "...")` blocks to access the macro's arguments as Lua variables. Use `luiecho(string)` to inject LUI code back into the expansion.

### Example: Auto-incrementing grid
```lisp
(lua "column_count = 0")

(defmacro auto-col(text) 
  (lua "
    local x = column_count
    column_count = column_count + 1
    luiecho(string.format('(label :label %q :gridx %d)', text, x))
  ")
)
```

## Related Concepts
- **`(lua ...)`**: Execute Lua code during macro expansion.
- **`luiecho(string)`**: Function used inside Lua blocks to output LUI code.
- **`macros.expand(ast)`**: The Lua function responsible for processing macros in the AST.
- **Splicing**: The mechanism where a macro returning multiple nodes integrates them directly into the parent list's scope.
