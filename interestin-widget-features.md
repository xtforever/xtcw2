# Interesting Widget Features: Alert.w

The `XfwfAlert` widget (`Alert.w`) provides several advanced implementation patterns for `wbuild` and Xt-based widgets.

## Distinct Features and Patterns

### 1. Shell Widget Specialization
Inheriting from `TransientShell` instead of `Core` or `Composite` allows the widget to behave as a top-level popup dialog managed by the window manager.
*   **Implementation:** `@class XfwfAlert (TransientShell) @file=Alert`

### 2. Synchronous Dialog Pattern (`XfwfDialog`)
Implementation of a "blocking" utility function that encapsulates widget creation, realization, and a local event loop. This allows the application to call a single function and wait for a user response.
*   **Implementation:** `XfwfDialog` procedure using `XtAppProcessEvent` in a `while` loop.

### 3. Dynamic Child Management
The widget acts as a container for other widgets (`Icon`, `Label`, `Button`) by managing them as private state (`@var Widget *buttonw`) rather than exposing them as public children.
*   **Implementation:** `create_children` procedure handles destruction and recreation of internal widgets.

### 4. String Parsing for Resource Initialization
Using `strtok` to parse a single string resource (e.g., `"OK|Cancel|Retry"`) to dynamically instantiate and label a variable number of button widgets.
*   **Implementation:** Parsing the `buttons` resource string in `create_children`.

### 5. Custom Type Converter Integration
Registering a global type converter in `class_initialize` to allow the resource system to handle complex types (like `Icon`) from string definitions in resource files.
*   **Implementation:** `XtSetTypeConverter(XtRString, "Icon", cvtStringToIcon, ...)` within `@proc class_initialize`.

### 6. Manual Geometry Layout
Calculating child positions (`XtNx`, `XtNy`) and container size (`XtResizeWidget`) programmatically based on the dimensions of children.
*   **Implementation:** Coordinate calculations in `create_children` and final `XtResizeWidget` call on the `outer` board.

### 7. Callback Propagation
Mapping internal child events (a button click) to the widget's own public callback list.
*   **Implementation:** `XtCallCallbackList($, $callback, (XtPointer) $lastChoice)` inside the `click` procedure.

### 8. Resource Lifecycle Management
Strict handling of allocation and deallocation of string resources across widget states.
*   **Implementation:** Using `XtNewString` in `initialize` and `set_values`, and `XtFree` in `destroy`.

## New Features from TextMenu.w

### 9. OverrideShell for Menus
Inheriting from `OverrideShell` to create a menu that bypasses window manager decorations and stays on top.
*   **Filename:** `TextMenu.w`

### 10. Dynamic Accelerator Generation
Parsing a custom menu description string to programmatically generate and set `XtNaccelerators` on the widget.
*   **Filename:** `TextMenu.w`

### 11. Multi-column Label Support (Tablist)
Integrating with a `TabString` library to support labels with specific pixel-based tab stops.
*   **Filename:** `TextMenu.w`

### 12. Manual Shortcut Underlining
Programmatically calculating the position of a specific character (prefixed by `_`) and drawing a line underneath it to indicate a keyboard shortcut.
*   **Filename:** `TextMenu.w`

### 13. Bitmask-based State Management
Using a single `long` resource as a bitmask to manage the active/inactive state of multiple children (menu items).
*   **Filename:** `TextMenu.w`

### 14. Keyboard/Pointer Grab Management
Using `XtGrabKeyboard` and `XChangeActivePointerGrab` to control input focus and cursor appearance when a popup menu is active.
*   **Filename:** `TextMenu.w`

## New Features from VScrollb.w

### 15. Default Translation Specialization
Creating a subclass specifically to provide default translations for a superclass without changing its logic.
*   **Filename:** `VScrollb.w`

## New Features from MenuBar.w

### 16. Custom Method for Child Communication (`process_menu`)
Defining a custom method in the parent widget that children can call (via a wrapper function like `XfwfCallProcessMenu`) to coordinate complex interactions like menu switching.
*   **Filename:** `MenuBar.w`

### 17. Passive and Local Grab Management
Using `XtAddGrab` and `XtPopup` with `XtGrabNonexclusive` to implement "menu dragging" behavior, where moving the pointer between parent buttons automatically switches active popups.
*   **Filename:** `MenuBar.w`

### 18. Cross-Widget State Tracking
Maintaining a reference to a child's popup (`current_menu`) in the parent to manage its lifecycle (popping it down when another child is activated).
*   **Filename:** `MenuBar.w`

## New Features from Group.w

### 19. Superimposed Labels on Borders
Programmatically drawing a label over the widget's border (interrupting it) by using `compute_inside` and `XDrawImageString` in the `expose` method.
*   **Filename:** `Group.w`

### 20. Automatic Logic Injection (`insert_child`)
Overriding `insert_child` to detect specific child classes (`XfwfToggle`) and automatically attaching internal callbacks (`on_cb`, `off_cb`) to implement radio-button or multi-select logic without user intervention.
*   **Filename:** `Group.w`

### 21. Declarative Selection Styles (Enums)
Implementing complex selection logic (None, Single, One, Multiple) through a custom Enum and associated type converters, providing a high-level API for child behavior.
*   **Filename:** `Group.w`

### 22. Bitmask and Index-based Child Addressing
Supporting both a single index (for radio buttons) and a bitmask (for multi-select) within the same `long` resource to track the state of up to 32 children.
*   **Filename:** `Group.w`

## New Features from RadioGrp.w

### 23. StringArray-based Child Instantiation
Automatically creating and managing child widgets based on a `StringArray` resource, simplifying bulk creation of common elements (like a list of radio buttons).
*   **Filename:** `RadioGrp.w`

### 24. Convenience Wrapper Class
A subclass (`XfwfRadioGroup`) that adds no new core logic but simplifies the configuration of its superclass (`XfwfGroup`) by providing a more specialized, data-driven interface.
*   **Filename:** `RadioGrp.w`

## New Features from Slider2.w

### 25. Exporting Methods via Resources (`scrollResponse`)
Allowing external widgets or applications to trigger internal logic by exporting a method as a read-only resource (`XtNscrollResponse`).
*   **Filename:** `Slider2.w`

### 26. 2D Scroll Logic (XY Thumb)
Implementing a thumb that moves in two dimensions (X and Y) using normalized float coordinates (0.0 to 1.0) and supporting both proportional sizing and minimum pixel sizes.
*   **Filename:** `Slider2.w`

### 27. Optimized Movement with `XCopyArea`
Using `XCopyArea` to move a rendered portion of the widget (the thumb) without a full redraw, and manually calculating the minimal "exposed" regions to clear.
*   **Filename:** `Slider2.w`

### 28. Geometry-Aware Action Mapping
Using a single "start" action that branches behavior (Page Up/Down vs. Drag) based on whether the click coordinate falls inside or outside a programmatically calculated "thumb" area.
*   **Filename:** `Slider2.w`

