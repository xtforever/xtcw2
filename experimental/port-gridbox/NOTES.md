# Gridbox → wbuild `.widget` port experiment

This directory is an isolated proof of the porting pipeline:

1. extract the resource/constraint surface from a working plain-C widget,
2. declare it in a thin `.widget`,
3. keep the existing C implementation in a library,
4. let wbuild generate the public/private headers and the class record,
5. make every `.widget` method a one-line delegate into the library.

The production files are untouched:

- `plainc_widgets/Gridbox.c`
- `plainc_widgets/GridboxP.h`
- `plainc_widgets/Gridbox.h`
- `wbuild_widgets/Gridbox.widget` (still `@nocode`)
- `build/`, `lui/`, root makefiles

## Files

| File | Purpose |
|---|---|
| `Gridbox.widget` | thin wbuild wrapper: class, resources, constraint fields, method delegates |
| `gridbox_core.c` | copy of `plainc_widgets/Gridbox.c`; class record removed, adapter functions appended |
| `gridbox_core.h` | exported `gridbox_core_*` adapter API |
| `gridbox_compat.h` | public `XtNgrid*`, `XtNfill`, `FillType`, `Fill*` API that wbuild no longer emits for `@private-constraints` |
| `test_port_gridbox.c` | self-checking 2x2 / weight / span / fill test |
| `abi_check.c` | prints old vs new struct sizes and offsets |
| `Makefile` | experiment-local wbuild + compile + run |

## Build and run

```sh
make -C experimental/port-gridbox
make -C experimental/port-gridbox run
```

`run` executes:

- `test_port_gridbox_orig` linked against `libplainc.a` (original class),
- `test_port_gridbox` linked against `libgridboxport.a` (wrapper + core, **no** `libplainc.a`),
- `abi_check_orig` vs `abi_check_port` with a `diff`.

## Resource extraction

| Source | wbuild |
|---|---|
| `resources[]` (`defaultDistance`) | `@PUBLIC int defaultDistance = 4`; core keeps the original `XtCThickness` table and installs it in `class_initialize` |
| `gridboxConstraintResources[]` | `@private-constraints` fields + core-owned `XtResource` table installed in `class_initialize` |
| `GridboxPart` private state | `@PRIVATE` |
| `GridboxConstraintsPart` fields | `@private-constraints` (public constraint resources first, then `prefWidth`, `prefHeight`, `queried`) |
| class record flags | `@CLASSVARS` (`compress_motion`, `compress_exposure`, `compress_enterleave`, `visible_interest`) |
| class methods | `@METHODS` delegating to `gridbox_core_*` |

`@CONSTRAINTS` is deliberately **not** used: existing widgets (`WPaned`,
`WtabWidget`, `WAppWindow`) document that wbuild's `@constraints` generation is
not reliable. The wrapper uses `@private-constraints` and registers the
constraint table manually, exactly like those widgets.

## Core split

The copied `gridbox_core.c` keeps all layout algorithms and static helpers. Only
two changes were made to the body:

- `#include "GridboxP.h"` → `#include <xtcw/GridboxP.h>` plus `gridbox_core.h`
- the hand-written `GridboxClassRec` / `gridboxWidgetClass` definitions were removed

The original static functions keep their names. A small adapter block at the end
of the file exposes `gridbox_core_*` entry points; the `.widget` methods call
those. This keeps the port mechanical and makes the wrapper easy to audit.

## Verified result

`make run` passes:

```
PASS: ./test_port_gridbox_orig (x11_errors=0, xt_warnings=0)
PASS: ./test_port_gridbox      (x11_errors=0, xt_warnings=0)
```

`abi_check_orig` and `abi_check_port` produce identical output for:

- `sizeof(GridboxClassRec)`, `sizeof(GridboxPart)`, `sizeof(GridboxRec)`,
  `sizeof(GridboxConstraintRec)`
- offsets of `defaultDistance`, `nx`, `ny`, `maxgw`, `maxgh`, `max_wids`,
  `wids`, `total_wid`, `total_weightx`, `needs_layout`
- offsets of `gridx`, `gridy`, `gridWidth`, `gridHeight`, `fill`, `gravity`,
  `weightx`, `weighty`, `margin`, `prefWidth`, `prefHeight`, `queried`

