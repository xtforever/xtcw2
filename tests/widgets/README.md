# Single-widget probes

Run one widget in the standard 700×600 harness:

```bash
./run_widget.sh '(Wlabel :label "Hello" :fontSize 20)'
./run_widget.sh tests/widgets/wlabel.lui
./run_widget.sh --after tests/widgets/populate_list.lua tests/widgets/wlistview.lui
./run_widget.sh --autoclose 500 '(Wlabel :label "probe")'
```

The harness places the probe in a host `Gridbox` that fills the window and
adds a small **Quit** button at the bottom right. Probes that do not specify
grid constraints get a full-cell default (`weightx 1`, `weighty 1`, `fill 3`).
For a natural-sized, centered control, add
`:gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill none`.

Harness options must come before the widget; everything after the widget is
passed to `run_lui.sh` (for example `-Tracelevel 5`).

Useful hooks:

- `--setup FILE` — Lua file run before the widget is built (callback globals)
- `--after FILE` — Lua file run after the widget is built (populate models)
- `--result FILE` — write `key=value` geometry/probe data
- `--autoclose MS` — exit automatically for CI

The probes in this directory start with the P0 widgets from
`Improved-Widget-Set.md`. Add a probe whenever a new widget lands.

`wappwindow.lui` is the first P0 implementation probe: it uses the
`WAppWindow` menu/toolbar/content/status slots and verifies the slot layout.
`wmenubar.lui` covers the first menu slice: `WMenuBar`, `WmenuButton` triggers
and `WPopupMenu` content. `waction.lui` covers checkable `WAction` items with
shortcut hints. `wtoolbar.lui` covers the themed `WToolBar` action row. `wstatusbar.lui`
covers the message + permanent-widget `WStatusBar`. `wcombobox.lui` covers the
read-only `WComboBox` and its popup menu. `wradiogroup.lui` covers the themed
radio group. `wlineedit.lui` covers placeholder + clear-button input. `wprogressbar.lui`, `wcheckbox.lui`
and `wformlayout.lui` cover the newer form/feedback widgets.
