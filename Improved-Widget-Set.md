# Improved Widget Set

A prioritized, buildable widget roadmap for **xtcw2**: a high-speed, C-based
Xt widget set with modern desktop features and first-class **Lua/LUI rapid
development**.

This is a living document. It is derived from:

- `style-guide.md` — layout, spacing, typography, colour roles, dialog anatomy
- `cookbook/` — the reference application (tabs, lists, forms, editors, dialogs)
- `tools/t1_menu.lua` — the launcher (lists, rich descriptions, actions)
- `demos/` — the widget/demo matrix (`WListView`, `WtabWidget`, `KaroEd`,
  `Wbravo`, `Wretex`, `EqFader`, `Wviewport`, …)
- `know_bugs.md` / `todo.md` — known defects, legacy gaps and follow-ups
- the current `wbuild_widgets/`, `plainc_widgets/` and `lui/registry.lua`
- the Qt5/PyQt5 widget catalogue, used as a **feature reference only**

## Goals

1. **Fast by construction.** No per-cell child widgets in large lists/tables/
   trees; draw with Xft/cairo into a backing pixmap; virtualize rows; keep
   callbacks thin and Lua-friendly.
2. **Modern look without a heavy toolkit.** One theme registry
   (`themes.hdf`), style-guide roles/metrics/fonts, flat controls, dark/light
   palettes, high-DPI-aware text.
3. **Rapid Lua development.** Every widget gets a short LUI tag, sensible
   defaults, declarative resources, `LUA(...)` callbacks and a documented model
   interface. A typical form/dialog/list screen should be a few dozen lines.
4. **Xt-native.** Widgets remain `Widget`/`Composite`/`Constraint` classes with
   Xt resources and callbacks; no Qt dependency, no C++ runtime.
5. **Progressive migration.** New widgets replace legacy Xaw-era widgets via
   aliases; old classes stay available until the demos/cookbook/tests are
   migrated.
6. **Cookbook as testing ground.** Every new widget is integrated into the
   cookbook first, then gets a `tests/widgets/*.lui` probe and, where useful,
   a C regression test.

## Current inventory (short)

| Family | Current | Assessment |
|---|---|---|
| Base/theme | `Wheel`, `Wlabel`, `Frame`, `Gridbox`, `VBox`, `HBox` | Solid foundation; `Wheel` is the theme/input base |
| App shell | `WAppWindow` (new) | Themed 4-slot container: menu, toolbar, content, status |
| Menus | `WMenuBar`, `WPopupMenu`, `WAction` (new), `WmenuButton` | First modern menu slice with checkable actions; keyboard mnemonics pending |
| Toolbar | `WToolBar` (new) | Themed natural-width action row; cookbook integration done |
| Buttons | `Wbutton`, `Wcheckbox`, `WToggleSwitch`, `WpixBtn` | `Wbutton`/`WToggleSwitch` good; checkbox indicator incomplete; icon button legacy |
| Text input | `Wedit`, `WeditMV`, `Wpassword`, `KaroEd`, `Wbravo`, `Wretex` | `KaroEd`/`Wbravo` strong; `Wedit` is legacy; no validation/completion |
| Lists | `WListView` (modern), `Wlist`, `Wlist4`, `WlistMulti`, `Wls`, `WlsMulti` | `WListView` is the future; legacy lists must be frozen |
| Tabs/scroll | `WtabWidget`, `Wviewport`, `WSplitter`, `WPaned` | Modern after recent fixes; splitter/paned need consolidation |
| Menus | `Wmenu`, `WmenuButton`, `WmenuPopup` | Legacy menu internals; no menu bar/action model |
| Dialogs | `MessageBox`, `WfileChooser`, `WfileSelector` | File chooser improved; message box and input dialogs are legacy/missing |
| Feedback | `Gauge`, `VSlider`, `HSlider`, `EqFader` | Sliders good; no progress bar/status bar/tooltips |
| Data/domain | `WListView`, `DrawCanvas`, `Canvas`, `IconSVG`, `Board`, `Dartboard` | Good domain widgets; no table/tree/model abstraction |

## PyQt5/Qt5 comparison

Status: **Good** = usable and themed; **Partial** = exists but incomplete or
legacy; **Legacy** = works but should be replaced; **Missing** = not present.

