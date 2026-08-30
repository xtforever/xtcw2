# Widget Reference

> Canonical source: `wbuild_widgets/` (42 widgets). All other directories contain duplicates or legacy versions.

## Core Widgets (wbuild_widgets/)

### Display Widgets

| Widget | Class | Description | LUI Tag | TRACE(50) |
|--------|-------|-------------|---------|------------|
| Wlabel | `wlabelWidgetClass` | Rich text label with re-tex layout | `label`, `Wlabel` | ✅ info, select_start, select_extend, select_end |
| Wbutton | `wbuttonWidgetClass` | Push button | `button`, `Wbutton` | ❌ |
| WpixBtn | `wpixBtnWidgetClass` | Pixel button (image + callback) | `WpixBtn` | ✅ highlight, notify, reset, next_pixmap |
| IconSVG | `iconSVGWidgetClass` | SVG icon display | `image` | ❌ |
| Wtext | `wtextWidgetClass` | Text display | — | ❌ |
| WviewVar | `wviewVarWidgetClass` | Variable viewer | — | ❌ |
| WSeparator | `wSeparatorWidgetClass` | Separator line | `separator` | ❌ |
| Gauge | `gaugeWidgetClass` | Progress gauge | — | ❌ |

### List Widgets

| Widget | Class | Description | LUI Tag | TRACE(50) |
|--------|-------|-------------|---------|------------|
| Wlist | `wlistWidgetClass` | Single-column list | — | ❌ |
| Wlist4 | `wlist4WidgetClass` | 4-column list | `list4`, `wlist4`, `Wlist4` | ✅ highlight, reset, motion_start, motion_end, toggle_line_state_act, select_line |
| WlistMulti | `wlistMultiWidgetClass` | Multi-column list | — | ❌ |
| Wls | `wlsWidgetClass` | List with slider | — | ❌ |
| WlsMulti | `wlsMultiWidgetClass` | Multi-column with slider | `list-view` | ❌ |

### Input Widgets

| Widget | Class | Description | LUI Tag | TRACE(50) |
|--------|-------|-------------|---------|------------|
| Wedit | `weditWidgetClass` | Single-line editor | `edit`, `Wedit` | ❌ |
| WeditMV | `weditMVWidgetClass` | Multi-value editor | — | ❌ |
| Wcombo | `wcomboWidgetClass` | Combo box | `Wcombo` | ✅ SetKeyboardFocus, set_cursor, insert_char, remove_char, focus_in, focus_out, notify |
| Woption | `woptionWidgetClass` | Option buttons | — | ❌ |
| Wcheckbox | `wcheckboxWidgetClass` | Checkbox toggle | `check` | ❌ |
| WspinBox | `wspinBoxWidgetClass` | Numeric spinbox | `WspinBox` | ❌ |
| Wpassword | `wpasswordWidgetClass` | Password input | `Wpassword` | ❌ |
| Wradio | `wradioWidgetClass` | Radio buttons | — | ❌ |
| Radio2 | `radio2WidgetClass` | Radio buttons v2 | — | ❌ |

### Layout Widgets

| Widget | Class | Description | LUI Tag | TRACE(50) |
|--------|-------|-------------|---------|------------|
| Gridbox | `gridboxWidgetClass` | Weight-based grid layout | `grid`, `vertical`, `horizontal` | ❌ |
| VBox | `vBoxWidgetClass` | Vertical box | — | ❌ |
| HBox | `hBoxWidgetClass` | Horizontal box | — | ❌ |
| WPaned | `wPanedWidgetClass` | Paned window with grips | — | ❌ |
| Wsplitter | `wsplitterWidgetClass` | Two-pane splitter | `splitter`, `Wsplitter` | ❌ |

### Slider Widgets

| Widget | Class | Description | LUI Tag | TRACE(50) |
|--------|-------|-------------|---------|------------|
| HSlider | `hSliderWidgetClass` | Horizontal slider | — | ❌ |
| VSlider | `vSliderWidgetClass` | Vertical slider | `vslider` | ❌ |
| Woptc | `woptcWidgetClass` | Slider widget | — | ❌ |

