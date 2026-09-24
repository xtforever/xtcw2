# XTCW2 + LUI Documentation

This guide explains how to build lightweight, future-proof GUI applications using the **XTCW2** toolkit and the **LUI** declarative language.

## 1. Getting Started
To run an LUI application, use the `luarunner` tool:
```bash
./luarunner -Luafile myapp.lua
```

## 2. Declarative UI (LUI)
Define your interface using S-expressions.

```lisp
(window :id "top" :title "My App"
  (grid
    (label :id "msg" :label "Hello World" :gridx 0 :gridy 0)
    (button :id "btn" :label "Click Me" :gridx 0 :gridy 1 :callback "LUA(on_click)")
  )
)
```

## 3. Lua API (`gui` module)

### Widget Manipulation
*   `gui.set(id, property, value)`: Set a widget resource.
*   `gui.get(id, property)`: Get a widget resource.
*   `gui.manage(id)`: Make a widget visible.
*   `gui.unmanage(id)`: Hide a widget.

### Stores & Data Binding
Stores are used for widgets like `list-view`.
*   `store = gui.create_store(columns)`: Create a data store.
*   `gui.store_append(store, {val1, val2, ...})`: Add data to a store.
*   `store:clear()`: Remove all data from a store.

### Dialogs (Modal)
*   `gui.alert(message)`: Show an info dialog.
*   `gui.confirm(message)`: Show a Yes/No dialog (returns boolean).
*   `gui.prompt(title, message, default)`: Show an input dialog (returns string or nil).

## 4. Advanced Text Alignment (`re-tex`)
The `label` widget uses the `re-tex` engine. Use the `:align` property:
*   `0`: Left
*   `1`: Center
*   `2`: Right
*   `3`: Justify

## 5. Event Loop
Every Lua script must end with:
```lua
gui.loop()
```
