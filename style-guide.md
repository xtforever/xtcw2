# XTCW Widget Style Guide

A practical layout, typography and colour guide for widgets and dialogs, based on
Apple's Human Interface Guidelines (Layout, Toolbars, Buttons) and adapted to this
toolkit's theme/geometry model.

Read this together with:

- `wbuild_widgets/CONVENTIONS.md` — how to author the widget (syntax, geometry, theming)
- `themes.hdf` — the colour/metric/font values this guide names
- `utils/theme.h` — the theme query API
- `dialog-guide.md` — LLM-facing, operational dialog guide (decision rules,
  microcopy, patterns, self-check, output contract)

The canonical reference implementations are `WfileChooser` (dialog layout),
`WListView` (list surface) and `Wbravo` (editor surface).

## 1. Principles

- **Reading order.** Most important content goes top and leading (left for LTR).
  People scan top → bottom, leading → trailing.
- **Align edges.** Align labels, fields and lists to shared left/right edges. Use
  indentation only to express hierarchy.
- **Group with space.** Use negative space or separators to show which controls
  belong together; too many groups make a toolbar feel cluttered (aim for ≤ 3).
- **One primary action.** Exactly one prominent/accent button per view (Save/Open/
  OK). Use style, not size, to distinguish the preferred choice.
- **Differentiate controls from content.** Toolbars/inputs get their own surface
  roles; content (lists/editors) gets the content roles.
- **Consistent spacing.** All spacing comes from the scale below, never ad-hoc.

## 2. Spacing scale (px)

Xt geometry is in pixels. (`font_*` values are Xft/retex font sizes.)

| Token | px | Use |
|-------|----|-----|
| `XS` | 4  | icon↔text gap, tight internal padding |
| `S`  | 8  | control↔control, label↔field, action gap |
| `M`  | 12 | between rows/groups inside a panel |
| `L`  | 16 | dialog/window outer margin, panel padding |
| `XL` | 20 | large section separation |

Theme keys: `window_margin` (16), `control_gap` (8), `group_gap` (12),
`action_gap` (8).

## 3. Control metrics

| Theme key | Default | Meaning |
|-----------|---------|---------|
| `control_height` | 24 | uniform height for buttons, inputs, toolbar items |
| `control_padding` | 6 | inner horizontal padding |
| `control_border` | 1 | flat 1px outline for inputs/controls |
| `list_row_height` | 22 | minimum list body row height |
| `list_header_height` | 24 | list column header height |
| `list_cell_pad_x` | 8 | list cell horizontal padding |
| `list_cell_pad_y` | 4 | list cell vertical padding |
| `icon_size` | 16 | list/toolbar icon box |
| `bar_size` | 14 | scrollbar thickness |

Button hit target: keep the whole control at least `control_height` tall and
prefer ≥ 24px; Apple recommends 44pt for touch, which we approximate for a
pointer-driven desktop toolkit. Keep a set of choices (Cancel/Open) at the same
size.

`control_height` is a **target/minimum**; the actual row height follows the
`font_ui` metrics. Inputs and buttons in the same toolbar should end up within a
couple of pixels of each other (use `control_padding` and small label gaps).

## 4. Typography

| Theme key | Default | Use |
|-----------|---------|-----|
| `font_ui` | `"Sans-13"` | **all controls**: buttons, labels, inputs, lists, toolbars |
| `font_tiny` / `font_small` | 9 / 11 px | secondary/status text |
| `font_normal` | 13 px | body text |
| `font_large` | 20 px | emphasized values |
| `font_heading2` / `font_heading1` | 18 / 24 px | panel/dialog headings |

Rules:

- One UI font/size (`font_ui`) for every control. Do not mix 12/13/16/22 by
  accident; editors may choose their own content size but their chrome uses
  `font_ui`.
- Monospace only for code/editors (and only via an explicit content resource).
- Buttons use title-style capitalization; an action that opens another view gets
  a trailing ellipsis ("Open…").

## 5. Colour roles

Group roles so widgets share a palette. Each widget falls back to the general
roles when a specific role is undefined (documented per widget).

**Surfaces**