| Qt / PyQt5 | Current Xt widget | Status | Improved-Widget-Set action | Priority |
|---|---|---|---|---|
| `QMainWindow` | shell + `Gridbox` + `WtabWidget` | Partial | **WAppWindow** (menu/toolbar/content/status) | P0 |
| `QDialog` | `MessageBox`, `WfileSelector` | Partial | **WDialog** (modal/modeless, buttons, focus) | P0 |
| `QLabel` | `Wlabel` | Good | keep; add link/rich helpers | — |
| `QPushButton` | `Wbutton` | Good | keep; add default/primary roles | — |
| `QToolButton` | `Wbutton`, `WpixBtn` | Partial | **WIconButton** (icon/text/toggle/menu) | P0 |
| `QRadioButton` | `Wradio`, `Radio2` | Legacy | **WRadioGroup** / themed radio | P0 |
| `QCheckBox` | `Wcheckbox` | Partial | improve check indicator + tri-state | P0 |
| `QComboBox` | `Wcombo` | Legacy/broken | **WComboBox** (editable, model, completion) | P0 |
| `QSpinBox` | `WspinBox` | Partial | improve +/- buttons, format, validation | P0 |
| `QDoubleSpinBox` | — | Missing | extend **WSpinBox** with double/step | P1 |
| `QLineEdit` | `Wedit` | Legacy | **WLineEdit** (validation, completion, clear) | P0 |
| `QTextEdit` | `Wbravo` | Partial | keep rich editor; add document format later | P1 |
| `QPlainTextEdit` | `KaroEd`, `WeditMV` | Partial | **WPlainTextEdit** (simple form editor) | P0 |
| `QTextBrowser` | `Wretex`/`Wlabel` | Good | keep; add link/scroll helpers | — |
| `QSlider` | `VSlider`, `HSlider` | Good/Partial | improve ticks/labels/hit area | P1 |
| `QScrollBar` | `VSlider`, `HSlider` | Good | keep; unify theming | — |
| `QProgressBar` | `Gauge` | Legacy | **WProgressBar** (determinate/indeterminate) | P0 |
| `QDial` | — | Missing | optional | P2 |
| `QCalendarWidget` | — | Missing | **WCalendarWidget** | P1 |
| `QDateEdit`/`QTimeEdit`/`QDateTimeEdit` | — | Missing | **WDateEdit**/`WTimeEdit`/`WDateTimeEdit` | P1 |
| `QLCDNumber` | — | Missing | optional | P2 |
| `QListView`/`QListWidget` | `WListView` | Good | keep; formalize model API | — |
| `QTableView`/`QTableWidget` | flat `WListView` only | Partial | **WTableView** + header/sort/filter | P0/P1 |
| `QTreeView`/`QTreeWidget` | — | Missing | **WTreeView** (lazy, keyboard) | P1 |
| `QHeaderView` | internal to `WListView` | Partial | reusable **WTableHeader** | P1 |
| `QAbstractItemModel` | MLS string rows | Partial | **WModel** (rows/cols/get/flags/sort/filter) | P0 |
| `QSortFilterProxyModel` | — | Missing | **WSortFilterModel** | P1 |
| `QStyledItemDelegate` | — | Missing | **WItemDelegate** (render/edit hooks) | P1 |
| `QCompleter` | — | Missing | **WCompleter** | P1 |
| `QValidator` | — | Missing | **WValidator** (int/double/regex) | P1 |
| `QMenuBar` | — | Missing | **WMenuBar** | P0 |
| `QMenu` | `Wmenu`, `WmenuPopup` | Legacy | **WMenu** (submenu/checkable/icons) | P0 |
| `QAction` | `Wbravo` actions / `LUA` callbacks | Partial | **WAction** (text/icon/shortcut/checkable) | P0 |
| `QToolBar` | `Gridbox` row of `Wbutton` | Partial | **WToolBar** (overflow, separators, tooltips) | P0 |
| `QStatusBar` | `label` | Partial | **WStatusBar** (message + permanent widgets) | P0 |
| `QShortcut` | Xt translations | Partial | action accelerators + mnemonics | P0/P1 |
| `QContextMenu` | — | Missing | **WContextMenu** | P1 |
| `QToolTip` | — | Missing | **WToolTip** | P0 |
| `QTabWidget`/`QTabBar` | `WtabWidget` | Good | keep; add closable/overflow tabs | P1 |
| `QStackedWidget` | — | Missing | **WStackedWidget** | P1 |
| `QSplitter` | `WSplitter`, `WPaned` | Partial | consolidate **WSplitter** | P1 |
| `QScrollArea` | `Wviewport` | Good | keep; add edge fades | P1 |
| `QGroupBox` | `Frame` | Partial | **WGroupBox** (title/checkable) | P1 |
| `QFrame` | `Frame` | Good | keep | — |
| `QDockWidget` | — | Missing | optional | P2 |
| `QMdiArea` | — | Missing | optional | P2 |
| `QToolBox` | — | Missing | optional | P2 |
| `QWizard` | — | Missing | **WWizard** on `WStackedWidget` | P1 |
| `QMessageBox` | `MessageBox` | Legacy | **WMessageBox** (themed, buttons, details) | P0 |
| `QFileDialog` | `WfileChooser`/`WfileSelector` | Good/Partial | **WFileDialog** polish (shortcuts, multi-select, overwrite) | P1 |
| `QColorDialog` | — | Missing | **WColorDialog** | P1 |
| `QFontDialog` | — | Missing | **WFontDialog** | P1 |
| `QInputDialog` | — | Missing | **WInputDialog** (text/int/double/choice) | P0 |
| `QProgressDialog` | — | Missing | **WProgressDialog** | P1 |
| `QClipboard` | `Wbravo`/`KaroEd` partial | Partial | **WClipboard** (text + rich runs) | P1 |
| `QDrag`/`QDrop` | — | Missing | **WDragDrop** for lists/editors | P1 |
| `QSettings` | `store/persistence.lua` stub | Partial | **WSettings** / Lua settings API | P1 |
| `QTimer` | `xtapptimeout` | Good | keep | — |
| `QProcess` | Lua `os.execute`/`popen` | Partial | optional **WProcess** helper | P2 |
| `QThread` | Xt single-thread | — | not planned; async helper only | P2 |
| `QGraphicsView`/`QGraphicsScene` | `DrawCanvas`, `Canvas` | Partial | extend canvas/scene API | P2 |
| `QOpenGLWidget` | — | Missing | optional | P2 |
| `QWebEngineView` | — | Missing | not planned (external if needed) | P2 |
| `QMediaPlayer` | — | Missing | not planned | P2 |
| `QSystemTrayIcon` | — | Missing | optional | P2 |
| `QAccessible`/high-DPI | partial | Partial | accessibility + HiDPI pass | P1/P2 |

