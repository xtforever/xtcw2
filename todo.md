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
