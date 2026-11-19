-- Wmarkdown demo: a read-only Markdown help-text viewer rendered with retex.
--
-- The viewer understands the common help-text subset: headings, justified
-- paragraphs, bullet/numbered lists, block quotes, fenced code blocks,
-- horizontal rules, **bold**, *italic*, `inline code` and [links](url).
-- Select text with the mouse, Ctrl+A selects all, Ctrl+C copies.  Scroll with
-- the mouse wheel, Up/Down, PageUp/PageDown, Home/End.

local gui = require('gui_xt')
local lui = require('lui')

local help = [[
# Wmarkdown

A small read-only viewer for help texts. It renders Markdown with the same
re-tex layout engine as Wbravo, so paragraphs are **fully justified** while
headings and list items stay left-aligned.

## Supported syntax

- `# Heading` through `###### Heading`
- Paragraphs separated by blank lines
- Bullet lists with `-`, `*` or `+`
- Numbered lists like `1.`
- `> Block quotes`
- Fenced code blocks with ``` or ~~~
- Inline `code`, **bold** and *italic*
- Links: [the label is shown](https://example.com)

### A code block

```
int main(void)
{
    return 0;
}
```

### A quote

> Good help is short, searchable and readable.
> This line continues the same quote.

---

Select text with the mouse and press Ctrl+C to copy it. Scroll with the mouse
wheel or the arrow / PageUp / PageDown keys.
]]

local ui = [[
(window :id "win" :title "Wmarkdown - help viewer" :width 720 :height 620
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr"
           :label "Wmarkdown - Markdown help text (scroll with wheel / keys)"
           :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)
    (Wmarkdown :id "help"
               :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3
               :fontSize 15
               :useThemeFont 0
               :fontFace "TeX Gyre Pagella"
               :alignment 3
               :text "Loading help...")))
]]

local ok, err = lui.run(ui)
if not ok then error("demo_markdown: " .. tostring(err)) end

-- Pass the Markdown source directly; LUI's parser is not involved here, so
-- newlines and backticks stay exactly as written.
gui.set("help", "text", help)
