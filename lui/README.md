# LUI — Lua User Interface

LUI is a small S-expression based markup language for building X Toolkit (Xt) widget
trees, embedded in and driven by Lua. It is parsed to a Lua AST, macro-expanded, and
then built into real widgets by a backend.

This document describes the language and — in particular — its **macro** system,
including compile-time code generation written in Lua.

## Table of Contents

1. [Overview](#overview)
2. [Language Syntax](#language-syntax)
3. [Processing Pipeline](#processing-pipeline)
4. [Widgets](#widgets)
5. [Macros](#macros)
6. [Compile-Time Lua / Code Generation](#compile-time-lua--code-generation)
7. [Special Forms](#special-forms)
8. [Worked Examples](#worked-examples)
9. [Pitfalls](#pitfalls)

---

## Overview

A LUI document is a sequence of S-expressions. The canonical "hello world" widget:

```
(window :title "Hello" :width 300 :height 200
  (grid
    (button :label "OK" :callback "LUA(on_ok)")))
```

LUI is a *preprocessor language*: source text is turned into a Lua table (the AST),
which is then expanded by the macro engine and handed to a backend that creates the
actual widgets. There is no separate compiler binary — everything runs inside Lua.

The core modules are:

| Module | Role |
|--------|------|
| `parser` | LPeg parser: source text → AST |
| `macros` | Macro registration and expansion (incl. compile-time Lua) |
| `registry` | Maps widget tags to widget classes |
| `backend_xt` | Turns the expanded AST into widgets, applies properties |
| `gui_xt` | High-level GUI helpers (`gui.set`, `gui.get`, stores, dialogs, …) |
| `lui` | Entry points `lui.run` / `lui.load` / `lui.loop` |

---

## Language Syntax

### Atoms

| Kind | Syntax | AST value |
|------|--------|-----------|
| symbol | `foo`, `bar-baz`, `_x.y` | string `"foo"` |
| keyword | `:name` | `{ keyword = ":name" }` |
| number | `42`, `3.14` | number |
| string | `"text"` | string (escapes `\"` `\\` `\n` `\t`) |
| boolean | `true` / `false` | boolean |
| nil | `nil` | `parser.NIL` sentinel |

### Comments and whitespace

- `;` starts a comment that runs to end of line.
- Commas `,` are whitespace — `(a b)` and `(a, b)` are equivalent.

### Lists and tables

- `( ... )` parses to an **array** (numeric-indexed table) — used for widget nodes and
  macro calls.
- `{ key = value ... }` parses to a **hash** table — used for compile-time data.

```
(grid :gridx 0 :gridy 1)          ; array: { "grid", {keyword=":gridx"}, 0, ... }
{ left = 10, right = 20 }          ; hash:  { left = 10, right = 20 }
```

---

## Processing Pipeline

`lui.run(source)` (see `lui/lui.lua`) does:

```
source ──parse──▶ AST ──expand──▶ expanded AST ──build──▶ widgets ──manage──▶ shown
```

1. `parser.parse(source)` — LPeg parse to an AST.
2. `macros.expand(ast)` — register macros, evaluate compile-time Lua, expand macro calls.
3. `backend.build(node)` — for each top-level node, create the widget and its children,
   applying properties (with `process_property` aliases).
4. `gui.manage(first_widget)` — manage the root widget so the shell becomes visible.

The same pipeline runs inside `lui.load(filename)`, which additionally accepts a raw
S-expression file (first non-blank character `(`) or a Lua script that sets
`_G.lui_source`.

---

## Widgets

Widget tags come from `registry.lua` (friendly aliases like `button`, `grid`,
`list-view`) and `registry_generated.lua` (one tag per compiled class, e.g. `Wbutton`,
`WlsMulti`). A node is written as:

```
(tag :prop1 value1 :prop2 value2 (child ...) (child ...))
```

`backend_xt.process_property` maps friendly property names to widget resources
(`on-click` → `callback`, `on-row-activated` → `notify`, `model` → `tableStrs`,
`columns` → `columnWidths`, …).

---

## Macros

A macro is a named pattern that expands to a sequence of AST nodes. Macro calls look
like ordinary list nodes whose head is the macro name:

```
(name arg1 arg2 ...)
```

### Defining a macro in LUI: `defmacro`

```
(defmacro grid_opts (gx gy wx wy)
  :gridx gx :gridy gy :weightx wx :weighty wy)
```

This registers `grid_opts` with formal parameters `gx gy wx wy` and a body. It is
removed from the output, so `(defmacro …)` produces no widget. Because registration
happens in a first pass over the whole AST, a `defmacro` may appear *after* its use.

Usage:

```
(Wbutton :label "OK" (grid_opts 0 1 1 0) :callback "LUA(on_ok)")
```

expands to:

```
(Wbutton :label "OK" :gridx 0 :gridy 1 :weightx 1 :weighty 0 :callback "LUA(on_ok)")
```

### Defining a macro from Lua: `macros.register`

```lua
local macros = require('macros')

macros.register('grid_opts', { 'gx', 'gy', 'wx', 'wy' }, {
    { keyword = ':gridx' },  'gx',
    { keyword = ':gridy' },  'gy',
    { keyword = ':weightx' }, 'wx',
    { keyword = ':weighty' }, 'wy',
})
```

The body is an array of AST nodes. This is the same macro as above, but defined in Lua
(useful for plugins, e.g. `list_scrollbar.lua`).

### Expansion semantics

When `(name a1 a2 …)` is expanded:

1. Each argument is itself expanded first.
2. The arguments are bound to the macro's formal parameters in a fresh environment.
3. Each body node is expanded in that environment.
4. If the body expands to a single node, that node is returned; otherwise the list is
   returned.

During body expansion, `expand_node` applies these substitution rules:

- A **keyword** (`{ keyword = ":x" }`) is returned unchanged.
- A **string** that matches an environment variable is replaced by its value (this is
  how formal parameters are substituted).
- A `{ lua = "…" }` node is evaluated as compile-time Lua (see below).
- A nested macro call is expanded recursively.
- A regular list has its children expanded.

### Splicing

When expanding a list, a child whose expansion is itself a *list of AST nodes* is
**spliced** into the parent. This is what lets `(grid_opts 0 1 1 0)` inject multiple
`keyword/value` pairs into a widget node in place.

### Evaluation result

The value of a macro argument/body can be anything the macro engine can produce —
numbers, strings, or AST nodes — so macros can both *substitute values* and *generate
structure*.

---

## Compile-Time Lua / Code Generation

The macro engine can run arbitrary Lua at expansion time. This is the "code gen by Lua
code" facility: you write Lua that *produces* LUI markup or AST nodes, which are then
parsed/built as if you had written them by hand.

### `(lua …)` blocks

A `(lua …)` node evaluates its body (raw Lua, balanced parentheses respected) at
expansion time. The body may use a helper, **`luiecho`**, to turn a generated
S-expression string into AST nodes:

```
(lua
  local s = ""
  for i = 1, 3 do
    s = s .. "(label :label \"Row " .. i .. "\" :gridx 0 :gridy " .. (i - 1) .. ")"
  end
  luiecho(s))
```

This expands to three `(label …)` nodes. `luiecho(text)` parses `text` and returns the
resulting AST; the returned nodes are spliced into the surrounding list. Note that
`luiecho` output is inserted **as-is** — it is not re-run through macro expansion, so
the generated string must contain fully-expanded markup.

### `{ lua = "…" }` nodes in a macro body

Inside a macro body, a node of the form `{ lua = "…" }` runs Lua code and **returns its
value**. The code runs with the macro's formal parameters in scope, so you can compute
values from the arguments:

```lua
macros.register('labeled', { 'id', 'text' }, {
    'label',
    { keyword = ':id' }, { lua = "return tostring(id)" },
    { keyword = ':label' }, { lua = "return tostring(text) .. '!'" },
})
```

Here `(labeled foo "hi")` expands to a label with id `foo` and label text `hi!`.

This is how `list_scrollbar.lua` builds a composite widget, generating ids and callback
strings from a single `id` argument:

```lua
macros.register('list-scrollbar', {'id','width','height','slwidth'}, {
    'grid', {keyword=':id'}, { lua = "return tostring(id) .. '_grid'" },
    { 'list4', {keyword=':id'}, { lua = "return tostring(id)" },
      {keyword=':gridx'}, 0, {keyword=':gridy'}, 0,
      {keyword=':vscroll'}, { lua = "return 'LUA(_list_scrollbar_vscroll_cb, ' .. id .. ')'" } },
    { 'vslider', {keyword=':id'}, { lua = "return id .. '_slider'" },
      {keyword=':callback'}, { lua = "return 'LUA(_list_scrollbar_slider_cb, ' .. id .. ')'" } }
})
```

Note the code strings are full Lua chunks — use an explicit `return` to yield a value.

### The sandbox environment

Compile-time Lua runs in a restricted environment providing:

- Constants: `screenwidth`, `screenheight`, `dpi`, `charwidth`, `charheight`, `platform`.
- `gui` (the `gui_xt` module), `plugins`, `loaded_modules`.
- Standard: `_G`, `io`, `math`, `table`, `string`, `pairs`, `ipairs`, `next`,
  `tonumber`, `tostring`, `type`, `select`, `error`, `assert`, `print`, `pcall`,
  `xpcall`, `load`.
- A restricted `os` (`date`, `time`, `clock`, `difftime`, `execute`, `exit`, `getenv`).
- `luiecho(text)` — parse a string to AST and return it.

When a `(lua …)` / `{ lua = "…" }` runs inside a macro expansion, the macro's formal
parameters (and their bound values) are merged into this environment first, so the code
can read the arguments directly.

---

## Special Forms

The macro engine recognizes several special forms:

| Form | Phase | Effect |
|------|-------|--------|
| `(defmacro name (args) body…)` | first pass | Register a macro (produces no widget) |
| `(import "module")` | first pass | `require` a Lua module into the compile-time env |
| `(require name [version])` | first pass | Load a plugin (currently stubbed) |
| `(style "file")` | first pass | Call `gui.load_css(file)` (currently a no-op warning) |
| `(include "file")` | expansion | Parse + expand another LUI file in place |
| `(lua …)` | expansion | Evaluate compile-time Lua, splice resulting AST |

---

## Worked Examples

### Simple macro

```
(defmacro grid_opts (gx gy wx wy)
  :gridx gx :gridy gy :weightx wx :weighty wy)

(window :title "Demo" :width 400 :height 300
  (grid
    (button :label "A" (grid_opts 0 0 1 1))
    (button :label "B" (grid_opts 1 0 1 1))))
```

### Code generation: a loop of widgets

```
(window :title "Generated" :width 300 :height 400
  (grid
    (lua
      local s = ""
      for i = 1, 5 do
        s = s .. "(label :label \"Item " .. i .. "\" :gridx 0 :gridy " .. (i - 1) .. " :weightx 1 :weighty 0)"
      end
      luiecho(s)))
```

### Code generation: a parameterized composite

```lua
-- in Lua, before lui.run(...)
local macros = require('macros')
macros.register('labelled', { 'id', 'text' }, {
    'label',
    { keyword = ':id' },   { lua = "return tostring(id)" },
    { keyword = ':label' }, { lua = "return tostring(text) .. '!'" },
})
```

```lua
-- LUI source
(window :title "Composite"
  (grid
    (labelled a "one")
    (labelled b "two")))
```

---

## Pitfalls

- **`defmacro` vs. `macros.register`**: both register into the same table, so a macro
  registered in Lua is visible to LUI source and vice versa.
- **Argument substitution is name-based**: a bare symbol in a macro body is replaced
  *only* if it matches a bound formal parameter (or environment variable). Use
  `{ lua = "…" }` when you need computed values (e.g. string concatenation, `tonumber`).
- **`{ lua = "…" }` must `return`**: the code string is a complete chunk; without an
  explicit `return` the node's value is `nil` and the property is dropped.
- **`luiecho` parses S-expressions**: the generated string must be valid LUI markup,
  and its output is inserted as-is (it is **not** re-macro-expanded).
- **Splicing**: a macro that expands to multiple top-level nodes is spliced; ensure the
  expanded head is an AST node (a string tag or special form) so the engine splices
  rather than nests it.