### Container / Special Widgets

| Widget | Class | Description | LUI Tag | TRACE(50) |
|--------|-------|-------------|---------|------------|
| Frame | `frameWidgetClass` | Frame container | — | ❌ |
| Canvas | `canvasWidgetClass` | Cairo drawing surface | — | ❌ |
| ScrolledCanvas | `scrolledCanvasWidgetClass` | Scrolled canvas | `scrolled` | ❌ |
| Board | `boardWidgetClass` | Game board | — | ❌ |
| Dartboard | `dartboardWidgetClass` | Number board | — | ❌ |
| MessageBox | `messageBoxWidgetClass` | Message dialog | — | ❌ |
| Wheel | `wheelWidgetClass` | Scroll wheel | — | ❌ |
| Wmenu | `wmenuWidgetClass` | Menu button | — | ❌ |
| WmenuPopup | `wmenuPopupWidgetClass` | Popup menu | — | ❌ |
| Common | `commonWidgetClass` | Common resources | — | ❌ |
| SelectReq | `selectReqWidgetClass` | Selection request | — | ❌ |
| WfileSelector | `wfileSelectorWidgetClass` | File picker | — | ❌ |

### Plain C Widgets (plainc_widgets/)

| Widget | Class | Description | In libplainc.a | TRACE(50) |
|--------|-------|-------------|----------------|------------|
| Gridbox | `gridboxWidgetClass` | Grid layout | ✅ Yes | ❌ |

**Not linked** (missing symbols, broken includes):
- SelectW — compiles but link fails (`sig_send` undefined)
- SliderW — compile fails (`WheelWidgetClassInstance` undefined)
- Flip — compile fails (`XtNnormalFg` undefined)
- Ctrl — compile fails (same Wheel issues)
- MiniW — compiles but not linked
- IconLabel — compiles but not linked

### Xaw Standard Widgets (registered via XpRegisterAll)

| Widget | Class | Notes |
|--------|-------|-------|
| Command | `commandWidgetClass` | Must call set() before notify() |
| Toggle | `toggleWidgetClass` | — |
| List | `listWidgetClass` | — |
| Repeater | `repeaterWidgetClass` | — |
| Scrollbar | `scrollbarWidgetClass` | — |
| AsciiText | `asciiTextWidgetClass` | — |
| Paned | `panedWidgetClass` | — |
| Box | `boxWidgetClass` | — |

**Note**: Xaw widgets have no TRACE(50) instrumentation (upstream code).

## Widget Registration

### XtcwRegister (from wbuild)

Auto-generated by `wbuild_widgets/register-widgets.sh` → `build/source/register_wb.c`. Registers all 43 xtcw widget classes.

### XpRegisterAll (from wcl)

Registers Xaw standard widgets (Command, Toggle, List, Repeater, Scrollbar, AsciiText, Paned, Box) plus resource converters.

### LUI Registry (lui/registry.lua)

27 friendly tag mappings:

```lua
M.register('window', { class = 'topLevelShellWidgetClass' })
M.register('label', { class = 'Wlabel' })
M.register('Wlabel', { class = 'Wlabel' })
M.register('button', { class = 'Wbutton' })
M.register('Wbutton', { class = 'Wbutton' })
M.register('command', { class = 'command' })
M.register('toggle', { class = 'Toggle' })
M.register('edit', { class = 'Wedit' })
M.register('Wedit', { class = 'Wedit' })
M.register('grid', { class = 'Gridbox' })
M.register('vertical', { class = 'Gridbox' })
M.register('horizontal', { class = 'Gridbox' })
M.register('image', { class = 'IconSVG' })
M.register('WpixBtn', { class = 'WpixBtn' })
M.register('separator', { class = 'WSeparator' })
M.register('check', { class = 'Wradio' })
M.register('scrolled', { class = 'ScrolledCanvas' })
M.register('list-view', { class = 'WlsMulti' })
M.register('splitter', { class = 'Wsplitter' })
M.register('Wsplitter', { class = 'Wsplitter' })
M.register('vslider', { class = 'VSlider' })
M.register('list4', { class = 'Wlist4' })
M.register('wlist4', { class = 'Wlist4' })
M.register('Wlist4', { class = 'Wlist4' })
M.register('Wcombo', { class = 'Wcombo' })
M.register('WspinBox', { class = 'WspinBox' })
M.register('Wpassword', { class = 'Wpassword' })
```

