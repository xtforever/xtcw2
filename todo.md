# TODO — Wbravo rich-text editor (next steps)

The rich-text foundations are in place (atom document model, styled layout,
WYSIWYG text styles). These are the remaining features, starting with inline
objects.

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
