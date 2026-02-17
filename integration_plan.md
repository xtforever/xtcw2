# Integration Plan: Lui + XTCW2

## Overview
Integrate the `lui` (Lua User Interface) declarative language into the `xtcw2` project. This will allow building UIs using S-expressions while leveraging the lightweight `xtcw` widget set and the high-quality `re-tex` layout engine.

## 1. Core Parser & Macros
*   **Action**: Import `parser.lua` and `macros.lua` from the `lui` project.
*   **Dependency**: Ensure `LPeg` is available in the Lua environment.
*   **Location**: Place these in `xtcw2/lua/lui/`.

## 2. Xt/XTCW Backend (`backend_xt.lua`)
*   **Action**: Implement a new backend module that translates the `lui` AST into `xtcw` widget trees.
*   **Mapping**:
    *   `(window)` -> `applicationShellWidgetClass`
    *   `(vertical)` / `(horizontal)` -> `Gridbox` or `VBox`/`HBox`
    *   `(label)` -> `Wlabel` (using `re-tex`)
    *   `(button)` -> `Wbutton`
    *   `(input-string)` -> `Wedit`
*   **Implementation**: Use the `xtcreate` and `xtsetvalue` functions exposed by the host application.

## 3. Lua Host Enhancement (`luarunner.c`)
*   **Action**: Enhance `luarunner.c` to be the primary host for `lui` applications.
*   **New Bindings**:
    *   `xtgetvalue(widget, resource)`: Fetch widget resources into Lua.
    *   `xtmanage(widget)` / `xtunmanage(widget)`: Control widget visibility.
    *   `xtdestroy(widget)`: Clean up widgets.
*   **Event Loop**: Ensure the `XtAppProcessEvent` loop integrates cleanly with Lua callbacks.

## 4. `re-tex` Integration in `lui`
*   **Action**: Ensure `Wlabel` properties like `fontFace` and `fontSize` are easily accessible via `lui` keywords.
*   **Feature**: Enable multi-paragraph text in `lui` labels using the recently added `TOK_PAR` support in `re-tex`.

## 5. Memory Management (`mls.c`)
*   **Action**: Ensure all Lua/C bridge data (strings, widget handles) uses `mls` for allocation to maintain the toolkit's low-footprint and easy-maintenance goals.

## 6. Deployment & Portability
*   **Action**: Bundle Lua 5.3 and LPeg source.
*   **Goal**: Ensure the entire toolkit (including `lui` and `re-tex`) can be compiled with minimal external dependencies beyond a standard X11 environment and Cairo.

## Roadmap
1.  **Step 1**: Update `luarunner.c` with necessary C-Lua bindings.
2.  **Step 2**: Implement `backend_xt.lua` and `gui_xt.lua`.
3.  **Step 3**: Create a demo `.lui` file that reproduces `retex_test` using the new system.
4.  **Step 4**: Update `plan.md` to reflect the completed integration.
