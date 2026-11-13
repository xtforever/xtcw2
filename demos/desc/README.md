# Demo descriptions (`.retex`)

Optional rich descriptions for the demo launcher (`t1_menu`).  A description
is a plain UTF-8 text file using the **re-tex** markup understood by
`Wretex`/`Wlabel`; there is no metadata header in this version.

```text
A calculator demo with a \bf bold\rm  emphasis and an icon:
\includesvg{LuaRunner/file.svg}{12pt}{12pt}
```

Supported markup (see `re-tex/src/builder.c`):

- `\bf`, `\it`, `\rm` — bold / italic / roman
- `\fontface{Sans}` — font face
- `\tiny`, `\small`, `\normalsize`, `\large`, `\Large`, `\LARGE`, `\huge`,
  `\Huge` — relative sizes
- `\includesvg{file.svg}{width}{height}` — inline SVG icon
- `\hspace`, `\vspace`, `\rule`, `\hbox`, `\raisebox`, `\vcenter`, `\parbox`
- `$...$` — math
- `%` — line comment

## Paths

`\includesvg` paths are resolved relative to the **`.retex` file's directory**
by `lui/retex_file.lua` (`resolve_includes`).  Absolute paths are left
unchanged.

```lua
local rf = require('retex_file')
local text, base = rf.load('demos/desc/calc.retex')
local body = rf.resolve_includes(text, base)
-- pass body to Wretex/Wlabel
```

## Files

Use one file per demo, named after the demo key:

```text
demos/desc/calc.retex
demos/desc/listview.retex
```

The demo registry in `t1.sh` remains the source of truth for key/title/short
description; the `.retex` file only holds the rich detail body.