| Role | Meaning | Fallback |
|------|---------|----------|
| `bg_norm` | window/dialog background | — |
| `gridbox_bg` | layout container background | `bg_norm` |
| `button_bg` | flat button surface | `bg_norm` |
| `input_bg` | text field surface | `bg_norm` |
| `list_bg` | list body | `bg_norm` |
| `list_header_bg` | column header bar | `bg_hi` (legacy) |
| `list_alt_bg` | alternating row | `list_bg` |
| `scrollbar_thumb` | scrollbar thumb/grip | `list_header_bg` |

**Text**

| Role | Meaning | Fallback |
|------|---------|----------|
| `fg_norm` | primary text | — |
| `button_fg` | button label | `fg_norm` |
| `input_fg` | input text | `fg_norm` |
| `list_fg` | list text | `fg_norm` |
| `list_header_fg` | header text | `fg_norm` |
| `list_sel_fg` | selected row text | `fg_hi` |

**Accent**

| Role | Meaning | Fallback |
|------|---------|----------|
| `bg_sel` | selection surface | — |
| `bg_hi` | hover/active surface | — |
| `accent` | primary button / selection | `bg_hi` |
| `accent_fg` | text on accent | `fg_hi` |
| `list_sel_bg` | selected row surface | `bg_sel` |

**Borders / lines**

| Role | Meaning | Fallback |
|------|---------|----------|
| `control_border` | input/control outline | `gridbox_border` |
| `list_line` | list grid/separator | `control_border` |
| `gridbox_border` | layout container border | — |
| `borderwidth` / `bordercolor` | widget frame (flat style: 0/none) | — |

Rules:

- Never hard-code white backgrounds or `XtDefaultForeground` for inputs; always
  resolve from the theme.
- Flat style: buttons are filled rectangles with no bevel; inputs get a 1px
  `control_border` outline and no rounded corners.
- Selection text must contrast with `list_sel_bg`/`accent` (`accent_fg`).

## 6. Dialog / window anatomy

```
┌──────────────────────────────────────────────────────────────┐
│ toolbar:  [Up] [Home]   path label            [Filter…] [☰]  │  leading nav, trailing filter/actions
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  content: list / editor / form                               │  fills the middle (weighty 1)
│                                                              │
├──────────────────────────────────────────────────────────────┤
│                                         [Cancel]  [Open]     │  action bar, right aligned, primary trailing
└──────────────────────────────────────────────────────────────┘
```

- **Toolbar at the top**, one row. Leading = navigation (Up/Home/Back, view
  title/path). Trailing = search/filter and the primary action. Group with
  `control_gap`; separate text-labelled buttons so they don't run together.
- **Content fills the remaining space** (`weighty 1`, `fill 3`); it scrolls
  under a fixed header where applicable.
- **Action bar at the bottom**, right-aligned: secondary (Cancel) left of the
  primary (Open/OK). The primary uses `accent`/`accent_fg`.
- **Outer margin** `window_margin` (16) on all sides; panel gaps `group_gap`.
  `Gridbox` uses one `defaultDistance` for both the outer margin and the
  inter-cell gap; a dialog that needs a 16px outer margin may therefore have
  larger inter-control gaps. Use a nested `Gridbox`/`VBox` when the inner gaps
  must be tighter.
- Avoid critical controls at the very bottom *except* the dismissal row.
- A help button, if any, goes bottom-left opposite the dismissal buttons.

## 7. Widget recipes

### Flat button (`Wbutton`)
- Surface `button_bg`, text `button_fg`, font `font_ui`, height `control_height`.
- No bevel/border. Hover/armed uses `bg_hi`/`fg_hi` (Wheel `colors_changed`).
- Primary variant: `accent` surface, `accent_fg` text, same size as siblings.

### Text input (`Wedit`)
- Surface `input_bg`, text `input_fg`, font `font_ui`, height `control_height`.
- Flat 1px `control_border`, `cornerRoundPercent = 0`, `lineWidth = 1`.
- Placeholder/empty value is fine; do not use a different background.

### List (`WListView`)
- Body `list_bg`/`list_fg`, alternating `list_alt_bg`, selection
  `list_sel_bg`/`list_sel_fg`, separators `list_line`.
- Header `list_header_bg`/`list_header_fg`, height `list_header_height`,
  1px `list_line` under it.
- Rows are `font_ui` line height + 2×`list_cell_pad_y` (default 4), with
  `list_row_height` (22) as a minimum floor; an explicit `rowHeight` wins.
  Cells are padded `list_cell_pad_x`/`y`, icons use `icon_size`. Deriving the
  row from the font keeps the gap correct when the UI font or DPI changes.
