# Converting Standard Xt Widgets to wbuild Format

This guide explains how to convert a standard Xt widget (like `Grip.c` from libXaw) into the `.widget` format used by `wbuild`.

## 1. Class Declaration

In standard C, you define the class record and the widget class pointer. In `wbuild`, use the `@class` directive.

**Standard C (`Grip.c`):**
```c
#define Superclass (&simpleClassRec)
GripClassRec gripClassRec = { ... };
WidgetClass gripWidgetClass = (WidgetClass)&gripClassRec;
```

**wbuild (`Grip.widget`):**
```wbuild
@class Grip (Simple)
```
*Note: The parent class is specified in parentheses. `Simple` corresponds to `simpleWidgetClass`.*

## 2. Resources (@public)

Standard Xt resources are defined in an array. In `wbuild`, they go into the `@public` section.

**Standard C:**
```c
static XtResource resources[] = {
  {
    XtNcallback,
    XtCCallback,
    XtRCallback,
    sizeof(XtPointer),
    XtOffsetOf(GripRec, grip.grip_action),
    XtRCallback,
    NULL
  },
  // ... width, height, etc.
};
```

**wbuild:**
```wbuild
@public
@var Callback callback = NULL
@var Dimension width = 10
@var Dimension height = 10
@var border_width = 0
```
*Note: `wbuild` handles the `XtOffsetOf` and resource record generation automatically. If you are overriding a superclass resource default, just use `@var name = value`.*

## 3. Private Variables (@private)

Variables that would normally be in the `GripPart` of the `GripRec` (but aren't resources) go here.

**wbuild:**
```wbuild
@private
    int internal_state;
```

## 4. Actions (@actions)

Action functions are defined in an `actionsList` and then implemented.

**Standard C:**
```c
static XtActionsRec actionsList[] = {
  {"GripAction", GripAction},
};

static void GripAction(Widget widget, XEvent *event, String *params, Cardinal *num_params) {
    // ...
}
```

**wbuild:**
```wbuild
@actions
@proc GripAction
{
    XawGripCallDataRec call_data;
    call_data.event = event;
    call_data.params = params;
    call_data.num_params = *num_params;

    XtCallCallbacks($, XtNcallback, (XtPointer)&call_data);
}
```
*Note: `$` refers to the current widget instance (the `Widget` pointer). Variables like `event`, `params`, and `num_params` are automatically available in action procedures.*

## 5. Methods (@methods)

Class methods like `initialize`, `realize`, `expose`, and `set_values` are mapped to the `@methods` section.

**Standard C:**
```c
GripClassRec gripClassRec = {
  { /* core */
    NULL, // initialize
    XtInheritRealize, // realize
    // ...
  }
};
```

**wbuild:**
```wbuild
@methods

@proc initialize
{
    // Custom initialization logic
}

@proc expose
{
    // Custom drawing logic
}
```
*Note: If you don't define a method, `wbuild` will use the default or inherited one.*

## 6. Imports and Includes (@imports)

Standard `#include` directives go into `@imports`.

**wbuild:**
```wbuild
@imports
@incl <X11/Xaw/XawInit.h>
@incl <X11/StringDefs.h>
```

## 8. Constraint Widgets (@constraints)

Constraint widgets (like `Paned` or `Box`) manage extra resources for each child. In `wbuild`, these are defined in `@constraints`.

**WIP Warning:** There is a known bug in `wbuild` where defining resources in `@constraints` causes a segfault.

**Workaround for Constraint Resources:**
Use `@private-constraints` for the variable storage and manually register resources in `class_initialize`.

```wbuild
@class WPaned (Constraint)

@private-constraints
@var Dimension min
@var Dimension max

@methods
@proc class_initialize
{
    static XtResource resources[] = {
        {XtNmin, XtCMin, XtRDimension, sizeof(Dimension),
         XtOffsetOf(WPanedConstraintRec, wPaned.min), XtRImmediate, (XtPointer)1},
        // ...
    };
    ConstraintWidgetClass cwc = (ConstraintWidgetClass)wPanedWidgetClass;
    cwc->constraint_class.resources = resources;
    cwc->constraint_class.num_resources = XtNumber(resources);
}
```

## 9. Important wbuild Caveats

- **Naming Consistency:** If your class is `WPaned`, `wbuild` generates the part members as `wPaned` (lowercase first letter, then original case). Use `pw->wPaned.member` if `pw` is a typed widget pointer.
- **Macro Placement:** `@def` or `#define` in `@imports` or `@utilities` might fail if not placed carefully. Prefer placing complex logic or type-specific macros in an external header (e.g., `WPaned_types.h`) and include it via `@exports` and `@imports`.
- **Constraint Part Access:** Use a macro like `#define CPane(w) (&((WPanedConstraintRec*)((w)->core.constraints))->wPaned)` to access child constraints.
- **Inheritance:** When inheriting from `Constraint`, ensure the private header `X11/ConstraintP.h` is available. On some systems it might be named `X11/ConstrainP.h` (without 't'), requiring a compatibility symlink or header.
