# XTCW Widget Demos

This directory contains working demo scripts showcasing various XTCW widgets and features.

## Running Demos

All demos are designed to be run from the `experimental/` directory using `commander_runner`:

```bash
cd /home/jens/git/xtcw2/experimental
./commander_runner -Tracelevel 2 -Luafile ../demos/demo_wlabel_svg.lua
```

## Available Demos

### demo_wlabel_svg.lua
**Wlabel with inline SVG images**

Demonstrates the `\includesvg{filename}{width}{height}` command that can be used within Wlabel text to embed SVG images inline. Features:
- Left, center, and right aligned labels with SVG icons
- Multiple SVG images in a single label
- Different sizes for SVG images (8pt to 24pt)

**Syntax:** `\includesvg{path/to/file.svg}{width}{height}`

Where:
- `path/to/file.svg` - Relative path to the SVG file
- `width` - Width in points (e.g., `12pt`, `1cm`)
- `height` - Height in points (e.g., `12pt`, `1cm`)

## Demo Ideas (TODO)

- [ ] Wlabel text selection and copy
- [ ] Gridbox layout with weights
- [ ] Wlist4 multi-column list
- [ ] Wcombo dropdown
- [ ] WspinBox numeric input
- [ ] WpixBtn pixel button
- [ ] Wsplitter paned layout
- [ ] IconSVG standalone images
- [ ] Wedit rich text editing
