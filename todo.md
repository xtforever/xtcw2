# TODO — Wbravo rich-text editor (next steps)

The rich-text foundations are in place (atom document model, styled layout,
WYSIWYG text styles). These are the remaining features, starting with inline
objects.

## Paragraph formatting (branch `feature/wbravo-rich-editor`)

- [x] Paragraph model: a blank line separates paragraphs; a single `\n` is a
      soft line break inside the same paragraph.
- [x] retex: `RetexParaFormat`, `retex_layout_styled_paras()` and
      `retex_paragraphs()` (shared paragraph indexing).
- [x] Per-paragraph alignment (left/center/right/justify), left/right/first-line
      indentation, and space before/after (vertical kerns).
- [x] Wbravo `ParaFmt` array, edit-time remapping (split/merge by position) and
      undo/redo snapshots paired with the existing caret stacks.
- [x] Actions: `align_{left,center,right,justify}`, `indent`/`outdent`,
      `indent_right`/`outdent_right`, `indent_first`/`outdent_first`,
      `set_space_before`/`set_space_after`.
- [x] WmenuButton Format menu (Style / Paragraph tabs) in
      `demos/demo_rich_bravo.lua`.
- [x] Tests: `tests/test_retex_styled.c` (paragraph model/alignment/indent/
      spacing) and `tests/test_rich_wbravo.c` (actions, undo, remap).
- [ ] Hanging (negative first-line) indent: `first_line` is currently clamped at
      0; add a negative leading-kern path if needed.
- [ ] Paragraph-format "painting" on select-all / paste of multiple paragraphs
      (splitting an existing paragraph currently inherits the containing format).

## External action interface

- [x] All editing/formatting commands are Xt action procs, callable from other
      applications with
      `XtCallActionProc(w, "name", NULL, params, nparams)`.
- [x] Document: `set_text`, `insert_text`, `insert_newline`,
      `delete_backward`/`delete_forward`, `undo`/`redo`,
      `cut_selection`/`copy_selection`/`paste`.
- [x] Caret/selection: `set_caret` (code points), `set_caret_byte` (UTF-8
      bytes), `set_selection` (anchor, caret), `select_all`, movement/paging.
- [x] Text style: `bold`/`italic`/`roman` (toggles), `set_bold`/`set_italic`
      (explicit), `set_font`, `set_size`.
- [x] Paragraph: `align_*`, `set_alignment` (name or 0..3), `indent`/`outdent`
      family, `set_indent_left`/`set_indent_right`/`set_first_line_indent`,
      `set_space_before`/`set_space_after`.
- [ ] Read-back: actions are void, so query caret/selection/paragraph state via
      the `text` resource today; add explicit read resources or a query
      callback if external callers need them.

## Full-state snapshots (debugging)

- [x] `save_state <path>` / `load_state <path>` actions write/read a binary
      snapshot of the whole editor: rich document (atoms + looks), paragraph
      formats, caret/anchor/scroll, font face/size, gaps, pending look and the
      complete undo/redo history (rtable piece snapshots + caret/para entries).
- [x] `rtable_serialize()` / `rtable_deserialize()` cover buffers, pieces and
      undo/redo spans; faces and object payloads are stored as strings so a
      snapshot can be loaded in another process.
- [x] Demo "State" tab: save/load `/tmp/wbravo_state.snap`.
- [x] Test: `tests/test_wbravo_state.c` (round-trip, byte-identical re-save,
      undo/redo replication).
- [ ] Optional: file chooser instead of the fixed demo path.

## Demo plain-text I/O

- [x] `demo_rich_bravo.lua` Text tab: import/export plain UTF-8 files using an
      editable path field. Blank lines separate paragraphs; single newlines are
      soft breaks. Import resets rich formatting and normalises CRLF/CR to LF;
      export writes the text only, so all formatting is lost.

## ListView / file chooser

- [x] `WListView` (Composite): themed multi-column list with header, weighted or
      fixed columns, text/icon/icon+text cells (SVG and XPM), alternate rows,
      grid lines, vertical + horizontal `VSlider`/`HSlider`, wheel scrolling and
      keyboard navigation. Single selection with `notify`/`activate` callbacks.
- [x] `WfileChooser`: directory listing with Name/Size/Date/Type columns,
      filter, hidden-file toggle, Up/Home navigation, OK/Cancel and
      `accept`/`cancel` callbacks.
- [x] `WfileSelector` now pops a `WfileChooser` instead of `SelectReq`; exposes
      the accepted full path as `selectedPath`.
- [x] LUI: `listview`/`file-chooser` aliases, `model` maps to `WListView` rows,
      `on-activate`/`on-header` callback aliases; `gui.set_string_list`.
