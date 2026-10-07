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
| `demo_login.lua` | Login dialog — password fields, toggles, feedback |
| `demo_wbutton.lua` | Wbutton variants — macros, sizes, alignment, callbacks |
| `demo_wlabel_svg.lua` | Wlabel with inline SVG images |
| `demo_wretex.lua` | Wretex — multiline retex text with inline SVG |
| `demo_wretex_debug.lua` | Wretex debug — fixed-size widgets |

## Wlabel inline SVG

`demo_wlabel_svg.lua` demonstrates the `\includesvg{filename}{width}{height}`
command used inside Wlabel text to embed SVG images inline, mixed into a line of
text. Sizes are given in points (e.g. `12pt`). The SVG assets live in
`LuaRunner/` (`alert.svg`, `question.svg`, `ink1.svg`).