### 29. Graphic Expose Compression
Setting `compress_exposure = XtExposeCompressMultiple | XtExposeGraphicsExpose` in `@classvars` to handle complex exposure events generated by `XCopyArea`.
*   **Filename:** `Slider2.w`

## New Features from Arrow.w

### 30. Repeating Callbacks via `XtAppAddTimeOut`
Implementing a "press and hold" behavior where a callback is triggered repeatedly at a set interval using a recursive timer (`XtIntervalId`) that is started in a `ButtonPress` action and stopped in `ButtonRelease`.
*   **Filename:** `Arrow.w`

### 31. Manual 3D Triangle Geometry
Drawing a 3D arrow by manually calculating and filling multiple polygons (`XFillPolygon`) for the face and each shadow edge, adjusting the geometry based on the `direction` (Top, Left, Right, Bottom).
*   **Filename:** `Arrow.w`

### 32. Visual "Press" Feedback without Redraw
Simulating a button press by swapping the Light and Dark shadow GCs and refilling the polygons in the `push_down` and `push_up` actions to provide immediate visual feedback.
*   **Filename:** `Arrow.w`

## New Features from FText.w

### 33. TeX-like Box Layout Engine
Implementation of a full-featured box-based layout engine within a widget, supporting concepts like chunks (words, spaces, inlines), paragraphs, floating figures, and vertical/horizontal skips.
*   **Filename:** `FText.w`

### 34. Dynamic Font Management (5x4x7 Matrix)
Managing a large matrix of 140 potential fonts (5 families, 4 styles, 7 sizes) and loading them on-demand to support rich text formatting.
*   **Filename:** `FText.w`

### 35. Floating Content Support (Left/Right)
Algorithmically placing paragraphs or images in left/right margins and causing subsequent text to flow around them by dynamically recomputing target widths for each line.
*   **Filename:** `FText.w`

### 36. Inline Widget Integration in Text Flow
Treating child widgets as "chunks" within the text stream, managing their mapping/unmapping based on visibility, and proxying input events (via `XfwfPassClick`) back to the parent.
*   **Filename:** `FText.w`

### 37. Background Work Procedures (`XtWorkProc`)
Using `XtAppAddWorkProc` to perform expensive layout calculations (`do_layout`) only when the X event queue is empty, preventing UI freezes during complex document reformatting.
*   **Filename:** `FText.w`

### 38. Hyperlink and Active Area Tracking
Implementing "active areas" in text (hyperlinks) by associating `XtPointer data` with specific chunks and using `motion` and `activate` actions to trigger callbacks and cursor changes.
*   **Filename:** `FText.w`

## New Features from SpinLabel.w

### 39. Composite Widget with Internal Sub-widgets
Creating a higher-level component by instantiating and managing private child widgets (`XfwfArrow`, `XfwfLabel`) and coordinating their callbacks and geometry.
*   **Filename:** `SpinLabel.w`

### 40. Orientation-Switched Geometry Logic
Implementing complex geometry calculations that can switch between horizontal and vertical layouts (arrows beside label vs. arrows above/below label) based on a single `horizontal` resource.
*   **Filename:** `SpinLabel.w`

### 41. Keyboard-Driven Component Navigation
Providing a rich keyboard interface (`<Key>Home`, `<Key>Left`, `<Key>plus`, etc.) that maps standard keys to specific component actions (`first`, `prev`, `next`, `last`) for a custom control.
*   **Filename:** `SpinLabel.w`

### 42. Propagation of Child Callbacks
Attaching internal callback routines to private children that then re-trigger the parent widget's own public callback list with specialized `call_data` (Enums like `XfwfNext`, `XfwfPrev`).
*   **Filename:** `SpinLabel.w`

## New Features from RowCol.w

### 43. Dynamic Grid-based Layout
Implementing a grid layout where cell sizes are determined by the maximum width/height of all managed children, with support for filling by row or by column.
*   **Filename:** `RowCol.w`

### 44. Child Alignment within Grid Cells
Using a bitmask-based `Alignment` resource to position children within their allocated grid cells (TopLeft, Center, BottomRight, etc.) during the layout phase.
*   **Filename:** `RowCol.w`

### 45. Self-Resizing Composite (`shrinkToFit`)
A composite widget that can programmatically request a new size from its own parent (`XtVaSetValues`) based on the calculated bounding box of its laid-out children.
*   **Filename:** `RowCol.w`

### 46. Sophisticated Geometry Negotiation
Overriding `geometry_manager` to always grant child size requests but immediately re-triggering a full parent layout to ensure all siblings are correctly repositioned on the grid.
*   **Filename:** `RowCol.w`

## New Features from XmIcon.w

### 47. Non-Rectangular Windows (X Shape Extension)
Using `XShapeCombineMask` in the `realize` and `set_values` methods to apply a transparency mask to the widget's top-level window, allowing for non-rectangular icons.
*   **Filename:** `XmIcon.w`

### 48. Background Pixmap Optimization
Directly setting the `XtNbackgroundPixmap` resource of the widget to the loaded `image->pixmap` to allow the X server to handle the icon rendering automatically without an explicit `expose` method.
*   **Filename:** `XmIcon.w`

### 49. XPM Image Metadata Integration
Accessing `XpmAttributes` (width, height) from a custom `Icon` structure to dynamically adjust the widget's own size and layout properties upon image loading.
*   **Filename:** `XmIcon.w`

## New Features from Button.w

### 50. Extreme Minimization via Inheritance
Demonstrating "pure" OOP by implementing a functional button widget with only six lines of unique code, inheriting almost all logic (multi-line text, 3D frames, and location) from `XfwfLabel` and its ancestors.
*   **Filename:** `Button.w`

### 51. Visual Feedback via Translation Swapping
Using translations (`set_shadow("sunken")`) to manipulate inherited visual properties on-the-fly, providing interactive feedback (the button "depresses" when clicked) without custom C drawing code.
*   **Filename:** `Button.w`

### 52. Keyboard Traversal Activation
Explicitly enabling `traversalOn = True` to integrate the custom widget into the X Toolkit keyboard focus chain, allowing activation via the `Return` key.
*   **Filename:** `Button.w`

## New Features from PieMenu.w

### 53. Circular Geometry and X Shape Ellipse
Using `XmuReshapeWidget($, XmuShapeEllipse, ...)` to create a truly circular window and using trigonometric functions (`sin`, `cos`, `atan2`) to handle input and drawing in a polar coordinate system.
*   **Filename:** `PieMenu.w`

### 54. Domain-Specific Language (DSL) for Actions
Implementing a custom string-based DSL (e.g., `"Label -> action(param1, param2)"`) and a dedicated `parse_menu` utility to decompose complex interaction rules into executable `XtCallActionProc` calls.
*   **Filename:** `PieMenu.w`

### 55. Polar Coordinate Hit Testing
Determining user selection by calculating the angle and distance of a mouse release relative to the widget's center (`atan2(dy, dx)`), rather than using standard rectangular bounding boxes.
*   **Filename:** `PieMenu.w`

