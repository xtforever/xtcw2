# XTCW2 Documentation Index

> Version 1.2 | Last updated: 2026-04-13

## Documentation Files

| File | Content | Source |
|------|---------|--------|
| [architecture.md](architecture.md) | Project structure, build pipeline, wbuild, C host, TRACE mechanism | Consolidated from `overview.md`, `learn.md` |
| [widgets.md](widgets.md) | Widget class reference, LUI registry, known bugs, TRACE(50) status | Consolidated from `WIDGETS.md`, `learn.md` |
| [lui.md](lui.md) | LUI framework, S-expression syntax, property mapping, callbacks, C bindings, tests | Consolidated from `lui-summary.md`, `howto.md`, `learn.md` |
| [mls.md](mls.md) | MLS library: lists, strings, debugging | Consolidated from `how-to-use-mls.md` |
| [testing.md](testing.md) | Test plan: categories (E/P/A/L/T), measures, infrastructure | New (`test_plan.md`) |
| [widget-demos.md](widget-demos.md) | Shell-pasteable demo commands for all widgets, layouts, SVG, scrolling, etc. | New |
| [retex.md](retex.md) | Retex layout engine API, coordinate transformation, first_line_height | New (2026-04-15) |
| [xt-geometry.md](xt-geometry.md) | X Toolkit Intrinsics geometry protocol: query_geometry, set_values_almost, resize chain | New (2026-04-15) |
| [gridbox.md](gridbox.md) | Gridbox layout widget: fill modes, weights, gravity, geometry manager | New (2026-04-15) |
| [test_plan.md](../test_plan.md) | Same as testing.md (in project root for backward compat) | — |

## Legacy Documentation (superseded)

These files still exist in the project root but their content has been consolidated into `docs/`:

| Legacy File | Replaced By | Status |
|-------------|-------------|--------|
| `overview.md` | `docs/architecture.md` | Superseded |
| `WIDGETS.md` | `docs/widgets.md` | Superseded |
| `lui-summary.md` | `docs/lui.md` | Superseded |
| `how-to-use-mls.md` | `docs/mls.md` | Superseded |
| `howto.md` | `docs/lui.md` | Superseded |
| `learn.md` | Multiple `docs/` files | Still current (contains test automation findings) |
| `errors.md` | `docs/widgets.md` (bug table) + `learn.md` | Still current |
| `ultimate_plan.md` | — | Still current (updated) |
| `test_plan.md` | `docs/testing.md` | Duplicate |

## Key References by Topic

### Getting Started
- Building: `docs/architecture.md` → Build Pipeline
- Creating a widget: `docs/architecture.md` → wbuild Widget Transpiler
- Running LUI scripts: `docs/lui.md` → Running LUI Scripts

### Running & Verifying Widgets
- Pastable demo commands: `docs/widget-demos.md`
- Quick verification: `docs/widget-demos.md` → All Widgets smoke test
- TRACE(50) regression: `docs/widget-demos.md` → Section 13
- Visual demos: `docs/widget-demos.md` → Sections 1-14
- Headless testing: `docs/widget-demos.md` → Appendix

### Widget Development
- Widget class list: `docs/widgets.md` → Core Widgets
- LUI tag → class mapping: `docs/widgets.md` → LUI Registry
-TRACE(50) status: `docs/widgets.md` → TRACE(50) column in each table
- Known bugs: `docs/widgets.md` → Known Widget Bugs

### Testing
- Test categories: `docs/testing.md` → Test Categories (E/P/A/L/T)
- Current test status: `docs/lui.md` → Test Infrastructure
- Bug patterns to guard against: `docs/testing.md` → Lessons Learned
- Widget verification checklist: `ultimate_plan.md` → Verification Checklist

### Memory Management
- MLS API: `docs/mls.md`
- Debug mode: `docs/mls.md` → Debugging
- Leak detection: `docs/mls.md` → Leak Detection Output

### Architecture & Internals
- Project structure: `docs/architecture.md` → Directory Structure
- C bindings: `docs/lui.md` → lua_bridge C Bindings
- Callback dispatch: `docs/lui.md` → Callback Dispatch Latency
- Command widget guard: `docs/lui.md` → xtaction and Command Widget

### Planning
- Overall plan: `ultimate_plan.md`
- Current test status: `ultimate_plan.md` → Critical Findings
- Missing widgets: `docs/widgets.md` → Missing Widgets