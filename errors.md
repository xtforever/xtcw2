# Fixed Errors and Bugs

## Multi-column Rendering: s_split including Delimiter
- **Problem**: `s_split` in `utils/mls.c` incorrectly included the delimiter (e.g., `	`) in the resulting substrings when `remove_wspace` was 0. This caused layout issues in multi-column lists (like the filemanager).
- **Solution**: Adjusted the `strndup` length calculation in `utils/mls.c` to properly exclude the delimiter.
- **File**: `utils/mls.c`

## WlsMulti Property Propagation
- **Problem**: The `WlsMulti` wrapper widget was not propagating layout and font properties (`columnWidths`, `retexCells`, `fontSize`, etc.) to its internal `WlistMulti` child when they were updated via `set_values`.
- **Solution**: Enhanced the `set_values` method in `WlsMulti.widget` to explicitly call `XtVaSetValues` on the child widget for all relevant properties.
- **File**: `wbuild_widgets/WlsMulti.widget`

## Memory Corruption: m_free_user Pointer Handling
- **Problem**: `m_free_user` in `utils/m_tool.c` passed the address of the pointer within the list to the `free()` function instead of the pointer itself. This led to "List not allocated" errors and crashes when clearing string lists.
- **Solution**: Updated `m_free_user` to check if the element width equals the pointer size and dereference the pointer correctly before calling the free function.
- **File**: `utils/m_tool.c`

## Gridbox: Unsigned Underflow in layoutChild
- **Problem**: `layoutChild` in `Gridbox.c` performed calculations like `cell_size - margin - border` using unsigned `Dimension` variables. If the cell was too small, this resulted in an underflow to a very large value (e.g., 65534), causing widgets to be rendered off-screen or cover other widgets.
- **Solution**: Used intermediate `int` variables for geometry calculations to correctly handle negative results before enforcing a minimum size of 1 pixel.
- **File**: `plainc_widgets/Gridbox.c`

## Woptc: Missing query_geometry
- **Problem**: The `Woptc` widget (used as a slider) did not define a `query_geometry` method. When placed inside a `Gridbox` with `weightx=0`, the parent could not determine its preferred width, often resulting in a 0-width or incorrectly sized slider.
- **Solution**: Implemented `query_geometry` in `Woptc.widget` to return the `sliderWidth` as the preferred width.
- **File**: `wbuild_widgets/Woptc.widget`

## WlsMulti: Inconsistent Callback Resource Name
- **Problem**: `WlsMulti` used a resource named `callback` for row activation, while LUI and other list widgets expected `notify`. This caused `:on-row-activated` properties in LUI to have no effect.
- **Solution**: Renamed the `callback` resource to `notify` in `WlsMulti.widget` and updated the internal callback bridge.
- **File**: `wbuild_widgets/WlsMulti.widget`

## Wlabel: Geometry Calculation and Alignment Issues
- **Problem**: `Wlabel` widget had several geometry issues: 
  1. Height and width were truncated due to `int` casting of subpixel measurements, causing text clipping.
  2. "Infinite" width constant used for natural width calculation was too small, leading to incorrect preferred width (10000px).
  3. Documentation/comment for `alignment` was wrong (3 was described as right-justified, but 2 is RIGHT and 3 is JUSTIFY).
  4. `autoHeight` property was defined but not implemented.
- **Solution**: 
  1. Used `ceil()` for all pixel size conversions in `calculate_size`.
  2. Increased the "infinite" width constant to ensure it exceeds the `re-tex` threshold for natural layout.
  3. Corrected the `alignment` comment and ensured `set_values` triggers geometry requests when `autoHeight` is enabled.
- **File**: `wbuild_widgets/Wlabel.widget`