## Improved Widget Set

### P0 — app shell and everyday controls

These unblock the cookbook, `t1_menu` and most Lua apps. Each gets a LUI tag,
style-guide roles, keyboard handling and tests.

1. **WAppWindow** — `NEW`
   - Shell layout: menu bar row, toolbar row, content (`WtabWidget`/`WStackedWidget`), status bar.
   - Resources: `title`, `menuBar`, `toolBar`, `statusBar`, `content`, `windowMargin`.
   - LUI: `(app-window …)` / `(main-window …)`.
   - Status: implemented as a themed slot container (`menu`, `toolbar`,
     `content`, `status`); the cookbook now uses it as its main window.
     The dedicated menu/toolbar/status widget classes follow in the next P0
     slices.
   - Acceptance: cookbook main window built without hand-coded outer `Gridbox`; live theme switch; Quit/close handling.

2. **WMenuBar / WMenu / WAction** — `NEW` + `IMPROVE`
   - Modern menu model: text, icon, shortcut, checkable/radio, submenu, separator, enabled.
   - Keyboard: Alt mnemonics, arrows, Enter, Esc; popup placement inside the screen.
   - Replace the internals of `Wmenu`/`WmenuPopup`; keep `WmenuButton` as the popup trigger.
   - Status: first slice implemented — `WMenuBar` (natural-width themed bar),
     `WPopupMenu` (themed vertical popup container), `WmenuButton` trigger
     width/height resources, and `WAction` (checkable/shortcut item). The
     cookbook uses a File/View/Help menu bar as the integration test. Keyboard
     mnemonics and full menu action styling remain.
   - Acceptance: cookbook/t1_menu menus; Wbravo Format menu uses `WAction`; keyboard-only navigation.

3. **WToolBar** — `NEW`
   - Icon/text actions, separators, overflow “»”, tooltips, primary/secondary.
   - Style roles: `button_*`/`toolbar_*`; height `control_height`/`toolbar_height`.
   - Status: first slice implemented — themed natural-width action row with
     `toolbarHeight`, `itemGap`, `toolbarPadding`; cookbook uses a New/Quit
     toolbar. Overflow, separators and tooltips remain.
   - Acceptance: cookbook header/toolbar and file chooser toolbar.