### 56. Multi-GC Segment Rendering
Managing a dedicated array of GCs to allow each segment of the "pie" to have its own unique background, foreground, and font, all while sharing a common central "hole" and radial divider lines.
*   **Filename:** `PieMenu.w`

### 57. Resource Default Procedures (`CallProc`)
Extensive use of `CallProc` to implement "cascading" defaults, where specific resources (like `font1` or `background1`) automatically inherit from a central resource (`font`, `background`) if not explicitly set.
*   **Filename:** `PieMenu.w`

## New Features from Common.w

### 58. Hierarchical Keyboard Traversal Engine
Implementation of a complete keyboard navigation system (Up, Down, Left, Right, Home, Next, Prev) that uses recursive `traverse` and `accept_focus` methods to coordinate focus movement between siblings and across parent-child boundaries.
*   **Filename:** `Common.w`

### 59. X Color Context (XCC) Integration
Using a dedicated library (XCC) to manage color allocation, providing automatic fallback to nearest available colors, support for standard colormaps, and private colormap management.
*   **Filename:** `Common.w`

### 60. Virtual Focus and Border Highlighting
Distinguishing between "real" focus and "virtual" focus (where a descendant has the focus) using `FocusIn`/`FocusOut` details, and managing a separate `highlightThickness` border.
*   **Filename:** `Common.w`

### 61. Protocol-based Geometry Methods
Defining foundational methods like `compute_inside` and `total_frame_width` that subclasses are expected to override, establishing a standard protocol for geometry negotiation within the toolkit.
*   **Filename:** `Common.w`

### 62. Global Resource Converters (Alignment, Color, StringArray)
Registering essential type converters in the base class (`class_initialize`) so that all derived widgets automatically support complex string-to-type transformations for common properties.
*   **Filename:** `Common.w`

### 63. Dynamic Translation Augmentation
Programmatically merging a global set of `traversal_trans` into a widget's translation table only when a specific resource (`traversalOn`) is enabled.
*   **Filename:** `Common.w`

## New Features from Entry.w

### 64. Single-Line Text Editor Logic
Implementation of a full text-editing buffer within a widget, including cursor management (`text_pos`), insertion, backspace, deletion, and "kill to end of line" functionality.
*   **Filename:** `Entry.w`

### 65. Input Validation via Resource String
Restricting user input by checking each keypress against a `valid` resource string (e.g., only allowing numeric characters) within the `insert_char` utility.
*   **Filename:** `Entry.w`

### 66. Sensitive Data Input (Echo Mode)
Supporting password-style input by toggling a boolean `echo` resource that causes the widget to suppress the display of actual characters while still maintaining the internal buffer.
*   **Filename:** `Entry.w`

### 67. Manual Xlib Cursor Rendering
Drawing and erasing a text cursor manually by swapping between "normal" and "reverse" GCs and using `XDrawImageString` to redraw characters with inverted colors.
*   **Filename:** `Entry.w`

### 68. Mouse-to-Cursor Coordinate Mapping
Implementing "click to position" logic by iterating through text segments and using `XTextWidth` to find the character index closest to the mouse's X-coordinate.
*   **Filename:** `Entry.w`

### 69. Integration with `XComposeStatus`
Maintaining keyboard composition state (`XComposeStatus`) across multiple `XLookupString` calls to support dead keys and complex character input.
*   **Filename:** `Entry.w`

## New Features from IconBox.w

### 70. Rubberband (Area) Selection Logic
Implementation of a "rubberband" selection box that follows the mouse pointer, using `XDrawRectangle` with a `GXxor` GC to draw/erase outlines efficiently, and `XRectInRegion` to determine which icons are selected.
*   **Filename:** `IconBox.w`

### 71. Drag-and-Drop via ClientMessages
Implementing a custom drag-and-drop protocol using `XSendEvent` and `ClientMessage` to notify a target icon when other icons have been dropped onto it, and using a root window property (`DropSelection`) for data transfer.
*   **Filename:** `IconBox.w`

### 72. Global Shared Class-level Cache
Using a hash table stored in a class variable (`hashtable`) to share pre-loaded icon pixmaps across all instances of the `IconBox` widget, minimizing redundant file I/O and memory usage.
*   **Filename:** `IconBox.w`

### 73. Composite Icon-Label Generation
Dynamically instantiating and managing pairs of `XfwfIcon` and `XfwfButton` widgets for each item in a data list, and programmatically injecting per-item translations (e.g., `handle_drop(0)`, `handle_drop(1)`).
*   **Filename:** `IconBox.w`

### 74. Grid-based Canonical Layout
An algorithm for aligning a variable number of icons on a fixed grid (`horizontalGrid`, `verticalGrid`) while supporting manual "cleaning up" to the nearest grid point.
*   **Filename:** `IconBox.w`

### 75. XOR-based Multi-object Dragging
Drawing outlines for multiple selected objects during a drag operation using a single `GXxor` GC to provide real-time visual feedback without full widget redraws.
*   **Filename:** `IconBox.w`

## New Features from Enforcer.w

### 76. Layout-Policy Enforcement Pattern
Implementing a strict parent-child relationship where the parent forces its single child to always match its own "inside" dimensions, regardless of the child's preferred size.
*   **Filename:** `Enforcer.w`

### 77. "Deny-All" Geometry Manager
A geometry management strategy that always returns `XtGeometryNo`, effectively freezing the size of its child and preventing it from influencing the layout of the container.
*   **Filename:** `Enforcer.w`

### 78. Retrofitting Older Widgets
Using a "wrapper" widget to apply modern toolkit features (like Xfwf location resources or 3D frames) to legacy widgets that were not originally designed to support them.
*   **Filename:** `Enforcer.w`

## New Features from FoldTree.w

### 79. Expandable/Collapsible Tree Layout
Implementing a recursive tree structure where children can be hidden or shown by toggling an `expanded` state, with the parent widget automatically recalculating its own geometry and the positions of its siblings.
*   **Filename:** `FoldTree.w`

### 80. Visibility Control via `XtSetMappedWhenManaged`
Using `XtSetMappedWhenManaged` to hide child widgets without unmanaging them, allowing the widget to maintain its logical layout state while controlling what is physically visible on the screen.
*   **Filename:** `FoldTree.w`

### 81. Adaptive Connection Line Drawing
Manually drawing "tree lines" (`XDrawLine`) that adapt to the geometry of children, with specialized logic to point at the center of a child's control icon if that child is itself a `FoldingTree` instance.
*   **Filename:** `FoldTree.w`

### 82. First-Child Specialization (Label Widget)
Treating the first child added to the composite specifically as a "label" that stays visible next to the control icon, while all subsequent children form the collapsible subtree.
*   **Filename:** `FoldTree.w`

### 83. Recursive Layout and Shrinkage
A `layout` method that optionally shrinks the parent widget (`shrinkToFit`) to perfectly wrap only the currently visible (expanded) nodes, allowing for dynamic UI resizing.
*   **Filename:** `FoldTree.w`

## New Features from Label.w