- Single font (`font_ui`) for header and cells; right-align numeric columns.

### Checkbox / toggle (`Wcheckbox`)
- Same font/surface as buttons; label `fg_norm`; checked state uses `accent`.

### Toolbar
- One row; leading navigation, trailing filter/primary action; `control_gap`
  between items; labels `fg_norm` or `button_fg`; no busy backgrounds.

## 8. Icons

- Monochrome glyphs (`folder`, `file`, `up`, `home`, …) sized `icon_size`,
  drawn as flat silhouettes that read on both light and dark surfaces.
- Prefer SVG assets under `LuaRunner/` (or XPM under `XPM/`).
- Keep a fixed icon box; centre within it and pad by `XS`.
- Until SVG tinting is available, use neutral gray/white glyphs that work on
  both light and dark themes.

## 9. Theming implementation notes

- Text widgets resolve `font_ui` in `initialize` **and** `colors_changed`, so a
  live theme switch updates the UI font. Editor content may keep its own size.
- Colour changes go through the existing `colors_changed` hook; non-`Wheel`
  widgets register with `theme_register`/`theme_unregister` (see CONVENTIONS §30).
- Metrics are read once per layout (`theme_int_defined`/`theme_get_int`) and
  cached; invalidate the widget's cache in `colors_changed`.
- Every new role must be added to **all** themes in `themes.hdf`, with a sensible
  fallback chain for old/partial themes.
- Migration status: the modern text widgets (`Wlabel`, `Wbutton`, `Wcheckbox`,
  `Wedit`, `WeditMV`, `WListView`, `Wbravo`, `Wretex`) resolve `font_ui` and the
  new roles. The legacy Xft list widgets (`Wlist`, `Wlist4`, `WlistMulti`,
  `Wradio`, `EqFader`, `Gauge`, `WmenuPopup`, `Wcombo`) still use the `Wheel`
  `xftFont` default (`Sans-22`); migrating them to `font_ui` is a follow-up.
  Lowering the `Wheel` default destabilises `WlistMulti` (see `know_bugs.md`).

## 10. Dialogs

Dialogs interrupt the main task and must be easy to understand and dismiss.
For the LLM-facing decision rules, patterns, microcopy and self-check, see
`dialog-guide.md`; this section is the design reference it builds on.

### Types and when to use them
| Type | Use when | Behaviour |
|------|----------|-----------|
| **Modal dialog** | The task cannot continue until the user answers | Blocks its parent window; Cancel/OK always available |
| **Sheet / attached dialog** | The dialog belongs to one window (file open/save, rename) | Attached to the window, dims the parent, blocks only that window |
| **Alert** | Reporting a problem or confirming a destructive action | Short message + at most 2–3 buttons |
| **Modeless panel** | Inspector / palette used alongside editing | Stays open, follows the selection, never blocks |

Prefer a **sheet** for document/window-scoped work and a **modal dialog** only
when there is no sensible parent. Avoid stacking dialogs; if a dialog opens
another view, close the first or make it a sheet on the same window.

### Anatomy
```
┌───────────────────────────────────────────────┐
│ Title / question (font_heading2)              │  title
│                                               │
│ Explanatory text, wraps; optional form:       │  body
│   Label       [ input field            ]      │
│   Label       [ input field            ]      │
│                                               │
│                          [Cancel]  [Primary]  │  action bar
└───────────────────────────────────────────────┘
```

- **Title**: one short question or action ("Open", "Rename", "Delete file?").
  Use `font_heading2`/`font_heading1`; do not repeat the app name.
- **Body**: one or two sentences of context. Provide the *why* before the
  controls. Keep text width readable; wrap, never horizontal-scroll.
- **Form rows**: one label per field; labels and fields aligned on a common
  leading edge or label above field. Keep the label→field gap at `S` (8).
  Group fields with `group_gap` (12); do not show a grid/border around each row.
- **Action bar**: bottom-right; secondary buttons leading, **one** primary
  trailing. Use `action_gap` (8) between buttons, `window_margin` to the edges.
- **Sheet frame**: a transient shell (`transientShellWidgetClass` +
  `XtNtransientFor`) attached to the parent. Xt has no per-window grab; use
  `XtGrabExclusive` when the sheet must block the application.