- [x] Demos `demo_listview.lua`, `demo_filechooser.lua`; tests
      `test_wlistview`, `test_wfilechooser`.

## Style guide / whole-toolkit theming

- [x] `style-guide.md`: principles, spacing scale, control
      metrics, typography, colour-role map, dialog anatomy and widget recipes
      (based on Apple HIG Layout/Toolbars/Buttons).
- [x] Theme keys in every `themes.hdf` theme: `font_ui`, `button_*`, `input_*`,
      `list_*`, `accent`, `window_margin`, `control_*`, list metrics.
- [x] Helpers `theme_font_spec`/`theme_font_parts`/`theme_metric`.
- [x] `Wheel` control/primary roles; `Wlabel`/`Wbutton`/`Wcheckbox` resolve
      `font_ui` + button roles; `Wedit`/`WeditMV` resolve input roles + font;
      `WListView` resolves font/metrics; flat 1px input border.
- [x] `WfileChooser` toolbar/content/action-bar layout, flat controls, primary
      OK, monochrome folder/file icons.
- [x] Regenerate label layout baselines; demo suite 73/73.
- [ ] Migrate the legacy Xft list widgets (`Wlist`, `Wlist4`, `WlistMulti`,
      `Wradio`, `EqFader`, `Gauge`, `WmenuPopup`, `Wcombo`) to `font_ui` and the
      style-guide roles. Lowering `Wheel.xftFont` currently destabilises
      `WlistMulti` (see `know_bugs.md`).
- [ ] `test_wlistmulti`'s self-checking binary is intermittent under
      `MLS_DEBUG`: `render_retex_row()` occasionally corrupts the MLS free list
      (`s_split` → `_m_put`). Pre-existing/legacy WlistMulti issue; keep its
      font at 22 until the renderer is reworked.

## Inline SVG objects

- [ ] Implement `RATOM_OBJECT` handling in `retex_layout_styled()` (it currently
      emits a zero-width placeholder rule).
- [ ] Add an object-builder dispatch table keyed by `obj_type`
      (`ROBJ_SVG` → `node_create_svg`) so new object types are additive.
- [ ] Give an SVG object a display size (e.g. width/height in points) and make
      it participate in word/line breaking (advance + line height).
- [ ] Add rtable object edit ops (insert/replace/remove an object atom) with
      undo coverage, mirroring the text/style ops.
- [ ] Wbravo action + toolbar button "Insert SVG" (file picker + size) that
      inserts an `RATOM_OBJECT`/`ROBJ_SVG` atom.
- [ ] Caret/selection: an object is one document position — verify
      `get_caret_pos` / `get_offset_at` / Up-Down move over it cleanly.
- [ ] Test: `tests/test_rich_objects.c` (insert SVG atom, layout, caret).
- [ ] Demo: extend `demos/demo_rich_bravo.lua` with an "SVG" button.

## Inline math objects

- [ ] Represent an inline math span as `RATOM_OBJECT` with an `ROBJ_MATH`
      payload (interned math-source string handle).
- [ ] Reuse the existing retex math builder (tokenize/build math nodes) when the
      styled layout encounters an `ROBJ_MATH` atom; feed width/height into line
      breaking.
- [ ] Add an action / demo button to insert an inline math object and edit its
      source.
- [ ] Test: `tests/test_rich_math.c` (layout a math span, caret over it).

## Follow-ups / polish

- [ ] Rich clipboard: copy/paste selection as text **and** runs (styles), not
      just the UTF-8 text.
- [ ] Document serialization: save/load atoms + looks (persist a rich document).
- [ ] Font face/size pickers in the demo (beyond the fixed toolbar buttons).
- [ ] `set_values` handling of formatting-related resources.
- [ ] Verify all of the above under the release build (`make debug_enable=0`).

## Build / CI (found during clean-container verification)

- [ ] **Layout trace baselines are font-environment specific.** `make all` builds
      cleanly in `ci/Dockerfile` (Ubuntu 24.04), but `test_wlabel`'s LAYOUT diff
      differs from the committed baseline (same font file/version, but Ubuntu's
      fontconfig hinting gives a slightly different advance at the small math
      size). It passes on the current dev host. Make the comparison robust
      (structure-only or a tolerance) or regenerate baselines in the canonical
      CI image.
- [ ] `test_wlabel_selection` baseline is stale — it fails on the dev host too.
- [ ] `test_wlistmulti` LAYOUT baseline is flaky (pre-existing uninitialized
      widths in WlistMulti).
- [ ] `make -C experimental/karo_widget test` fails to build `xkey.o`.
- [ ] CI currently uses `submodules: recursive`; nanosvg is vendored now, so
      that can be dropped from `.github/workflows/ci.yml`.
