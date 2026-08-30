# Gridbox Layout Widget

## Overview

Gridbox is a constraint-based composite widget that arranges children in a 2D grid. It supports fill modes, weights for space distribution, and gravity for positioning non-expanded children.

---

## Constraint Resources

Each child has constraint resources set via `XtVaSetValues`:

| Resource | Type | Default | Description |
|----------|------|---------|-------------|
| `gridx` | Position | 0 | Column position (0-based) |
| `gridy` | Position | 0 | Row position (0-based) |
| `gridWidth` | Dimension | 1 | Number of columns to span |
| `gridHeight` | Dimension | 1 | Number of rows to span |
| `weightx` | int | 0 | Horizontal stretch weight (for extra space distribution) |
| `weighty` | int | 0 | Vertical stretch weight |
| `fill` | FillType | FillBoth | How child expands to fill cell |
| `gravity` | XtGravity | CenterGravity | Position when cell is larger than child |
| `margin` | int | defaultDistance (4) | Cell padding in pixels |

---

## Fill Types

```c
typedef enum {
    FillNone = 0,   // Don't expand; use preferred size
    FillX = 1,      // Expand horizontally only
    FillY = 2,      // Expand vertically only
    FillBoth = 3   // Expand in both directions
} FillType;
```

**Fill vs. Weight - What's the difference?**

- **fill**: How much the child **expands** to fill available space in its cell
- **weight**: How **extra space** (beyond children's preferred sizes) is **distributed** among columns/rows

Example:
- Container is 100px wider than sum of column preferred widths
- Each column with `weightx > 0` gets: `extra_space * weight / total_weight`
- Each child with `FillX` fills its entire column width

---

## Gravity Types

When child is smaller than cell (fill = FillNone), gravity determines position:

| Gravity | Effect |
|---------|--------|
| `CenterGravity` | Center in cell |
| `NorthGravity` | Top of cell |
| `SouthGravity` | Bottom of cell |
| `WestGravity` | Left side |
| `EastGravity` | Right side |
| `NorthWestGravity` | Top-left corner |
| `NorthEastGravity` | Top-right corner |
| `SouthWestGravity` | Bottom-left corner |
| `SouthEastGravity` | Bottom-right corner |

---

## Geometry Manager

Gridbox implements `geometry_manager` to handle child resize requests:

```c
XtGeometryResult GridboxGeometryManager(Widget child, 
    XtWidgetGeometry *request, XtWidgetGeometry *reply);
```

**Flow:**
1. Child requests size via `XtMakeGeometryRequest()`
2. Gridbox computes new column/row sizes to accommodate
3. Gridbox requests size change from its parent
4. On success, Gridbox recomputes all cell sizes
5. Calls `layoutChild()` for each managed child
6. Returns: `XtGeometryDone` (done), `XtGeometryAlmost` (compromise), or `XtGeometryNo` (unchanged)

---

## Important Behaviors

### 1. Preferred Size Query
Gridbox queries each child's preferred size ONCE and caches it. If child changes preferred size (e.g., label text changes), child must request resize.

### 2. Resize Handling
When Gridbox resizes:
1. Recomputes column/row sizes from cached preferred sizes + weights
2. Distributes extra space based on weights
3. Calls each child's `resize()` with new cell size
4. Child's `set_values_almost` is called only if Gridbox offered compromise

### 3. Change Managed Children
When managed children change:
1. Gridbox queries all children's preferred sizes
2. Recomputes grid dimensions
3. Requests new size from parent
4. On success, calls `resize()` on self

---

## LUI/lua_xt Usage

In LUI, Gridbox constraints are set via keywords:

```lua
(window :id "w"
  (grid :id "g" :weightx 1 :weighty 1
    (Wlabel :id "title" :label "Title" 
            :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :fill 3)
    (Wlabel :id "left" :label "Left" 
            :gridx 0 :gridy 1 :weightx 1 :fill 1 :gravity "west")
    (Wlabel :id "right" :label "Right" 
            :gridx 1 :gridy 1 :weightx 1 :fill 1 :gravity "east")))
```

Keywords map to constraint resources:
- `:gridx` → `gridx`
- `:gridy` → `gridy`
- `:gridWidth` → `gridWidth`
- `:gridHeight` → `gridHeight`
- `:weightx` → `weightx`
- `:weighty` → `weighty`
- `:fill` → `fill` (numeric: 0-3)
- `:gravity` → `gravity` (string: "center", "north", "south", etc.)

---

## Gridbox.resize() Implementation

```c
static void GridboxResize(Widget w)
{
    GridboxWidget gb = (GridboxWidget)w;
    
    // Compute row/column sizes if not already done
    if (gb->gridbox.max_wids == NULL)
        computeWidHgtInfo(gb);
    
    // Layout children in cells
    layout(gb, gb->core.width, gb->core.height);
    
    // For each managed child: set position and size
    for each child in children:
        if XtIsManaged(child):
            layoutChild(gb, child, &width, &height, &x, &y);
            XtConfigureWidget(child, x, y, width, height, 0);
}
```

---

## Common Issues

### 1. Child not appearing
**Cause:** Child not managed (`XtManageChild()` not called)
**Fix:** Add child to grid, then manage it

### 2. Child smaller than cell, positioned wrong
**Cause:** Wrong fill or gravity settings
**Fix:** Set `fill = FillNone` and appropriate `gravity`

### 3. Extra space not distributed
**Cause:** All children have `weightx = 0` (default)
**Fix:** Set `weightx` on children that should grow

### 4. Gridbox not resizing to fit children
**Cause:** Not querying children's preferred sizes
**Fix:** Ensure managed children have working `query_geometry()`

### 5. Wlabel sizing issues in Gridbox
**Cause:** Wlabel's `query_geometry` may return wrong size, or `set_values_almost` not implemented
**Fix:** Ensure Wlabel properly reports preferred size and accepts geometry compromises

---

## References

- Source: `plainc_widgets/Gridbox.c` (1225 lines)
- Widget definition: `wbuild_widgets/Gridbox.widget`
- Constraint header: `plainc_widgets/GridboxP.h`