4. **WStatusBar** — `NEW`
   - Message area + permanent widgets (progress, theme, selection info).
   - LUI: `(status-bar :message "…")`; `gui.set(id,"message",…)`.
   - Acceptance: cookbook/t1_menu status line; progress integration.

5. **WComboBox** — `NEW` (replaces `Wcombo`)
   - Read-only and editable modes; item model; keyboard type-ahead; completion.
   - Popup list uses `WListView`; no Xaw popup shell.
   - Acceptance: category selection in cookbook; no widget-name fallback; theme switch.

6. **WSpinBox** — `IMPROVE`
   - Visible `-`/`+` buttons, repeat, wrap, step, min/max, `%d`/`%g` format.
   - Keep `value`/label sync (already fixed); add `set_value` action and validation.
   - Acceptance: cookbook servings/minutes; keyboard Up/Down/Home/End; buttons usable at `control_height`.

7. **WCheckBox** — `IMPROVE`
   - Draw the check indicator (accent), support tri-state, keyboard Space, mnemonic.
   - Acceptance: form/cookbook toggles; visual state matches `checked`.

8. **WRadioGroup / WRadioButton** — `NEW` (replaces `Wradio`/`Woption`)
   - Mutually exclusive group, arrow-key navigation, labels, optional descriptions.
   - Acceptance: form choice groups; themed focus/hover.

9. **WLineEdit** — `NEW`/`IMPROVE` (replaces `Wedit` internals)
   - Style-guide input surface/border, placeholder, clear button, password mode.
   - Validation hooks (`WValidator`), completion (`WCompleter`), IME/UTF-8.
   - Keep `Wedit` as a compatibility alias during migration.
   - Acceptance: cookbook search/title/author; invalid input feedback.

10. **WPlainTextEdit** — `NEW`
    - Simple multiline text editor with style-guide chrome, line/column status, undo/redo, clipboard.
    - `KaroEd` remains the code editor; `Wbravo` remains the rich editor.
    - Acceptance: cookbook ingredient/step editors without KaroEd complexity; large text performance.

11. **WModel** — `NEW` (list/table/tree model contract)
    - `rows`, `columns`, `get(row,col)`, `set`, `flags`, `sort`, `filter`, `header`.
    - Lua table-friendly: `gui.set_model(id, {…})`; MLS string rows remain a fast path.
    - Acceptance: `WListView`, `WTableView`, `WTreeView`, `WComboBox` share one model.

12. **WTableView** — `NEW`
    - Flat table: columns, header, sorting, selection, editing, virtualized rows.
    - Build on `WListView` rendering + `WModel`; no per-cell widgets.
    - Acceptance: recipe/ingredient/shopping tables; 10k rows smooth; keyboard navigation.

13. **WDialog / WMessageBox / WInputDialog** — `NEW`/`IMPROVE`
    - Modal/modeless dialog base with action bar, default/cancel, focus trap, Esc/Enter.
    - Themed message box (info/warning/error/question, details, checkbox).
    - Input dialog (text/int/double/choice) with validation.
    - Acceptance: cookbook delete confirmation, quit confirmation, file overwrite prompt.

14. **WProgressBar** — `NEW`
    - Determinate/indeterminate, text overlay, themed `progress_*` roles.
    - Acceptance: file loading/saving and long model operations.

15. **WFormLayout** — `NEW`
    - Label/field alignment, required marker, help text, section headings, `S`/`M` gaps.
    - Acceptance: cookbook editor form built with one form layout, aligned labels/fields.

16. **WToolTip** — `NEW`
    - Hover/focus tooltips with delay, screen-edge placement, theme roles.
    - Acceptance: toolbar/action/list tooltips; keyboard focus shows help.

17. **WIconButton / WToolButton** — `NEW`/`IMPROVE`
    - SVG/XPM icon + optional text, toggle, menu arrow, primary/secondary.
    - Acceptance: toolbar, file chooser, t1_menu actions; icon scaling `icon_size`.

18. **WSearchField** — `NEW`
    - Search icon, clear button, completion, Enter/`on-change` callback.
    - Acceptance: cookbook/t1_menu search; debounced filtering.

### P1 — data views, dialogs and platform features

