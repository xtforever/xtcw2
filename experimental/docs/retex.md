# Retex Layout Engine

## Overview

Retex is a TeX-inspired text layout engine used by Wlabel and WlistMulti for rendering formatted text with FreeType fonts.

## API Reference

### retex_layout()

```c
RetexParagraph* retex_layout(Backend *be, const char *text, 
    double width_pt, const char *font_face, double font_size_pt, RetexAlign align);
```

Lays out text into a paragraph, wrapping at `width_pt` (in points).

**Parameters:**
- `be`: Backend for font rendering (e.g., `backend_xpixmap_create`)
- `text`: Input text (can include SVG commands like `\includesvg{...}`)
- `width_pt`: Target width for text wrapping, in points
- `font_face`: Font family name (e.g., "Sans", "Serif")
- `font_size_pt`: Font size in points
- `align`: Alignment (RETEX_ALIGN_LEFT, RETEX_ALIGN_CENTER, RETEX_ALIGN_RIGHT, RETEX_ALIGN_JUSTIFY)

**Returns:** Opaque handle to laid-out paragraph, or NULL on failure.

**Important:** Layout depends on `width_pt`. Changing widget width requires re-layout.

---

### Measuring Bounds

```c
double retex_paragraph_get_height(RetexParagraph *para);
double retex_paragraph_get_width(RetexParagraph *para);
double retex_paragraph_get_first_line_height(RetexParagraph *para);
```

- `get_width()` returns max line width (not paragraph width)
- `get_first_line_height()` returns baseline offset from top of paragraph

---

### Rendering

```c
void retex_paragraph_render(RetexParagraph *para, Backend *be, 
    double x, double y, const RendererColors *colors, int reverse_mode);
```

**Critical:** The `(x, y)` coordinates specify the **baseline** of the first line, NOT the top-left corner. This is why `retex_paragraph_get_first_line_height()` must be added to the y position when rendering.

---

## Coordinate Transformation

Xt widgets use pixels; Retex uses points (1pt = 1/72 inch).

```c
double pt2px(Widget w, double pt) {
    return pt * get_dpi(w) / 72.0;
}

double px2pt(Widget w, double px) {
    return px * 72.0 / get_dpi(w);
}
```

DPI defaults to 96 if not set in X resources (`Xft.dpi`).

---

## Usage in Wlabel

1. **Measurement** (in `query_geometry` or `initialize`):
   ```c
   double target_width_pt = px2pt($, inner_w_px);
   void *para = retex_layout(be, $label, target_width_pt, $fontFace, $fontSize, $alignment);
   *w = (int)ceil(pt2px($, retex_paragraph_get_width(para))) + gaps;
   *h = (int)ceil(pt2px($, retex_paragraph_get_height(para))) + gaps;
   ```

2. **Rendering** (in `update_cache`):
   ```c
   double x_pt = px2pt($, $leftGap) + h_offset_pt;
   double y_pt = px2pt($, $topGap) + v_centering + retex_paragraph_get_first_line_height(para);
   retex_paragraph_render(para, be, x_pt, y_pt, &colors, reverse_mode);
   ```

---

## Alignment Calculation

When text width < widget width, Retex returns natural text width. Widget must offset for alignment:

```c
// Center
h_offset_pt = (widget_width_pt - text_w_pt) / 2.0;

// Right  
h_offset_pt = widget_width_pt - text_w_pt;
```

---

## Common Issues

1. **Text not re-layout on resize**: Must call `retex_layout()` again when width changes
2. **Wrong vertical position**: Remember to add `get_first_line_height()` to y coordinate
3. **Subpixel truncation**: Use `ceil()` when converting pt→px to avoid clipping