### Buttons and keyboard
- **One primary button** (`primary` / `accent`), title-style capitalized, with a
  verb ("Open", "Replace", "Save"). Same height/size as its siblings.
- **Cancel** is always present, left of the primary in this toolkit. Escape
  triggers Cancel; Return/Enter triggers the primary. Tab order follows reading
  order, top → bottom, leading → trailing.
- **Destructive** actions (Delete/Overwrite) use a distinct danger style, are
  never the default/primary, and require explicit confirmation.
- Buttons that open another view get a trailing ellipsis ("Choose…", "Open…").
  The dialog's own primary that completes the task does not ("Open", "Save").
- Keep the whole control at `control_height` and the dialog width comfortably
  under the parent; avoid resizing the dialog per state.

### Labels, inputs and validation
- A field always has a visible label; a placeholder is a hint, not a label.
- `input_bg`/`input_fg` + 1px `input_border`, `font_ui`, flat (no bevel).
- Validate **inline**, next to the field that failed, not in a distant status
  bar; keep the dialog open and focus the first invalid field.
- Disable the primary only when the reason is obvious; otherwise allow the
  click and explain the error near the field.
- Keep typed content when validation fails; never clear the form.

### Open/Save (file chooser)
Use the file-chooser recipe (§6) inside a sheet:
- **Toolbar**: navigation (`Up`/`Home`) leading, current location, filter/search
  trailing, view toggles trailing.
- **Content**: `WListView` with Name/Size/Date/Type, folders first, header row,
  alternating rows, selection highlight, icons for folder/file.
- **Action bar**: Cancel + primary ("Open" / "Save") trailing. The button that
  opens the sheet is labelled "Open…"; the sheet's primary is "Open".
- For Save, include the filename field at the top of the action area, focused
  by default, with the extension visible and selectable for replacement.
- Put locations/shortcuts in a sidebar when the dialog is large enough; hide it
  in compact sizes rather than shrinking the list to unusable width.

### Dialog checklist
- [ ] One clear title and one primary action; Cancel always available
- [ ] Primary trailing, secondary/Cancel leading; destructive not default
- [ ] `window_margin`/`group_gap`/`control_gap` used consistently
- [ ] Labels visible, fields themed, validation inline
- [ ] Return = primary, Escape = cancel, reading-order tab chain
- [ ] Fits the smallest supported window; wraps/scrolls instead of clipping
- [ ] No command exists only in the dialog if it also belongs in a menu

## 11. Main application windows

A main window is the durable workspace; layout should stay stable while the
content changes.

### Anatomy
```
┌─────────────────────────────────────────────────────────────┐
│ toolbar: [nav] [title]        [search]   [tools] [primary]  │  top, one row
├──────────┬───────────────────────────────────────┬──────────┤
│ sidebar  │ content                               │ inspector│  optional panes
│ (nav)    │ (list / editor / canvas)              │ (details)│
│          │                                       │          │
├──────────┴───────────────────────────────────────┴──────────┤
│ status / hints (secondary, never critical controls)          │  bottom
└─────────────────────────────────────────────────────────────┘
```

- **Top toolbar**: navigation/leading, title, search and the primary action
  trailing. A window needs at most one primary action; everything else is
  secondary. Collapse overflow actions into a menu when the window narrows
  (apple HIG: use the system overflow rather than crowding).
- **Sidebar** (optional): navigation between areas/collections; selected item is
  highlighted; collapsible; give it a minimum width (`~180–240px`) and remember
  the user’s width.
- **Content**: the main task surface, `weighty 1`, fills the middle. Lists get a
  fixed header + scrolling body; editors scroll with the caret kept visible.
  Empty states explain what to do, not just "no data".
- **Inspector** (optional): details/properties for the current selection;
  trailing, collapsible, min width, follows selection without stealing focus.
- **Status bar**: secondary information only. Do not put critical controls or
  important state at the very bottom of a window; users resize windows past the
  bottom edge. A single status line with `font_small`/`fg_norm` is enough.

### Window sizing and adaptivity
- Provide a comfortable default size (for a document/list window: roughly
  900×600; dialogs 360–520 wide). Set a sensible **minimum** size so controls
  never overlap or clip; allow the window to grow freely.
- Use compact/regular widths rather than device assumptions: below a threshold,
  hide the sidebar/inspector into a button/menu, stack panels vertically, and
  keep the primary action visible.