- **WTreeView** — hierarchical model, expand/collapse, lazy children, keyboard.
- **WTableHeader** — reusable header, resize, sort indicator, alignment.
- **WSortFilterModel** — sort/filter proxy over `WModel`.
- **WItemDelegate** — cell renderer/editor hooks (text, icon, progress, custom Lua draw).
- **WCompleter / WValidator** — model-backed completion; int/double/regex validators.
- **WFileDialog** — polish `WfileChooser`: shortcuts, multi-select, overwrite confirm, recent places, keyboard.
- **WColorDialog / WFontDialog** — theme-aware pickers.
- **WDateEdit / WTimeEdit / WDateTimeEdit / WCalendarWidget** — themed date/time.
- **WProgressDialog** — progress + cancel.
- **WSplitter** — consolidate `WSplitter`/`WPaned`; keyboard resize, collapse.
- **WGroupBox** — titled/checkable group; use in forms.
- **WStackedWidget / WWizard** — page stacks and multi-step dialogs.
- **WContextMenu** — right-click menus wired to `WAction`.
- **WClipboard** — text + rich runs (Wbravo), selection owner, cut/copy/paste.
- **WDragDrop** — list/table/tree/editor drag source + drop target.
- **WSettings** — Lua settings API on top of the theme/persistence store.
- **WNotification/WToast** — transient status notifications.
- **WSeparator** — improve horizontal/vertical theming and spacing.
- **WTabWidget** — closable tabs, overflow, drag-reorder.

### P2 — advanced/optional

- **WDockWidget**, **WToolBox**, **WMdiArea**.
- **WGraphicsView/WGraphicsScene** (extend `DrawCanvas`/`Canvas`), **WOpenGLWidget**.
- **WWebView**, **WMediaPlayer**, **WSystemTrayIcon** (only if a real product needs them).
- **WLCDNumber**, **WDial**, **WCalendar** extras.
- **Accessibility**: accessible names/roles, focus rings, keyboard-only paths, screen-reader hooks.
- **High-DPI polish**: font metrics, icon scaling, tab strip, pixel snapping.
- **Input methods / RTL** where required.

## Cross-cutting platform features

These are part of the set even when they are not separate widgets:

1. **Theme roles for new surfaces** — add to every theme:
   `menu_bg/menu_fg/menu_sel`, `toolbar_bg/toolbar_fg`, `status_bg/status_fg`,
   `tooltip_bg/tooltip_fg`, `table_*`, `tree_*`, `progress_*`, `dialog_*`,
   `scrollbar_thumb` (already added).
2. **Metrics** — `menu_height`, `toolbar_height`, `status_height`,
   `dialog_padding`, `table_row_height`, `tree_indent`, `progress_height`.
3. **Model/view** — one `WModel` contract for lists, tables, trees, combos;
   virtualized rendering; O(1) row access; no per-cell widgets.
4. **Delegates** — default text/icon/number/progress delegates; Lua custom draw
   hook for domain cells.
5. **Validation/completion** — shared `WValidator`/`WCompleter` used by
   `WLineEdit`, `WSpinBox`, `WComboBox`, dialogs.
6. **Keyboard/focus** — tab order, mnemonics, accelerators, Enter/Esc,
   Home/End/PageUp/PageDown, type-ahead, focus ring.
7. **Clipboard/DnD/context menus** — common `WAction`/`WMenu` integration.
8. **Tooltips/help** — `WToolTip`; status-bar hints for hovered actions.
9. **High-DPI** — theme font metrics, icon size, tab strip, dialog placement.
10. **Lua/LUI ergonomics** — short tags, defaults, `:on-*` callbacks, model
    tables, `gui.set_model`, `gui.set_string_list`, live theme switch.
11. **Ownership/lifecycle** — borrowed strings, callback registration, destroy
    order, theme unregister; no leaks on `os.exit()`.
12. **Performance budget** — 10k-row list/table scroll at interactive rates;
    no per-row widget allocation; icon cache; lazy SVG decode.

## Legacy migration map