Unknown tags fall back to `{ class = tag }`, so any registered Xtcw/Xaw class can be used by name as a tag, but will produce a "Warning: Unknown widget tag" message. Add entries to `registry.lua` to suppress the warning.

## Known Widget Bugs (from errors.md)

| Bug | Widget | Root Cause | Fix |
|-----|--------|-----------|-----|
| Text truncation | Wlabel | `int` cast of subpixel values | Use `ceil()` for pixel sizes |
| Unsigned underflow | Gridbox | `cell_size - margin - border` with `Dimension` (unsigned) | Use `int` intermediates, min 1px |
| Selection not rendering | Wlabel | `$dirty` not set in select actions | Add `$dirty = 1` before redraw |
| TRANSLATIONS syntax | Wlabel | `@` instead of `@trans` | Changed to `@trans` prefix |
| Property propagation | WlsMulti | `set_values` didn't forward to child | Added explicit `XtVaSetValues` calls |
| query_geometry missing | Woptc | No preferred width reported | Implemented `query_geometry` |
| s_split delimiter inclusion | mls.c | `strndup` included delimiter in result | Fixed length calculation |
| m_free_user pointer | m_tool.c | Passed address-of-pointer instead of pointer | Dereference check before free |
| `retexCells=true` crash | WlsMulti | `vas_printf` MLS crash | **Bug**: Don't use `:retexCells true`; omit it |
| `CallOnEnter` action warning | Wcombo | Action not registered by wbuild | Harmless — widget works without it |
| Edit `value` property | Wedit/Wpassword | Text stored in `label`, not `value` | Use `gui.get(id, "label")` or `gui.get(id, "value")` (falls back to `label`) |
| SpinBox `spinValue` | WspinBox | S-expression property mapped to `value` | Use `:value` or `:spinValue` (both work) |

## Verified Widget Status

Tested via `lui_test/run_widget_tests.sh` (19/19 passing):

| Widget | Created | Properties | Callback | Notes |
|--------|---------|------------|----------|-------|
| Command | ✅ | ✅ label get/set | ✅ (set→notify→unset) | Requires `set` before `notify` |
| Label | ✅ | ✅ label get/set | — | |
| WpixBtn | ✅ | — | ✅ | TRACE(50) verified |
| Wlist4 | ✅ | — | — | TRACE(50) verified |
| Wlabel | ✅ | ✅ label get/set | — | TRACE(50) on select_start |
| Wcombo | ✅ | ✅ label get/set | — | TRACE(50) verified, CallOnEnter warning |
| WspinBox | ✅ | ✅ value/min/max get/set | — | |
| Wedit | ✅ | ✅ label get/set, value→label fallback | — | Also `text` falls back to `label` |
| Wpassword | ✅ | ✅ label get/set | — | Text in `label` resource |
| Toggle | ✅ | ✅ state get | ✅ | |
| Gridbox | ✅ | — | — | |
| Separator | ✅ | — | — | |
| WSeparator | ✅ | — | — | |
| Splitter | ✅ | ✅ fraction get/set | — | |
| Wsplitter | ✅ | — | — | |
| IconSVG | ✅ | — | — | Needs valid SVG path |
| ScrolledCanvas | ✅ | — | — | |
| WlsMulti (list-view) | ✅ | ✅ model/tableStrs | — | **Bug**: crashes with `retexCells=true` |
| Image | ✅ | — | — | |