`GridboxClassPart` keeps the historical 8-byte slot via an unused
`XtPointer gb_extension` class variable. wbuild treats a classvar named
`extension` as inherited from Core, so the field name differs, but the size and
alignment match the hand-written `XtPointer extension` exactly. No code reads
that slot, and `sizeof(GridboxClassRec)` is identical to the old header.

## Why the port still uses `@private-constraints` (verified)

The reference widgets (`WPaned`, `WtabWidget`, `WAppWindow`) register their
constraint table manually. This was re-verified against the current wbuild.

The original doc-generation crash has since been fixed in wbuild
(`wbuild/tex.w` had `@t contraints` instead of `@t constraints`;
`generatedoc.c` now also NULL-checks missing tags). The crash is kept here as
the historical reason for the manual pattern, but it is no longer the deciding
factor:

1. **Documentation generation used to crash.** A minimal `@CONSTRAINTS` widget
   generates C/headers fine, but with `-d ... tex.w nroff.w` wbuild exits with
   `SIGSEGV` (139):

   ```
   #0 __strlen_avx2
   #1 __GI__IO_fputs (str=0x0, ...)
   #2 constraint_resources (f=..., tag=..., c=..., shortdoc=0) at generatedoc.c:427
   #3 generate_doc ... at generatedoc.c:822
   #4 process_file ... at wbuild.c:201
   ```

   `constraint_resources()` called `fputs(topen(t_constraints), f)` without a
   NULL check. The real build always passes `tex.w`/`nroff.w`, so this was the
   normal failure mode. After the wbuild fix, the same `@CONSTRAINTS` widget
   generates docs successfully; the same `Gridbox.widget` using
   `@private-constraints` plus the manual table already did.

2. **Resource classes/types are not preserved.** Even with the crash fixed, a
   `@CONSTRAINTS` version of the Gridbox constraint set emits:

   | Original (plain C) | Generated by `@CONSTRAINTS` |
   |---|---|
   | `XtNgridx, XtCPosition, XtRPosition` | `XtNgridx, XtCGridx, XtRPosition` |
   | `XtNgridWidth, XtCWidth, XtRDimension` | `XtNgridWidth, XtCGridWidth, XtRDimension` |
   | `XtNweightx, XtCWeight, XtRInt` | `XtNweightx, XtCWeightx, XtRInt` |
   | `XtNgravity, XtCGravity, XtRGravity` | `XtNgravity, XtCGravity, XtRInt` |

   The class is derived from the field name and the type from the C type, which
   changes resource-database matching and the gravity converter. The manual
   core table keeps the exact original `XtC*`/`XtR*` pairs.

The manual table is still required for a faithful port because `@CONSTRAINTS`
loses the original `XtC*`/`XtR*` pairs. The runtime test therefore does not poke
at Xt's internal merged resource table; it reads all nine constraint resources
back through `XtVaGetValues` and asserts `margin == 4` after
`constraint_initialize`, which proves the manual table and constraint hooks are
active.

## Known deviations / findings

- **Class part field name:** the unused slot is named `gb_extension` instead of
  `extension`; its size/offset effect is identical and no code accesses it.
- **`geometry_manager` argument:** wbuild's `$` there is `XtParent(child)`, not
  the child. The wrapper must pass `child` to
  `gridbox_core_geometry_manager()`. Passing `$` caused a NULL constraint
  dereference in `WDialog`; this was found during the production swap and is
  fixed in both the production and experiment widgets.
- **Resource class for `defaultDistance`:** wbuild would derive
  `XtCDefaultDistance` from the resource name. The core table keeps the original
  `XtCThickness` and the wrapper installs it in `class_initialize`.
- **String → `FillType` converter:** the converter itself works. The earlier
  failure was a caller/API issue: raw `XtNfill, "both"` arguments are copied by
  Xt as raw `XtArgVal` values and never passed through the converter. Callers
  must use `XtVaTypedArg`. The active tests and the experiment now do so; the
  converter's `toVal->size` handling was also corrected. A raw string still
  stores a truncated pointer, which is why the old test looked broken.
- **Gridbox subclasses:** `Wls`, `MessageBox`, `WfileChooser`, `ScrolledCanvas`,
  `Radio2`, `SelectReq`, `WlsMulti` were dry-run regenerated and compiled
  against the generated header. A production replacement still requires the
  full rebuild described below.
