# LUI Project Summary

## Overview
LUI (Lisp-like User Interface) is a framework for defining and building X Toolkit (Xt) based GUIs using S-expressions. It consists of a Lua-based core for parsing and logic, and a C-based host (`luarunner`) for rendering and Xt integration.

## Directory Structure
- `lui/`: Core LUI engine.
    - `parser.lua`: LPeg-based parser. Converts S-expressions to AST.
    - `macros.lua`: Macro expansion and compile-time evaluation.
    - `backend_xt.lua`: Translates AST to Xt widgets via `xtcreate`.
    - `gui_xt.lua`: High-level API and event loop management.
    - `registry.lua`: Tag-to-WidgetClass mapping.
- `LuaRunner/`: The C host application.
    - `luarunner.c`: Initializes Xt and the Lua environment.
    - `luaxt.c`: Implements the `luaxt` C-module for Lua-C bridge.
- `lui_test/`: Test scripts and examples.

## Key Components & Technical Findings

### 1. Parser & AST
- **AST Format**: A nested table structure. Symbols are strings, keywords are tables like `{keyword=":id"}`.
- **Nil Sentinel**: `parser.NIL` is used to represent `nil` in the AST to distinguish it from missing keys in Lua tables.
- **Debugging**: `parser.dump(ast)` provides a formatted string of the AST for inspection.

### 2. Macro System
- **Usage**: Macros must be expanded using `macros.expand(ast)` before the AST is passed to the backend.
- **Detailed Documentation**: See [docs/defmacro.md](docs/defmacro.md) for full syntax and examples.
- **Splicing**: If a macro returns multiple nodes, the expansion logic "splices" them into the parent's child list.
- **Nesting**: Macros can call other macros or use compile-time Lua blocks `(lua ...)`.

### 3. Backend (Xt Integration)
- **Property Mapping**: `backend_xt.lua` maps LUI properties (e.g., `:on-click`, `:src`) to Xt resources (e.g., `callback`, `filename`).
- **Strict Typing**: The C-function `xtcreate` (in `luarunner.c`) expects all resource values as **strings**. 
- **Filtering**: Properties set to `nil` or `parser.NIL` in LUI must be explicitly filtered out in `backend_xt.lua` to avoid passing non-string pointers to the C layer.

### 4. Event Loop & Callbacks
- **Callback Format**: Callbacks are often defined as `LUA(function_name, argument)`.
- **Stack Mechanism**: `luaxt` uses a list (`cb_list`) to push callbacks. `luaxt_pushcallback` pushes `class_data` (event-specific data) and then the `callback_str`.
- **Parsing**: `gui_xt.lua` pulls these from the stack. It now supports comma-separated strings in `callback_str` (e.g., `"my_func, my_arg"`) and dispatches them correctly to the Lua environment.
- **Direct Functions**: `backend_xt.lua` can also handle direct Lua functions by registering them with a generated ID and wrapping them in a `LUA(...)` string.

## Common Workflows & Troubleshooting

### Running LUI Scripts
The preferred way to run LUI scripts is through the `luarunner` host to ensure the `luaxt` module is available:
```bash
cd lui_test
../LuaRunner/luarunner -Luafile my_script.lua
```

### Technical Gotchas
- **Missing Macros**: If widgets aren't appearing or properties aren't being set, check if `macros.expand` was called.
- **xtcreate Argument Error**: Usually caused by a `nil` or table being passed where a string was expected. Ensure `process_property` in `backend_xt.lua` is filtering sentinels.
- **Callback Not Triggering**: Verify the callback string format. If using arguments, ensure `gui_xt.lua` is parsing the comma correctly.
- **luaxt Not Found**: Occurs when running with standard `lua` instead of `luarunner`.

## Recent Fixes
- Added `parser.dump` for AST visualization.
- Fixed `backend_xt.lua` to handle both literal `nil` and `parser.NIL` sentinels.
- Fixed `gui_xt.lua` event loop to correctly parse and dispatch comma-separated callback arguments.
- Updated `test_improved.lua` to demonstrate proper macro expansion and direct Lua function callbacks.
