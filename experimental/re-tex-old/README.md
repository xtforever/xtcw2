# re-tex: Embeddable TeX-inspired Layout Engine

`re-tex` is a lightweight, box-based layout engine inspired by TeX's proven algorithms (Knuth-Plass line breaking, math spacing). It is designed to be reentrant, embeddable, and tightly integrated with the **Cairo** graphics library.

Unlike traditional TeX, `re-tex` does not rely on legacy TFM/PK files or global state. It uses modern system fonts via FreeType/Cairo and provides a clean C API for integration into existing applications.

---

## Key Features

*   **Advanced Typography**: Implements the Knuth-Plass paragraph breaking algorithm for optimal line breaks.
*   **Math Mode**: Support for LaTeX-style math formulas (e.g., `$E=mc^2$`) with TeX-compliant spacing.
*   **Vector Rendering**: Native support for Cairo and SVG integration.
*   **Reentrancy**: No global state; all context is passed explicitly, allowing for zero-subprocess integration.
*   **Safe Memory Management**: Built on the `memc` (mls) library for robust handle-based memory handling.

---

## Architecture Overview

The `re-tex` pipeline consists of five major stages:

1.  **Tokenizer**: Converts a UTF-8 string into a stream of tokens (characters, commands, groups).
2.  **Stomach (Builder)**: Processes tokens into a horizontal list (`hlist`) of nodes (glyphs, glue, penalties).
3.  **Layout Engine**:
    *   **Paragraph Builder**: Breaks the `hlist` into lines of a specific width.
    *   **Math Engine**: Converts math lists (`mlist`) into horizontal lists.
4.  **Box Tree**: The final geometric representation of the laid-out text.
5.  **Backend/Renderer**: Traverses the box tree and issues drawing commands to the chosen backend (Cairo, X11, etc.).

---

## Usage Guide

### 1. Initialize Memory
`re-tex` uses the `mls` library. You must initialize it before use:

```c
#include "mls.h"
m_init();
conststr_init();
```

### 2. Create a Backend
You need a `Backend` instance to handle font metrics and drawing. For Cairo:

```c
#include "backend_cairo.h"
Backend *be = backend_cairo_create(cairo_context);
```

### 3. Layout Text
Use `retex_layout` to create a `RetexParagraph` object.

```c
#include "retex.h"
RetexParagraph *para = retex_layout(
    be, 
    "Hello \bf{World}! This is e-tex.", 
    400.0,          // Width in points
    "Sans",         // Font face
    12.0,           // Font size in points
    RETEX_ALIGN_JUSTIFY
);
```

### 4. Render and Free
```c
retex_paragraph_render(para, be, x, y);
retex_paragraph_free(para);
be->destroy(be);
```

---

## Supported Commands

`re-tex` supports a variety of LaTeX-inspired commands for styling and layout:

### Styling
*   `\bf{...}`: Bold text.
*   `\it{...}`: Italic text.
*   `m{...}`: Roman (normal) text.
*   `	iny`, `\small`, `
ormalsize`, `\large`, `\Large`, `\LARGE`, `\huge`, `\Huge`: Font size modifiers.
*   `\fontface{FamilyName}{...}`: Change font family (e.g., `\fontface{Serif}{Text}`).

### Layout & Spacing
*   `\hspace{length}`: Horizontal space (e.g., `\hspace{10pt}`, `\hspace{2em}`).
*   `\vspace{length}`: Vertical space.
*   `\hfill`: Infinitely stretchable horizontal glue.
*   `ule{width}{height}`: Draw a solid rectangle (useful for rulers and lines).
*   `\hbox{...}`: Group content into a horizontal box.
*   `\parbox{width}{...}`: A box containing a paragraph of a specific width.
*   `aisebox{offset}{...}`: Raise or lower content.
*   `\vcenter{...}`: Vertically center content relative to the baseline.

### Miscellaneous
*   `\includesvg{path}{width}{height}`: Embed an SVG image.
*   `\hangindent{length}`, `\hangafter{lines}`: Configure hanging indentation.

---

## Design Principles

*   **TeX Inspiration**: We "steal shamelessly" from TeX's algorithms (Appendix G spacing, penalty values) while discarding legacy constraints.
*   **Modern Constraints**: Support for UTF-8, system fonts, and modern vector graphics is mandatory.
*   **Predictability**: Fixed-point arithmetic (`Scaled` type) ensures identical layout results across different platforms.

---

## Roadmap

*   **Phase 1 (Foundations)**: Basic types, memory management, and tokenizer (Completed).
*   **Phase 2 (Core)**: Knuth-Plass line breaking and basic commands (Completed).
*   **Phase 3 (Math)**: Math mode parser and layout engine (In Progress).
*   **Phase 4 (Backend)**: Advanced SVG support and PDF export (Planned).
*   **Phase 5 (Refinement)**: Tables (`\halign`) and sophisticated hyphenation (Planned).

---

## Build Instructions

Requires `libcairo2-dev`.

```bash
cd re-tex
make          # Builds all tests and demos
./demo_png    # Generates a sample output.png
```