- **MLS debug build:** the experiment compiles with `-DMLS_DEBUG` because the
  current `../../build` tree is a debug build and `libutils` uses the MLS debug
  wrappers. A release build would need this flag removed.


## Replacement readiness (dry-run)

The production replacement has now been applied: `wbuild_widgets/Gridbox.widget`
is the wrapper, `wbuild_widgets/gridbox_core.c` is the implementation, and
`plainc_widgets/Gridbox.*` has been removed. The dry-run results below are kept
as the validation record.

A production replacement was dry-run without touching the production widget:

- All seven Gridbox subclasses (`Wls`, `WlsMulti`, `MessageBox`,
  `ScrolledCanvas`, `SelectReq`, `Radio2`, `WfileChooser`) were regenerated with
  wbuild using the ported `Gridbox.widget` as superclass. All generated C files
  compile against the generated `Gridbox.h`/`GridboxP.h`.
- `tests/test_wfilechooser.c` linked with a recompiled `WfileChooser.o` and the
  ported Gridbox library passes (`test_wfilechooser passed`).
- Smoke tests instantiate and realize `Wls`/`WlsMulti` and
  `MessageBox`/`ScrolledCanvas`/`SelectReq`/`Radio2` with the ported Gridbox.
  `Radio2` additionally needs `svar_create()`/`mvar_init()`; that is unrelated
  to Gridbox.
- All test sources that include `<xtcw/Gridbox.h>` compile against the ported
  public header. `wcl/Xp.c` also compiles unchanged.
- ABI checks are identical old vs new:
  - `GridboxClassRec`, `GridboxPart`, `GridboxRec`, `GridboxConstraintRec`
  - `WlsClassRec` / `WlsRec` and their `gridbox_class`, `wls_class`,
    `gridbox`, `wls` offsets
  - `WfileChooserClassRec` / `WfileChooserRec` and the corresponding offsets

The `gb_extension` class variable is what keeps `GridboxClassPart` at the old
8-byte size, so subclass class-record layouts match exactly.

### Production migration steps

1. Replace `wbuild_widgets/Gridbox.widget` with the ported `.widget` and add
   `gridbox_core.c`, `gridbox_core.h`, `gridbox_compat.h`.
2. Remove `Gridbox.widget` from `NOCODE_WIDGETS` in
   `wbuild_widgets/makefile`.
3. Add `gridbox_core.c` to the hand-written sources copied to
   `build/source` by the root `libxtcw` target; the generated `Gridbox.c` is
   picked up by the `build/source` wildcard.
4. Install `gridbox_core.h` and `gridbox_compat.h` to
   `build/include/xtcw` (the `wbuild_widgets` install already copies `*.h`).
5. Remove `Gridbox.c` from `plainc_widgets/makefile` and retire
   `plainc_widgets/Gridbox.c`, `Gridbox.h`, `GridboxP.h`. Keep an empty
   `libplainc.a` or remove `-lplainc` from link lines.
6. Rebuild from scratch (`make clean && make all`) so every subclass class
   record and header is consistent with the generated Gridbox header.
7. Run the full test suite, especially `test_gridbox`, `test_gridbox_layout`,
   `test_wfilechooser`, `test_wdialog`, `test_wmenu*`, `test_wcombo*`,
   `test_wviewport` and the LUI demos.

### Caveats

- The pre-existing string→`FillType` converter issue remains; it is identical
  in the old and ported widget.
- `Radio2` needs its legacy `svar`/`var5` setup; that is not a Gridbox
  dependency.
- External/third-party code that reads the private `gridbox_class.extension`
  field would need a rename to `gb_extension`; nothing in this repository does.

## Next steps for a full port

1. Rebuild all Gridbox subclasses against the generated `GridboxP.h`.
2. Run `test_wfilechooser`, `test_wlistview`, `test_radio*`, `test_wls*` and the
   LUI demo suite.
3. Replace `wbuild_widgets/Gridbox.widget` (`@nocode`) and remove
   `plainc_widgets/Gridbox.c` only after the subclass rebuild is green.
4. Any remaining `test_fill_*` files that pass raw string fill arguments
   should be converted to `XtVaTypedArg` if they are promoted into the normal
   test suite.