| Legacy | Replace with | Action |
|---|---|---|
| `Wlist`, `Wlist4`, `WlistMulti`, `Wls`, `WlsMulti` | `WListView` / `WTableView` / `WTreeView` | freeze; keep aliases for old demos |
| `Wcombo` | `WComboBox` | freeze; cookbook no longer uses it |
| `Wradio`, `Woption`, `Woptc`, `Radio2` | `WRadioGroup` | freeze; migrate demos |
| `Wmenu`, `WmenuPopup` | `WPopupMenu`/`WAction` | keep `WmenuButton` as popup trigger |
| `Wtext` | `WPlainTextEdit` | deprecate |
| `Wedit`, `WeditMV` | `WLineEdit`, `WPlainTextEdit` | keep `Wedit` alias during migration |
| `WpixBtn` | `WIconButton` | deprecate |
| `MessageBox` | `WMessageBox` | keep alias |
| `WfileSelector`/`WfileChooser` | `WFileDialog` | keep aliases; polish UX |
| `WSplitter`/`WPaned` | `WSplitter` | consolidate |
| `Gauge` | `WProgressBar` (plus analog gauge) | keep analog |
| `Wcheckbox` | improved `WCheckBox` | improve in place |

## Phases

1. **Phase 1 — P0 foundation.** `WAppWindow`, menus/actions, toolbar, status bar,
   combo, spin, check/radio, line edit, plain text, dialog/message/input,
   progress, form layout, tooltip, icon button, search field, `WModel` +
   `WTableView`.
2. **Phase 2 — P1 data/platform.** tree view, header/sort/filter, delegate,
   completer/validator, file/color/font/date dialogs, splitter/group box,
   stacked/wizard, clipboard/DnD/context menu, settings.
3. **Phase 3 — P2 polish.** dock/toolbox/MDI, graphics/OpenGL, accessibility,
   high-DPI, optional web/media/tray, tab overflow/reorder.
4. **Phase 4 — cleanup.** migrate cookbook/t1_menu/demos, freeze legacy widgets,
   remove dead Xaw dependencies, document the final set.

## Single-widget probe harness

`run_widget.sh` makes isolated widget checks cheap. It wraps one LUI widget in
the standard probe window:

- 700×600 window
- 2-row × 1-column `Gridbox`
- top host row fills all available space
- probe widget inside the top host
- small Quit button at the bottom right

```bash
./run_widget.sh '(Wlabel :label "Hello" :fontSize 20)'
./run_widget.sh tests/widgets/wlistview.lui
./run_widget.sh --after tests/widgets/populate_list.lua tests/widgets/wlistview.lui
./run_widget.sh --autoclose 500 '(Wlabel :label "probe")'
```

Hooks:

- `--setup FILE` — Lua file run before the probe is built (callbacks)
- `--after FILE` — Lua file run after the probe is built (models/data)
- `--result FILE` — write geometry/probe data as `key=value` lines
- `--autoclose MS` — exit automatically for CI

The first probe corpus lives in `tests/widgets/` and starts with the existing
P0 foundation widgets:

`Wlabel`, `Wbutton`, `WspinBox`, `Wcheckbox`, `WListView`, `WtabWidget`,
`Wviewport`, `KaroEd`, `Wretex`, `WToggleSwitch`, plus the new `WAppWindow`,
`WMenuBar`, `WPopupMenu`, `WAction` and `WToolBar` widgets.

As each new P0 widget lands, add a matching `tests/widgets/<widget>.lui` probe.
The harness is intentionally an interim shell; it can later switch to
`WAppWindow` while keeping the same probe interface.

## Testing / definition of done

Every new or improved widget needs:

- C unit test (`tests/test_<widget>.c`) for resources, callbacks, keyboard,
  model updates and destroy.
- A `tests/widgets/<widget>.lui` probe that runs through `run_widget.sh`.
- Lua/LUI smoke test and a demo or cookbook usage.
- Xvfb screenshot verification (layout/theme/contrast).
- Live theme switch (all style-guide roles).
- Keyboard-only path (Tab, arrows, Enter/Esc, mnemonics).
- High-DPI check (font metrics, icon scaling).
- Memory/lifecycle check (no leaks on destroy/`os.exit()`).
- `run_all_tests.sh` coverage.

## Open questions / assumptions

- Keep the `Wheel`/`Wlabel` base and `theme_register` model; new widgets should
  not invent a second theming path.
- `WModel` is a lightweight Lua-facing contract, not a full Qt model/view
  framework; virtualized C rendering is the performance path.
- `WTableView`/`WTreeView` build on the `WListView` drawing core rather than
  wrapping one widget per cell.
- `WPlainTextEdit` is intentionally simpler than `KaroEd`/`Wbravo`; it is for
  forms and notes, not code editing or rich documents.
- Qt-only features (web engine, media, OpenGL) are optional and only added if a
  product requires them.