### 84. Multi-Line Alignment Logic
Implementing vertical and horizontal alignment for multi-line text by manually calculating the bounding box of each line and adjusting the starting `y` and per-line `x` coordinates based on an `Alignment` bitmask.
*   **Filename:** `Label.w`

### 85. Partial Text Highlighting (Substrings)
Supporting the highlighting of specific character ranges within a label using `rvStart`/`rvLength` (reverse video) and `hlStart`/`hlLength` (foreground change), achieved by splitting lines into segments during the `expose` phase.
*   **Filename:** `Label.w`

### 86. Performance-Optimized Property Updates (`set_label`)
Providing a custom `set_label` method that can bypass the full `XtSetValues` cycle when `shrinkToFit` is false, allowing for faster text updates by directly triggering an internal redraw.
*   **Filename:** `Label.w`

### 87. Integration of Pixel-based Tab Stops
Using an external `TabString` library to calculate text widths and draw strings that respect pixel-defined tab stops (`tablist`), enabling multi-columnar text within a single label.
*   **Filename:** `Label.w`

### 88. Insensitive "Graying Out" via Stippling
Simulating an inactive state by applying a transparent stipple GC over the entire text area when `sensitive` is False, providing a standard visual cue for disabled widgets.
*   **Filename:** `Label.w`

## New Features from Pager.w