- Remember size, position, split fractions and sidebar visibility per window
  kind. Never use min/max size to prevent resizing; use it to keep a usable
  layout.
- Split views: set a sensible initial fraction (e.g. 25% sidebar), enforce child
  minimums, and make the divider obvious and keyboard-adjustable.

### Menus and commands
- Every toolbar command must also exist in a menu (the toolbar can be hidden or
  customized). Group menus by function: File / Edit / View / Format / Window /
  Help.
- Use familiar shortcuts (Ctrl/Cmd+Z/Y, X/C/V, B/I/U, S, O, N, W, F, Q) and show
  them in menu labels. Do not invent shortcuts for infrequent actions.
- Menus toggle/disable with context; a disabled item explains *why* via a
  tooltip/status hint where possible.
- A burger/menu button is fine on small or utility windows; on a desktop main
  window prefer a real menu bar/toolbar.

### Lists and content
- Fixed column header; sortable columns indicate the active sort; click target
  is the whole header cell/row, not just the text.
- Single-click selects, double-click/Return activates; show the selection with
  `list_sel_bg`/`list_sel_fg`, not by moving text.
- Keep row/header heights, cell padding and `font_ui` consistent; right-align
  numeric columns; truncate with an ellipsis and expose the full value via a
  tooltip or inspector.
- Scroll under a fixed header; never let scrolled rows paint over the header.
- Preserve the selection and scroll position across relayouts/theme changes.

### Focus, keyboard and shortcuts
- Put initial focus in the content or the search/filter field, not on a
  destructive button.
- Provide a visible focus indication (Wheel colours/`fg_*`); the Tab chain
  follows reading order and does not jump between panes unexpectedly.
- Movement keys work in the focused list/editor; Page/Home/End behave as users
  expect. Escape clears transient states (menu, search) before closing anything.

### Theme, contrast and polish
- Window/dialog surface: `bg_norm`/`gridbox_bg`; controls: `button_*`;
  inputs: `input_*`; content: `list_*`/editor roles; borders from `control_border`
  / `list_line`; one accent (`accent`/`accent_fg`) for the primary action and
  selection.
- Verify both a light and a dark theme. No hard-coded white inputs, black text
  or per-widget literal colours; every resolved role must be used.
- Keep hit targets ≥ `control_height` (≥ 24px; Apple targets 44pt for touch),
  and avoid placing controls so close that hover/focus effects overlap.
- Use progressive disclosure (disclosure triangles, menus, collapsible panels)
  instead of overcrowding a single window.

### Main-window checklist
- [ ] One primary action; toolbar commands also in menus
- [ ] Sidebar/inspector optional, collapsible, with minimum widths
- [ ] Content fills and scrolls; fixed header never overdraws
- [ ] Minimum/default window size set; layout adapts when narrow
- [ ] Selection, scroll and pane fractions preserved across relayouts
- [ ] Focus starts in content/search; Tab order follows reading order
- [ ] Light + dark themes verified with the shared role map
- [ ] No critical state only at the very bottom of the window

### Toolkit mapping
| Need | Widget / mechanism |
|------|--------------------|
| Window/sheet frame | `WAppWindow`, `topLevelShellWidgetClass` popup shell, `WfileChooser`/`WfileSelector` |
| Toolbar / action bar | `WToolBar` with `Wbutton`/`WAction` (legacy: `Gridbox` row) |
| Status bar | `WStatusBar` |
| Form | `WFormLayout` |
| Dialog | `WDialog` |
| Selection | `WComboBox` |
| Single-line input | `WLineEdit` (legacy: `Wedit`) |
| Choice | `WRadioGroup` |
| Feedback | `WProgressBar` |
| Menu bar / burger menu | `WMenuBar` + `WmenuButton` + `WPopupMenu` (modern); `WmenuButton` + `WmenuPopup` legacy |
| Multi-pane main window | `Gridbox`, `Wsplitter`/`WPaned`, `Wviewport` |
| Tabs | `WtabWidget` |
| List/content | `WListView`, `table-view`, `WTreeView`, `WlistMulti`, `Wbravo`/`Wretex` |
| Scrollable page | `Wviewport` (clip + `VSlider`/`HSlider`) |
| Theme/metrics | `font_ui`, `button_*`, `input_*`, `list_*`, `accent`, `control_*`, `window_margin` |
