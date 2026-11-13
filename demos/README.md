# XTCW Widget Demos

Working LUI demo scripts showcasing XTCW widgets and layouts. Run them with the
`t1.sh` launcher at the repository root, or invoke `commander_runner` directly.

## Running Demos

The simplest way is the `t1.sh` TUI launcher:

```bash
./t1.sh                 # interactive whiptail menu (all demos)
./t1.sh form            # run a demo directly
./t1.sh --trace 50 form # run with TRACE(50) output
```

`t1.sh` auto-builds missing artifacts via `start.sh`, uses `xvfb-run` when no
`DISPLAY` is set, and returns to the menu after each demo exits.

To run a single LUI script from any directory, use the `run_lui.sh` wrapper at
the repository root. It resolves the script path and starts `commander_runner`
from the correct working directory:

```bash
./run_lui.sh demos/demo_form.lua
./run_lui.sh demos/demo_form.lua -Tracelevel 2
cd /tmp && /path/to/xtcw/run_lui.sh /path/to/xtcw/demos/demo_listview.lua
```

To run a script by hand, use `commander_runner` **from the `commander/`
directory** (it hard-codes `package.path = '../lui/?.lua'`, so it must run from
there):

```bash
cd commander
./commander_runner -Luafile ../demos/demo_form.lua
./commander_runner -Luafile ../demos/demo_form.lua -Tracelevel 2
```

## Available Demos

| Script | Description |
|--------|-------------|
| `demo_calc.lua` | Calculator — 4×6 grid with `gridWidth` spanning |
| `demo_contacts.lua` | Contact manager — multi-row form, spinbox, property round-trip |
| `demo_drawcanvas.lua` | DrawCanvas — custom drawing via a plug-in C callback |
| `demo_eqfader.lua` | EQ Fader — 10-band vertical Cairo/Xft EQ fader with dB scale |
| `demo_theme.lua` | Theme — central colour registry: switch themes and toggle borders |
| `demo_form.lua` | Gridbox layout — labels, edit, spinbox, toggle |
| `demo_karoed.lua` | KaroEd — multiline text editor (joins, selection, clipboard, tabs) |
| `demo_bravo.lua` | Wbravo — piece-table plain-text editor rendered with re-tex (undo/redo, clipboard) |
| `demo_rich_bravo.lua` | Wbravo rich text — styles, paragraph formatting, plain UTF-8 text import/export and full-state snapshots (burger menu; Ctrl+B/I/R) |
| `demo_listview.lua` | WListView — themed multi-column list with header, scrollbars, icons, alternate rows and keyboard navigation |
| `demo_filechooser.lua` | File chooser — WfileSelector/WfileChooser over WListView, imports a picked text file into Wbravo |
| `demo_login.lua` | Login dialog — password fields, toggles, feedback |
| `demo_wbutton.lua` | Wbutton variants — macros, sizes, alignment, callbacks |
| `demo_wlabel_svg.lua` | Wlabel with inline SVG images |
| `demo_wretex.lua` | Wretex — multiline retex text with inline SVG |
| `demo_wretex_debug.lua` | Wretex debug — fixed-size widgets |
| `demo_wtoggleswitch.lua` | WToggleSwitch — animated oval toggle switch with a sliding knob |
| `demo_viewport.lua` | Viewport — scrollable view of child widgets (right/bottom scrollbars) |
| `demo_wtabwidget.lua` | WtabWidget — tab group with scrollable pages |
| `demo_wmenubutton.lua` | WmenuButton — burger button with a tabbed popup menu |

## Wlabel inline SVG

`demo_wlabel_svg.lua` demonstrates the `\includesvg{filename}{width}{height}`
command used inside Wlabel text to embed SVG images inline, mixed into a line of
text. Sizes are given in points (e.g. `12pt`). The SVG assets live in
`LuaRunner/` (`alert.svg`, `question.svg`, `ink1.svg`).