### 89. External Data Sourcing (File/Pipe)
A resource-driven data loading mechanism that interprets strings starting with `@` as filenames and strings starting with `` ` `` as shell commands (using `popen`), automatically populating the internal text buffer.
*   **Filename:** `Pager.w`

### 90. Dynamic Font Scaling (XLFD Manipulation)
Algorithmically resizing text by parsing XLFD font names, identifying scalable fonts (point size 0), and generating new XLFD strings with calculated pixel sizes to fit a requested number of lines.
*   **Filename:** `Pager.w`

### 91. Text-to-Page Splitting Logic
Implementing a pagination engine that scans a large text buffer and pre-calculates an array of page-start indices based on either a fixed line count or the physical height of the widget.
*   **Filename:** `Pager.w`

### 92. Sub-widget Control Surface ("Dog's Ear")
Creating a specialized UI control (page flipper) by managing two private `XfwfIcon` widgets and coordinating their callbacks to navigate the parent's internal page list.
*   **Filename:** `Pager.w`

### 93. Automatic Font Inference
A `CallProc` default procedure (`guess_roman`) that attempts to automatically find and load a suitable "normal" font from a generic `fontFamily` name using `XListFonts`.
*   **Filename:** `Pager.w`

## New Features from Prompt.w

### 94. Template-based Input (Masked Entry)
Implementation of a masked text entry field using a `template` string (e.g., `"Date: __/__/__"`) and a corresponding `pattern` string (`"99/99/99"`) to define allowed character classes per position.
*   **Filename:** `Prompt.w`

### 95. Pattern Matching Logic
A comprehensive character-class validation system (`matches` utility) supporting digits, alphabetic, alphanumeric, file-safe characters, and custom placeholders.
*   **Filename:** `Prompt.w`

### 96. X11 Selection Owner (Clipboard Integration)
Implementing the `convert_proc` and `lose_ownership_proc` protocols to allow the widget to participate in the X11 PRIMARY selection, enabling Cut, Copy, and Paste between different applications.
*   **Filename:** `Prompt.w`

### 97. Interactive Input Validation Callback
Providing a `validate` callback that allows the application to intercept every character change and programmatically `Reject`, `Accept`, or mark the input as `Complete`.
*   **Filename:** `Prompt.w`

### 98. Template Run Management
Algorithmically managing "runs" of identical pattern characters within a template, enabling smart shifting and deletion of text without breaking literal characters in the mask.
*   **Filename:** `Prompt.w`

### 99. Synthetic Cursor Position Rendering
Calculating and drawing a cursor line (`XDrawLine`) manually by translating a character index into pixel coordinates using `XfwfTextWidth` and the current text alignment.
*   **Filename:** `Prompt.w`

## New Features from OptButton.w

### 100. State-reflecting Label Specialization
Implementing a specialization of a `PullDown` button where the widget's label is dynamically updated to reflect the `call_data` (specifically the label) of the last activated item in its associated popup menu.
*   **Filename:** `OptButton.w`

### 101. Cross-Widget Feedback Loop
Automatically attaching a callback (`set_label_cb`) to a child or associated popup widget during initialization to establish a feedback loop that synchronizes the parent widget's appearance with child interactions.
*   **Filename:** `OptButton.w`

### 102. Translation Redefinition Pattern
A workaround for tool limitations (like `wbuild`'s lack of translation inheritance) where a full set of parent translations is explicitly restated to ensure correct interaction flow in a subclass.
*   **Filename:** `OptButton.w`

## New Features from XmAnsiTerm.w

### 103. ANSI Terminal Finite State Machine (FSM)
Implementation of a complex state machine within the `write` method to parse ANSI escape sequences, supporting multi-register numeric parameters and transition-based logic for cursor movement and formatting.
*   **Filename:** `XmAnsiTerm.w`

### 104. Attribute-based Grid Storage
Managing a 2D grid of character data (`contents`) and a parallel grid of metadata (`attribs`) to track formatting states (Bold, Reverse, Underline, Invisible) per character cell.
*   **Filename:** `XmAnsiTerm.w`

### 105. "Dirty" Line Optimized Redrawing
Implementing a performance optimization where the `write` method only triggers redrawing for lines marked with an `ATTRIB_DIRTY` flag, significantly reducing X server traffic for bulk terminal output.
*   **Filename:** `XmAnsiTerm.w`

### 106. Recursive Font Weight Inference
Algorithmically generating a `boldfont` name from a base `font` string by parsing the XLFD, substituting the weight field, and loading the resulting font via `XLoadQueryFont`.
*   **Filename:** `XmAnsiTerm.w`

### 107. Blinking Cursor via Timer Recursion
Implementing a blinking text cursor using `XtAppAddTimeOut` to toggle the visibility of a character cell (swapping REV attribute) every 500ms, with specialized handling for FocusIn/Out and Map/Unmap events.
*   **Filename:** `XmAnsiTerm.w`

### 108. Input Protocol Stealing (Event Handlers)
Using `XtInsertEventHandler` with `XtListHead` to intercept all key presses before they reach the standard translation manager, ensuring the terminal emulator has exclusive first-access to keyboard input.
*   **Filename:** `XmAnsiTerm.w`

### 109. Bi-directional Terminal Communication
Supporting control sequences like `report_cursor_pos` that "send" data back to the application by triggering the widget's own `keyCallback` as if the user had typed the response.
*   **Filename:** `XmAnsiTerm.w`

## New Features from Calendar.w

### 110. Array-based Visual State Mapping
Using a 32-element integer array (`dayColors`) to map data indices (days of the month) to specific visual attributes (colors 1-5 and reverse video) without requiring separate child widgets.
*   **Filename:** `Calendar.w`

### 111. Julian Day Date Calculations
Implementation of astronomical algorithms (`day_number` via Steve Moshier) to calculate the correct day of the week for any given date from 4713 B.C. to 54,078 A.D.
*   **Filename:** `Calendar.w`

### 112. Fractional Grid Geometry Calculation
Manually calculating a 7x6 grid using floating-point math to ensure perfectly distributed spacing (`yspace`, `yinc`) even when the widget is resized to non-integer multiples of the character size.
*   **Filename:** `Calendar.w`

### 113. Interactive Header Buttons (Composite-lite)
Drawing and handling interactive "buttons" within the main widget window (for month/year navigation) using `XCopyPlane` for bitmap icons and manual hit-testing in the `buttonpress` action.
*   **Filename:** `Calendar.w`

### 114. XOR-based Selection Highlighting
Supporting specialized date highlighting by using a dedicated `reversingGC` with `GXxor` to invert specific date boxes without destructive redrawing.
*   **Filename:** `Calendar.w`

### 115. Optimized Partial Redraw (`XfwfSetDayColors`)
Providing an exported utility function that allows the application to update visual states for multiple days while only triggering `XClearArea` and redrawing for the specific cells that actually changed.
*   **Filename:** `Calendar.w`

## New Features from ThWheel.w

### 116. Multi-image Frame Animation
Simulating mechanical rotation by cycling through an array of slightly different `XImage` or `Pixmap` resources (`curpic`) in response to user input, creating a "turning" visual effect.
*   **Filename:** `ThWheel.w`

### 117. Background Pixmap Animation
Updating the widget's appearance by dynamically swapping the `XtNbackgroundPixmap` resource to point to different pre-loaded frames, avoiding explicit `expose` calls for simple animations.
*   **Filename:** `ThWheel.w`

### 118. Dual-Mode Input (Discrete vs. Continuous)
Implementing a single `turn` action that branches behavior based on click geometry: the "edges" trigger timed discrete steps (`XfwfSUp`/`XfwfSDown`), while the "center" enters a continuous drag mode using a pointer-query loop.
*   **Filename:** `ThWheel.w`

### 119. Multi-Button Speed Scaling
Assigning different adjustment speeds (100%, 50%, 25%) to different mouse buttons within the same action, providing fine-grained control over a single mechanical metaphor.
*   **Filename:** `ThWheel.w`

### 120. Custom Resource-driven Valuator
Implementing a "valuator" (scrolling) widget from scratch using normalized float values (0.0 to 1.0) for internal communication while exposing a standard `minValue`/`maxValue` integer API.
*   **Filename:** `ThWheel.w`

### 121. Pointer-Query Drag Loop
Using `XQueryPointer` in a `while` loop within an action to implement a custom, high-frequency drag behavior that bypasses the standard X event queue for smoother visual feedback.
*   **Filename:** `ThWheel.w`

## New Features from Icon.w

### 122. Non-Rectangular Clipping (X Shape Extension)
Applying an icon's transparency mask to the widget's physical window using `XShapeCombineMask`, allowing icons to have complex, non-rectangular boundaries that are respected by the window manager and pointer events.
*   **Filename:** `Icon.w`

### 123. Tiled GC Rendering Optimization
Rendering an icon by creating a GC with `FillTiled` and setting the `tile` to the icon's pixmap, then using `XFillRectangle` to draw the entire image in a single call.
*   **Filename:** `Icon.w`

### 124. Dynamic Sizing based on XPM Metadata
Automatically adjusting the widget's `width` and `height` during initialization and `set_values` by extracting dimensions from the `XpmAttributes` of the loaded icon.
*   **Filename:** `Icon.w`

### 125. Mask-dependent Decoration Suppression
Logic that implicitly hides the widget's 3D frame when a transparency mask is present, ensuring that modern icons don't have clashing rectangular borders.
*   **Filename:** `Icon.w`

## New Features from Animator.w

### 126. Frame-based Animation Engine
Implementing a timed animation sequencer that cycles through an array of `XImage` structures, using `XtAppAddTimeOut` to trigger frame updates and `XPutImage` for rendering.
*   **Filename:** `Animator.w`

### 127. Per-frame Variable Timing
Supporting a `CardinalList` of intervals that allows each frame in an animation to have its own unique duration, with fallback logic to repeat the last interval or use a default value.
*   **Filename:** `Animator.w`

### 128. Manual Image-to-Widget Geometry
Algorithmically determining the widget's preferred size by scanning an entire list of potential animation frames and adopting the dimensions of the largest image in the set.
*   **Filename:** `Animator.w`

### 129. String-to-ImageList Resource Converter
Implementing a custom type converter that parses a space/comma-separated string of filenames, loads each file via `XpmReadFileToImage`, and assembles a NULL-terminated array of `XImage` pointers.
*   **Filename:** `Animator.w`

### 130. Programmatic Animation Control
Providing exported methods (`XfwfStartAnimation`, `XfwfStopAnimation`) that allow external logic to restart or halt the widget's internal timer-driven update loop.
*   **Filename:** `Animator.w`

## New Features from HTML2.w

### 131. Semantic Subclassing (SGML to HTML)
Specializing a generic `SimpleSGML` widget by overriding `add_starttag` and `pop_style` to implement HTML-specific semantics, such as automatically inserting missing container tags (`BODY`, `TR`, `TABLE`) based on heuristic context rules.
*   **Filename:** `HTML2.w`

### 132. Form Data Management System
Implementing a structured "form" system that tracks nested `FormField` objects (Text, Checkbox, Radio, Select), managing their values, selection states, and associations with their parent `Form` structure.
*   **Filename:** `HTML2.w`

### 133. URL-Encoded Submission Logic (`submit_cb`)
An automated mechanism for collecting data from a distributed set of inline widgets, performing URL-escaping on keys and values, and assembling a standards-compliant GET/POST query string for application callbacks.
*   **Filename:** `HTML2.w`

### 134. Dynamic Inline Widget Instantiation
Creating and embedding diverse Motif widgets (`xmTextField`, `xmPushButton`, `xmScrolledList`) into a text stream on-the-fly during document parsing, using the parent's `add_inline` method for layout integration.
*   **Filename:** `HTML2.w`

### 135. Resource Database Merging
Using `XrmMergeDatabases` during widget initialization to programmatically inject hardcoded application defaults (e.g., `*TEXTAREA.savecontent:TRUE`) into the widget's internal style database.
*   **Filename:** `HTML2.w`

### 136. Tag-triggered Internal Style Stacks
Managing complex element-specific state (like `SELECT` options or `TEXTAREA` content) by leveraging the hierarchical `pop_style` and `push_style` hooks to capture and buffer nested text data.
*   **Filename:** `HTML2.w`

## New Features from SSGML.w

### 137. Resource Database as Stylesheet
Using an `XrmDatabase` to store and query formatting rules, where the resource hierarchy (e.g., `*DIV*P.textcolor`) mirrors the SGML element nesting, enabling powerful and declarative document styling.
*   **Filename:** `SSGML.w`

### 138. Recursive Attribute Macros (`@ifmatch`)
Implementation of a mini-expression evaluator within the stylesheet that supports conditional logic (e.g., `@ifmatch(!TYPE,"SUBMIT",...)`) and regular expressions to dynamically derive formatting from SGML attributes.
*   **Filename:** `SSGML.w`

### 139. Hierarchical Style Stack
Managing document state with a linked list of `FormatInfo` structures, where each node inherits properties from its parent and can override them based on stylesheet lookups for the current element.
*   **Filename:** `SSGML.w`

### 140. Token-based Lexical Scanner
A robust `get_token` implementation that handles SGML-specific syntax including tags (`<TAG>`), entities (`&entity;`), character data, and significant whitespace, supporting incremental parsing of buffered text.
*   **Filename:** `SSGML.w`

### 141. Entity Resolution Engine
A system for mapping SGML entities to replacement text and specific character sets (ISO-8859-1, Symbols, WWW-Icons), with support for both named and numerical entities.
*   **Filename:** `SSGML.w`

### 142. Context-Sensitive Resource Lookups
Using `XrmQGetSearchList` with a list of quarks representing the current tag nesting to perform highly efficient, context-aware property retrieval from the stylesheet database.
*   **Filename:** `SSGML.w`

## New Features from Board.w

### 143. Absolute/Relative Geometry (Location Management)
Implementation of a dual-coordinate layout system where every dimension (x, y, width, height) is a sum of an absolute pixel value and a percentage of the parent's container area, enabling responsive and flexible layouts.
*   **Filename:** `Board.w`

### 144. Logical Coordinate Units (`hunit`, `vunit`)
Supporting resolution-independent design by providing multipliers for absolute coordinates, allowing "80 units" to represent 80 characters, 80 millimeters, or any other logical measure instead of raw pixels.
*   **Filename:** `Board.w`

### 145. Synthetic Location DSL
A compact string-based domain-specific language (e.g., `"0.5-20 5 40 1.0-50"`) that encodes complex layout rules, with a custom `scan` utility to parse and synchronize these strings with internal float/int resources.
*   **Filename:** `Board.w`

### 146. High-level/Low-level Geometry Sync
Sophisticated logic in `set_values` and `initialize` to maintain bidirectional synchronization between high-level "location" resources and standard X Toolkit "Core" geometry (`x`, `y`, `width`, `height`).
*   **Filename:** `Board.w`

### 147. Automatic Accelerator Propagation
Recursively installing accelerators for all descendants in the `change_managed` method, ensuring that keyboard shortcuts defined deep within a UI hierarchy are correctly registered with the top-level Shell.
*   **Filename:** `Board.w`

### 148. Recursive Inside-Area Clipping
Refining the layout boundaries by checking if a parent is a `Board` subclass and, if so, automatically basing child positions on the parent's "inside area" (after frames/margins) rather than its raw window size.
*   **Filename:** `Board.w`

## New Features from Toggle.w

### 149. Stateful Interaction Metaphor
Implementing a binary state button (`on`/`off`) where activation toggles an internal boolean resource and triggers state-specific callbacks (`onCallback`, `offCallback`).
*   **Filename:** `Toggle.w`

### 150. Dynamic Margin Calculation
Automatically recalculating and setting an inherited resource (`XtNleftMargin`) during initialization and property updates to accommodate variable-sized icons without manual layout adjustments.
*   **Filename:** `Toggle.w`

### 151. Clip Masked Icon Rendering
Using `XGCValues.clip_mask` and `XSetClipOrigin` within a GC to render state-indicator icons (tickmarks, checkboxes) that support transparency, overlaid next to the widget's text label.
*   **Filename:** `Toggle.w`

### 152. Icon-based State Visuals
Supporting the use of different `Icon` structures for the "on" and "off" states, providing a flexible way to implement radio buttons, checkboxes, or generic toggles.
*   **Filename:** `Toggle.w`

## New Features from ArtText.w

### 153. Semantic Text Stream Specialization
Specializing a generic `TextOut` widget to handle a specific data format (RFC-822 email/news) by overriding high-level data entry methods to perform semantic parsing.
*   **Filename:** `ArtText.w`

### 154. Context-Aware Font/Color Swapping
Dynamically mapping message components (Header Keys, Header Values, Quotes, Body) to distinct font and color resources (`color1-4`, `font1-4`) during the data ingestion phase.
*   **Filename:** `ArtText.w`

### 155. Quote-Detection Parsing Logic
Implementation of a line-by-line parser that identifies quoted text by checking for a user-definable `quoteChar` (defaulting to `>`) at the start of each line and adjusting formatting accordingly.
*   **Filename:** `ArtText.w`

### 156. Automatic Header/Body Transition
Using `strstr(data, "\n\n")` to automatically detect the boundary between a message header and its body, and applying different parsing strategies to each section.
*   **Filename:** `ArtText.w`

## New Features from Tabs.w

### 157. Complex Window Shaping (X Shape Region)
Combining multiple `XPolygonRegion` masks into a single complex window shape using `XShapeCombineRegion`, enabling widgets with non-rectangular features like diagonal "index card" corners.
*   **Filename:** `Tabs.w`

### 158. 4-Way UI Orientation Logic
Implementing a widget that can dynamically reorient its entire geometry and rendering logic (Up, Down, Left, Right) via a single resource, with specialized `comp_hor_tab_shape` and `comp_ver_tab_shape` methods.
*   **Filename:** `Tabs.w`

### 159. Rotated Text Rendering
Integrating with a rotation library (`rotated.h`, `XRotDrawAlignedString`) to draw text labels at 90 or -90 degree angles for vertical tab strips.
*   **Filename:** `Tabs.w`

### 160. Overlapping Geometry Z-Order Drawing
Manually managing the visual "stacking" of tabs by controlling the order of `XFillPolygon` calls (Left tabs, then Right tabs, then the active "Front" tab) to simulate physical depth.
*   **Filename:** `Tabs.w`

### 161. Relative Callback Indexing
Providing a "relative" index in the `activate` callback (e.g., -1 for previous, +1 for next, 0 for current) instead of absolute positions, simplifying application logic for tab switching.
*   **Filename:** `Tabs.w`

### 162. Percent-based vs. Content-based Sizing
Supporting both fixed percentage-based widths and dynamic content-based widths (based on `XTextWidth` of labels) within the same geometry engine.
*   **Filename:** `Tabs.w`

## New Features from XmPager.w

### 163. Motif Manager Specialization
Inheriting from `XmManager` to ensure the widget behaves as a standard Motif container, respecting Motif's visual style (shadows, highlights) and participating correctly in the Motif keyboard focus model.
*   **Filename:** `XmPager.w`

### 164. Redefining Geometry for Shadows
Manually adjusting internal drawing coordinates to account for the Motif `XmNshadowThickness`, ensuring that text and sub-widgets (icons) do not overlap with the standard 3D manager borders.
*   **Filename:** `XmPager.w`

### 165. Motif Sub-widget Composition
Using `XmIcon` (a Motif-adapted icon widget) as the basis for composite UI elements within a larger Motif-based document viewer.
*   **Filename:** `XmPager.w`

## New Features from XmTabs.w

### 166. Motif-Integrated Tab Rendering
Adapting complex region-based window shaping and rotated text drawing to the Motif `XmManager` class, ensuring compatibility with Motif's visual management system.
*   **Filename:** `XmTabs.w`

### 167. Shadow-Thickness-Aware Layout
Dynamically using the Motif `XmNshadowThickness` resource (`shad`) to offset tab base calculations and shadow line drawing, ensuring a consistent look with other Motif components.
*   **Filename:** `XmTabs.w`

## New Features from TextOut.w

### 168. Linked-List Chunk Rendering
Implementing a rendering engine that processes a linked list of `TextChunk` structures, each specifying its own text, font index, color index, and line-break status.
*   **Filename:** `TextOut.w`

### 169. Dynamic GC Attribute Mapping
Using `XtAllocateGC` with a `dynamic_mask` to manage a single GC that can be rapidly reconfigured for different fonts and colors during a single `expose` cycle.
*   **Filename:** `TextOut.w`

### 170. Heterogeneous Line Height Calculation
Algorithmically determining the maximum ascent and descent for a line by scanning all chunks in that line and taking the supremum of their respective font metrics.
*   **Filename:** `TextOut.w`

### 171. Tab-Stop Calculation Logic
Manual implementation of tab stops by scanning strings for `\t` and calculating character-width offsets, using a fixed 40-pixel grid for alignment.
*   **Filename:** `TextOut.w`

### 172. Region-based Drawing Optimization
Using `XRectInRegion` during the `expose` method to skip the drawing of text chunks that fall outside the current update region, improving performance for long documents.
*   **Filename:** `TextOut.w`

### 173. Batch-Update Sizing Strategy
Providing an `XfwfAddText` method that only recomputes widget geometry when a `NULL` data pointer is passed, allowing for efficient batch loading of many text chunks without repeated layout recalculations.
*   **Filename:** `TextOut.w`

## New Features from Frame.w

### 174. Generic 3D Framing Engine (`XfwfDrawFrame`)
An exported, reusable utility for drawing diverse 3D borders (Raised, Sunken, Chiseled, Ledged) by manually calculating polygon points and filling them with light/dark shadow GCs.
*   **Filename:** `Frame.w`

### 175. Depth-Sensitive "Auto" Shadow Schemes
Logic that automatically chooses between true color shadows or 50% stippled shadows based on the available `DefaultDepthOfScreen`, ensuring consistent 3D effects on both monochrome and color displays.
*   **Filename:** `Frame.w`

### 176. Proxy Geometry Negotiation
Implementing a wrapper pattern where `query_geometry` and `geometry_manager` calls are transparently passed between a parent and a single child, with the parent adding/subtracting its own frame dimensions during the process.
*   **Filename:** `Frame.w`

### 177. Action-driven Property Animation (`set_shadow`)
Providing a `set_shadow` action that allows translations to temporarily override the `frameType` (e.g., during a button press) and immediately redraw the frame without a full `set_values` cycle.
*   **Filename:** `Frame.w`

### 178. Resource-driven Padding Model
A hierarchy of padding resources (`outerOffset`, `frameWidth`, `innerOffset`) that work together to define the "inside" drawable area, allowing for precise control over widget-child separation.
*   **Filename:** `Frame.w`

## New Features from AnsiTerm.w

### 179. Virtual Window Scrolling System
Implementing a "virtual" drawing area that can be larger than the physical widget window, managed via `xoffset` and `yoffset` pixels, with clipping regions (`XSetClipRectangles`) to ensure text stays within the frame.
*   **Filename:** `AnsiTerm.w`

### 180. Scrolling Widget Interface Policy (SWIP) Integration
Participation in a standardized scrolling protocol by exporting a `scrollResponse` method and triggering a `scrollCallback` with detailed `XfwfScrollInfo` (vpos, vsize, hpos, hsize), allowing seamless connection to external scrollbars.
*   **Filename:** `AnsiTerm.w`

### 181. Fractional and Page-based Scrolling
Support for diverse scrolling "reasons" (Up, Down, PageUp, Drag, Move) with manual calculation of new offsets and notification of changes back to the scrolling chain.
*   **Filename:** `AnsiTerm.w`

### 182. Structure-Aware Initialization
Using `StructureNotifyMask` and a `map_handler` to defer timer-based initialization (like cursor blinking) until the widget is actually mapped to the screen, improving startup performance and resource usage.
*   **Filename:** `AnsiTerm.w`

### 183. Dual Selection Models (Simple vs. Multi-line)
Handling two different data extraction strategies for text selection: a single-line buffer for simple drags and a newline-intercalated buffer for selections spanning multiple terminal rows.
*   **Filename:** `AnsiTerm.w`

## New Features from Rows.w

### 184. Variable-Height Row Packing
A layout algorithm that packs children into horizontal rows of variable height, where each row's height is determined by the tallest child in that specific row, rather than a global grid.
*   **Filename:** `Rows.w`

### 185. Greedy Line-Breaking Layout
Implementing a "text-flow" style layout for widgets using a greedy algorithm: children are added to the current row until the next child would exceed the parent's width, at which point a new row is started.
*   **Filename:** `Rows.w`

### 186. Per-Row Vertical Alignment
Supporting an `alignTop` boolean that toggles whether children are aligned to the top of their row or the bottom, providing flexible alignment for heterogeneous widget sets.
*   **Filename:** `Rows.w`

### 187. Integrated Geometry Syncing
A compact `layout` method that combines child measurement and positioning in a single pass, ensuring the layout is always consistent with the latest child dimensions.
*   **Filename:** `Rows.w`

## New Features from Scrollbar.w

### 188. Sub-widget Aggregation Pattern
A design where a high-level widget (`XfwfScrollbar`) contains no custom drawing logic of its own, but instead acts as a coordinator for multiple specialized sub-widgets (`XfwfArrow`, `XfwfSlider2`).
*   **Filename:** `Scrollbar.w`

### 189. Callback Event Unification
Redirecting internal sub-widget callbacks (e.g., slider drags, arrow clicks) to a single parent-level `scrollCallback`, providing a simplified and consistent API for the end-user while handling diverse internal event types.
*   **Filename:** `Scrollbar.w`

### 190. Closed-Composite "Forbidden Child" Policy
Redefining `insert_child` to throw a warning if an application tries to add external widgets to the composite, effectively treating the widget as a "primitive" component despite its multi-widget internal structure.
*   **Filename:** `Scrollbar.w`

### 191. Proportional Sizing Suggestions
Implementing logic that suggests new scroll positions (e.g., adding/subtracting a `step`) within `XfwfScrollInfo`, allowing scrollees to either adopt the suggestion or perform their own custom bounded arithmetic.
*   **Filename:** `Scrollbar.w`

### 192. Dynamic Orientation-Switched Sub-widgets
Configuring internal children (like arrow directions and slider layout) during initialization based on a single `vertical` resource, ensuring the composite component correctly adapts its internal structure.
*   **Filename:** `Scrollbar.w`

## New Features from ScrollWin.w

### 193. Multi-level Composite Nesting
A highly structured composite widget that instantiates its own hierarchy of internal children (`XfwfFrame`, `XfwfBoard`, `XfwfScrollbar`) to provide a complex UI service (scrolling) with automatic clipping.
*   **Filename:** `ScrollWin.w`

### 194. Recursive Child Redirection
Implementing a "proxy" parent pattern where `insert_child` intercepts external widgets and programmatically re-parents them into a deep internal descendant (`$board`), while maintaining the illusion of a single container.
*   **Filename:** `ScrollWin.w`

### 195. Event-Driven Sub-widget Synchronization
Using `XtAddEventHandler` with `StructureNotifyMask` on a child widget to automatically detect when the child resizes or moves, allowing the parent to update its scrollbars in real-time without explicit method calls.
*   **Filename:** `ScrollWin.w`

### 196. Negative-Coordinate Clipping Metaphor
Implementing scrolling by allowing a large "controlled widget" to have negative `x`/`y` coordinates relative to its clipped parent (`$board`), with scrollbars mapping their 0.0-1.0 range to the physical pixel bounds of the child.
*   **Filename:** `ScrollWin.w`

### 197. Cascading Resource Defaults (`CallProc`)
Using a `CallProc` to link the defaults of two independent resources (e.g., `hScrollAmount` defaults to `vScrollAmount`), simplifying configuration for symmetric UI components.
*   **Filename:** `ScrollWin.w`

## New Features from ScrollWin3.w

### 198. Synchronized Multi-Axis Scrolling
Implementing a layout where multiple widgets (Column Header, Row Header, Main Data) scroll in partial synchronization: the Column Header scrolls only horizontally, the Row Header only vertically, and the Main Data scrolls in both directions.
*   **Filename:** `ScrWin3.w`

### 199. Advanced Ordered-Child Redirection
A complex `insert_child` logic that maps the first four external children to specific internal boards (Top-Left, Column Header, Row Header, Main Data) based on their insertion order, creating a structured spreadsheet-like container.
*   **Filename:** `ScrWin3.w`

### 200. Cross-Axis Geometry Syncing
Automatically propagating the width of the main controlled widget to its column header and its height to its row header during insertion, ensuring visual alignment between data and headers.
*   **Filename:** `ScrWin3.w`

### 201. Global SWIP Scrolling API (`XfwfScrollTo`)
Implementing a generic, protocol-based scrolling function that works with any widget following the Scrolling Widget Interface Policy (SWIP) by querying the `scrollResponse` resource.
*   **Filename:** `ScrWin3.w`

### 202. Multi-Board Layout Management
Configuring a complex internal geometry composed of four clipped boards (`ulboard`, `cboard`, `rboard`, & `board`) and two scrollbars, with manual spacing and header-dimension resources.
*   **Filename:** `ScrWin3.w`

## New Features from Slider4.w

### 203. 4-Degree-of-Freedom Interaction
An extension of the 2D slider metaphor that adds resizing capabilities (width/height), allowing a single widget to control both the position and the magnitude of a data viewport.
*   **Filename:** `Slider4.w`

### 204. Triangular Region Hit-Testing
Implementing a hit-test for a triangular "sash" in the corner of a rectangular thumb by using a linear inequality (`y >= -x + b`) to distinguish between drag and resize operations.
*   **Filename:** `Slider4.w`

### 205. Contextual Action Branching
Using a single `start` action that dynamically sets internal flags (`drag_in_progress` vs. `resize_in_progress`) based on sub-region coordinates, allowing for complex modal interactions without adding new translations.
*   **Filename:** `Slider4.w`

### 206. Sub-region Redraw Optimization
Efficiently handling resizing by only clearing and redrawing the union of the `old` and `new` thumb rectangles during a drag operation, minimizing flicker during interactive resizing.
*   **Filename:** `Slider4.w`

### 207. Multi-layered 3D Sash Geometry
Drawing a complex 3D triangular button (sash) by manually filling multiple polygons (`XFillPolygon`) with appropriate light/dark GCs to match the widget's overall 3D theme.
*   **Filename:** `Slider4.w`

## New Features from PullDown.w

### 208. Spring-Loaded Popup Menus
Using `XtPopupSpringLoaded` to implement standard menu behavior where a button press displays a menu and releasing the button outside the button area (or over a menu item) triggers a selection and dismissal.
*   **Filename:** `PullDown.w`

### 209. Just-in-Time Widget Creation
Automatically instantiating an `XfwfTextMenu` sub-widget during initialization only if a string-based `menu` resource is provided, allowing for flexible declarative or programmatic menu assignment.
*   **Filename:** `PullDown.w`

### 210. Automatic Coordinate Translation (`XtTranslateCoords`)
Dynamically calculating the screen coordinates for a popup menu by translating the button's local (0, height) coordinate to absolute root window coordinates, ensuring the menu appears perfectly aligned below the button.
*   **Filename:** `PullDown.w`

### 211. Grab-Action Registration
Using `XtRegisterGrabAction` in `class_initialize` to ensure that specific actions (like `open_menu`) automatically trigger the necessary pointer and keyboard grabs required for modal-like menu interaction.
*   **Filename:** `PullDown.w`

### 212. Hotkey to Accelerator Expansion
Implementation of a `hotkey` resource that programmatically expands a simple key string (e.g., `"Alt<Key>a"`) into a full `XtAccelerators` table that triggers menu actions from anywhere in the application.
*   **Filename:** `PullDown.w`

### 213. Callback Proxying and Transformation
Acting as a proxy for internal menu callbacks, transforming menu-level `call_data` (like item indices) into parent-level notification events for the application.
*   **Filename:** `PullDown.w`

## New Features from ThWheel2.w

### 198. Bitmap-driven Visual Fallback
Optimizing for monochrome or limited-color environments by using `XCreatePixmapFromBitmapData` instead of full-color XPM pixmaps, ensuring the mechanical "wheel" effect is functional even without a color display.
*   **Filename:** `ThWheel2.w`

### 199. Specialized Linear Constraint
Implementing a geometry model that assumes vertical orientation only, simplifying the coordinate math for hit-testing and dragging compared to the multi-directional `ThWheel.w`.
*   **Filename:** `ThWheel2.w`

### 200. Embedded Bitmap Resource Pattern
Linking against static C header files containing raw bitmap bits (`wheel0.bm`, etc.) to provide built-in default visuals that do not require external file loading.
*   **Filename:** `ThWheel2.w`

## New Features from HScrollb.w

### 131. Resource Default Overriding
Creating a specialized subclass specifically to change the default value of an inherited resource (`vertical = False`) to make the widget more intuitive for a specific use-case.
*   **Filename:** `HScrollb.w`

### 132. Directional Key Binding Logic
Providing a set of default keyboard translations that map direction-appropriate keys (`Left`, `Right`, `Home`) to generic superclass actions (`Scroll`), simplifying the creation of directional UI controls.
*   **Filename:** `HScrollb.w`
