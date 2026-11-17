# plan-xtcw-completed.md — Growing xtcw2 to build a real application

> **Status:** living document  
> **Created:** 2026-10-10  
> **Companion artifact:** [`cookbook/`](cookbook/) — a reference "coobook" (recipe
> manager) demo application used to exercise the whole toolkit end-to-end and to
> drive this plan.

This document is the *result* of building the `cookbook/` demo app. It answers
two questions:

1. **Cleanroom planning** — what should a recipe-management ("cookbook")
   application be able to do, and which widgets does each capability require?
2. **Gap analysis + roadmap** — which of those widgets/features are missing or
   incomplete in xtcw2 today, and in what order should they be added?

The `cookbook/` app deliberately does *not* need to be functionally complete. Its
job is to be a realistic, deployable reference project that stresses the whole
system (wbuild, widgets, LUI, themes, the `commander_runner` host, the build and
the deployment path) and thereby exposes the gaps collected below.

---

## Status snapshot (2026-11-17)

The original gap list below is kept as a historical record. Many of its P0/P1
items have since landed; this snapshot is the current source of truth.

**Landed since the first draft**

- Build/deploy: production mode honours `debug_enable`, `distclean` exists,
  CI checks out submodules and installs `bison`/`flex`/`swig`/`xvfb`/`x11-utils`.
- App chrome: `WAppWindow`, `WMenuBar`/`WPopupMenu`/`WAction`, `WToolBar`,
  `WStatusBar`, `WtabWidget`.
- Forms/inputs: `WComboBox` (read-only), `WRadioGroup`, `WLineEdit`,
  `WSpinBox`, `WCheckBox`, `WFormLayout`, `WProgressBar`.
- Data views: `WListView`, `table-view` (sortable first slice), `WTreeView`
  (scrolling, focus and keyboard navigation), `WDialog`.
- Cookbook: recipes are persisted as a Lua chunk, the editor does full
  create/update, delete uses a real themed confirm dialog, and the recipe list
  carries stable row ids so sorting no longer corrupts selection→model mapping.
- QA: single-widget probes for the new widgets plus `tests/test_cookbook_widgets.sh`
  covering geometry, menus, sorting, tree expand, dialog, CRUD and persistence.
- Rendering: `Wretex` no longer loops on repeated `autoHeight` text updates
  (`set_values_almost` now follows the Xt `request_mode` contract); the cookbook
  workaround was removed.
- Canvas: `DrawCanvas` now has key/mouse callbacks, timer-driven dirty repaints,
  optional double buffering and deferred focus. The interactive paint demo and
  the Snake game (`demos/demo_snake.lua`, `wbuild_widgets/snake-draw.c`) exercise
  the new API.

**Still open / needs rework**

| Area | Gap | Where |
|---|---|---|
| P0 widgets | No code yet for `WPlainTextEdit`, `WIconButton`, `WSearchField`. | `wbuild_widgets/`, `lui/registry.lua` |
| Model | `WModel`/proxy covers rows/ids/sort/filter, but not flags, delegates, tree nodes or a C-side fast path. | `lui/wmodel.lua`, `lui/gui_xt.lua` |
| Search/filter | Cookbook search is model-backed now; debounce/completion chrome still open. | `cookbook/app/main.lua` (`cb_search`), `WSearchField` |
| Data I/O | No JSON/structured binding; persistence writes Lua chunks. | `cookbook/app/store/persistence.lua` |
| Tree | Scrolling, focus and keyboard navigation landed; no lazy children/virtualization, no full model. | `wbuild_widgets/WTreeView.widget` |
| Keyboard/focus | `WRadioGroup`/`WComboBox` now acquire focus and handle selection keys; `WDialog` focuses its input/OK and handles Return/Escape. Full Tab trap/focus restore and `WAction.shortcut` wiring remain. | those widgets |
| Dialogs | `gui.alert`/`confirm`/`prompt` use `WDialog` (messageType/details/checkbox/input row); initial focus and the default Return button landed, full Tab trap/focus restore remain. | `lui/gui_xt.lua`, `WDialog.widget` |
| Table UX | Header click sorts and draws an asc/desc indicator (callback or local fallback); no cell editing, and local sort has no original-order restore. | `WListView.widget`, `cookbook/app/main.lua` |
| Theme | Surface roles/metrics (`menu_*`, `toolbar_*`, `status_*`, `dialog_*`, `tooltip_*`, `tree_*`, `table_*`, `progress_*`) are defined in every shipped theme and consumed by the widgets; screenshot baselines for each theme remain. | `themes.hdf`, `cookbook/themes/cookbook.hdf`, `tests/test_theme_roles.sh` |
| Host | `commander_runner` still hardcodes `../lui/?.lua` and relative theme paths; LUI load errors are printed but do not fail the process. | `commander/commander_runner.c` |
| Parser | LUI number grammar has no leading sign. | `lui/parser.lua` |
| QA | No screenshot baselines; keyboard/persistence/search paths are only partly covered. | `tests/` |

**Immediate next rework order**

1. ~~`WTreeView` scrolling + focus/keyboard~~ (landed 2026-11-17); lazy children/virtualization remain.
2. ~~Focus/keyboard for `WRadioGroup`/`WComboBox`; dialog default button and initial focus~~ (landed 2026-11-17); full Tab trap/focus restore remain.
3. ~~Theme roles/metrics across all themes~~ (landed 2026-11-17); per-theme screenshot baselines remain.
4. ~~Header sort indicator~~ (landed 2026-11-17); cell editing for `table-view` remains.
5. `WPlainTextEdit`, `WIconButton`, `WSearchField`.
6. Host location independence and LUI error propagation.
7. Visual baselines and the remaining keyboard/persistence tests.

---

## 0. Context: what "a project built with xtcw2" looks like

A concrete xtcw2 application is currently:

- a **C host** (`commander/commander_runner`, linking `libxtcw.a`, `libwcl.a`,
  `libutils.a`, `libretex.a`, `libplainc.a` and embedding Lua 5.3), which
- loads **LUI scripts** (Lua + S-expressions) that declare the widget tree, and
- pulls **widgets** from `wbuild_widgets/*.widget` (transpiled by `wbuild` to
  C) plus hand-written plain-C widgets (`plainc_widgets/`), and
- is themed through `themes.hdf` + the central theme registry.

The `cookbook/` app follows exactly this shape and adds the packaging/deployment
layer (Docker, per-mode builds, smoke test) that a real project needs.

---

## 1. Cleanroom planning: what a cookbook application must be able to do

The capability list below is written **without** looking at what xtcw2 happens to
support; it is what a user would expect from a normal desktop recipe manager.
Each capability is then mapped to the widget(s) that would realise it.

### 1.1 Capabilities

| # | Capability | Description |
|---|------------|-------------|
| C1 | **Recipe list / overview** | Browse all recipes in a list, select one to open. |
| C2 | **Search & filter** | Free-text search over title/ingredients; filter by category, tags, preparation time. |
| C3 | **Recipe detail** | Title, author, servings, time, category, tags; ingredients table; preparation steps. |
| C4 | **Recipe editor** | Create/edit a recipe: text fields, numeric fields, category chooser, tag toggles, multi-line ingredients and steps. |
| C5 | **Categories** | Assign and manage categories (starter, main, dessert, …). |
| C6 | **Tags / dietary flags** | Vegetarian, vegan, gluten-free, … as toggles. |
| C7 | **Ingredient table** | Rows of (amount, unit, ingredient) — ideally editable, sortable, with row add/remove. |
| C8 | **Shopping list** | Aggregate ingredients across selected recipes; check items off. |
| C9 | **Persistence** | Load/save recipes to disk (import/export). |
| C10 | **Recipe images** | Show a picture per recipe. |
| C11 | **Screen navigation** | Switch between list / detail / editor / shopping-list views. |
| C12 | **Application chrome** | Menu bar, toolbar (new/save/delete/search), status bar. |
| C13 | **Dialogs** | Confirm delete, unsaved-changes prompt, message boxes. |
| C14 | **Theming** | Light/dark theme support. |
| C15 | **Multi-language / units** | Metric/imperial conversion, localised labels (nice-to-have). |

### 1.2 Capability → widget mapping

Legend: ✅ available today · ⚠️ partially available · ❌ missing.

| Capability | Required widget(s) | xtcw2 today |
|------------|--------------------|-------------|
| C1 recipe list | `Wlist` / `Wlist4` / `WlistMulti` / `WlsMulti` (tag `list-view`) | ✅ |
| C2 search | `Wedit` + `Wbutton` | ✅ |
| C2 filter by category | `Wcombo`, `Woption`, `Wradio` | ✅ |
| C3 detail text | `Wlabel`, `Wretex`, `Wtext` | ✅ |
| C3 ingredient table | **`WtableView`** (multi-column, cell render + sort) | ❌ (only fixed-column `Wlist4`/`WlistMulti`) |
| C3 steps | `KaroEd` (multi-line) / `WeditMV` | ✅ (maturity ⚠️) |
| C4 editor fields | `Wedit`, `WspinBox`, `Wpassword` | ✅ |
| C4 category chooser | `Wcombo`, `Woption` | ✅ |
| C5 categories | `Wcombo`, `Wlist` | ✅ |
| C6 tags | `Wcheckbox`, `Wradio`, `WToggleSwitch` | ✅ |
| C7 editable table | **`WtableView` + `WgridView`** | ❌ |
| C8 shopping list | `WlistMulti` + `Wcheckbox` | ✅ |
| C9 persistence | **JSON / file read-write bindings** | ❌ (only `task_copy` file copy) |
| C10 images | `IconSVG` / `WpixBtn` (SVG only) | ⚠️ no raster (PNG/JPEG) widget |
| C11 navigation | **`WtabWidget`** (tab bar + content) or page switcher | ❌ |
| C12 menu bar | `Wmenu`, `WmenuPopup` | ⚠️ no container menubar |
| C12 toolbar | **`Wtoolbar`** | ❌ |
| C12 status bar | **`WstatusBar`** | ❌ |
| C13 dialogs | `MessageBox` + `gui.alert/confirm/prompt`, `WfileSelector`, `SelectReq` | ⚠️ partial |
| C14 theming | central theme registry (`Wheel.colors_changed`, `theme_select`) | ✅ |
| C15 units/i18n | data-layer concern, `Wlabel`/`Wcombo` | ⚠️ no i18n framework |

### 1.3 Additional cross-cutting needs (not widget-specific)

| # | Need | Why the cookbook app forces it |
|---|------|-------------------------------|
| X1 | **List/model data binding** | Editing a recipe must refresh the list; selecting a row must populate the detail view. Today the MLS store helpers exist but there is no automatic model↔widget refresh. |
| X2 | **Location-independent host** | A deployed app must find its LUI modules, theme and assets regardless of the working directory. `commander_runner` hardcodes `../lui/?.lua` and relative theme paths. |
| X3 | **Production build** | Deployment must be possible with `-O3` and **without** `-DMLS_DEBUG`/ASan. |
| X4 | **App template / packaging** | Developers need a copy-paste project skeleton (this `cookbook/`), plus a documented build/deploy story (Docker). |
| X5 | **Automated UI verification** | "It compiles" ≠ "it renders". The cookbook app needs start-smoke, geometry and visual-regression checks. |
| X6 | **Robust LUI error reporting** | A malformed UI must fail loudly, not silently show a blank window. |

---

## 2. Gap analysis: what xtcw2 is missing

> **Historical note:** the tables in §2 and the capability mapping in §1.2
> describe the state at the first draft. Use the *Status snapshot (2026-11-16)*
> at the top of this file for current status; the text below is retained to
> show why each gap existed.

Each gap below is directly motivated by the capability or cross-cutting need it
blocks. Priorities: **P0** = blocks deployment / correctness, **P1** = blocks a
core cookbook capability, **P2** = polish / nice-to-have.

### 2.1 Build, packaging & deployment

| ID | Gap | Evidence / impact | Priority |
|----|-----|-------------------|----------|
| G1 | **No real production build of `commander_runner`** — `commander/makefile` hardcodes `-DMLS_DEBUG -O0 -fsanitize=address`, ignoring `debug_enable`. | Blocks X3 and the Docker production target. *Fixed while building `cookbook/` (Step 2).* | P0 |
| G2 | **No `distclean` target**; `make clean` leaves `LuaRunner/luaxt.o`, `luaxt_wrap.o` (ASan-instrumented after a debug build) and removes the git-tracked `LuaRunner/luaxt_wrap.c`. | Mode switching debug↔production links stale ASan objects; worktree dirties. *Addressed in Step 2.* | P0 |
| G3 | **No application packaging / template** — no install target for apps, no project skeleton, no documented deployment. | Blocks X4. | P1 |
| G4 | **CI is broken and incomplete** — `.github/workflows/ci.yml` calls non-existent targets (`make distclean`, `make luarunner`, `cd experimental && make commander_runner`), omits `bison`/`flex`/`swig`, does not initialise the `LuaRunner/nanosvg` submodule, and misses `libxtst-dev` for `tests/run_all_tests.sh`. | Blocks X5 in CI. | P0 |

### 2.2 Host / runtime

| ID | Gap | Evidence / impact | Priority |
|----|-----|-------------------|----------|
| G5 | **`commander_runner` is not location-independent** — `package.path = '../lui/?.lua;'..…` is hardcoded and it must run from `commander/`; theme lookup only tries `../themes.hdf`/`themes.hdf`. | Blocks X2 (a deployed app cannot be relocated). | P1 |
| G6 | **LUI swallows errors** — `lui.run`/`lui.load` return `nil, err`; `lui_bootstrap` only prints `LUI load error:` and keeps running, so a broken UI shows a blank window that "starts fine". | Blocks X6; makes smoke tests give false positives. | P0 |
| G7 | **LUI parser has no negative number literals** — `:value -1` fails and can make the whole top-level list fail to parse. | Known bug (`know_bugs.md`). | P1 |

### 2.3 Widgets

| ID | Missing widget | Needed for | Priority |
|----|----------------|------------|----------|
| G8 | **`WtabWidget`** (tab bar + content area) | C11 screen navigation | P1 |
| G9 | **`Wmenubar`** (container for `Wmenu`/`WmenuPopup`) | C12 menu bar | P1 |
| G10 | **`Wtoolbar`** | C12 toolbar | P1 |
| G11 | **`WstatusBar`** | C12 status bar | P1 |
| G12 | **`WtableView`** (multi-column, editable, sortable) | C3/C7 ingredient table | P1 |
| G13 | **`WgridView`** (spreadsheet-like) | C7 advanced editing | P2 |
| G14 | **`WtreeView`** (hierarchical) | category tree (nice-to-have) | P2 |
| G15 | **`WdatePicker`** | recipe date fields | P2 |
| G16 | **`WsearchBox`** (input + icon + clear) | C2 search | P2 |
| G17 | **Raster image widget** (PNG/JPEG) | C10 (only SVG via `IconSVG` works) | P2 |

### 2.4 Data & model

| ID | Gap | Evidence / impact | Priority |
|----|-----|-------------------|----------|
| G18 | **No JSON / structured file read-write bindings** in the Lua layer | C9 persistence; only `task_copy` (binary copy) exists | P1 |
| G19 | **No automatic list/model data binding** — `gui.create_store`/`store_append` exist, but no widget refresh on model change, no selection→detail binding | X1 | P1 |
| G20 | **No multi-language / units framework** | C15 | P2 |

### 2.5 Quality assurance

| ID | Gap | Evidence / impact | Priority |
|----|-----|-------------------|----------|
| G21 | **No visual / geometry regression testing** — no screenshot baselines, no `width>0`/`height>0` assertions, no X11-error counter in the test suite | X5; historically the most common bugs (truncated text, 65534px widths) were invisible to the tests | P0 |
| G22 | **Widget verification is incomplete** — only a handful of widgets have TRACE(50) actions and property round-trips; most of the ~45 classes are untested | `experimental/ultimate_plan.md` Phase 9 | P1 |
| G23 | **No per-app smoke-test helper** for arbitrary projects (the existing `run_demo_tests.sh` is specific to `demos/` and needs scrot/PIL/numpy) | X4/X5 | P2 |

---

## 3. Growth roadmap

Prioritised work needed to turn xtcw2 into a toolkit that can carry an
application like `cookbook/`. Each item references the gap(s) and capability it
unblocks. **P0** = blocks shipping/deploying an app, **P1** = blocks a core
cookbook capability, **P2** = polish.

### Phase A — Build, packaging & deployment (P0)

| Task | Addresses | Status |
|------|-----------|--------|
| A1. Honour `debug_enable` in `commander/makefile` (real production build, no `-DMLS_DEBUG`/ASan). | G1, X3 | ✅ done |
| A2. Add a root `distclean` target; make `clean` remove `LuaRunner/luaxt*.o`; stop tracking generated `luaxt_wrap.c`. | G2 | ✅ done (`.gitignore`) |
| A3. Stop tracking build artifacts (the `tests/test_*` binaries) so `make clean`/`distclean` no longer dirties the worktree. | G2, build hygiene | ⬜ open |
| A4. Fix CI (`.github/workflows/ci.yml`): `submodules: recursive`, correct targets, add `bison flex swig libxtst-dev x11-utils`, `make clean` before test builds. | G4 | ✅ done |
| A5. Document the real prerequisites (`lua-lpeg`, `swig`, `bison`, `flex`, submodule init) in `overview.md`. | G3/G4 | ✅ done |
| A6. An app template / `dist` packaging story (the `cookbook/` layout + Docker is a starting point). | G3, X4 | 🟡 partial (`cookbook/`) |

### Phase B — Host/runtime robustness (P0–P1)

| Task | Addresses | Status |
|------|-----------|--------|
| B1. Propagate LUI errors: make `lui_bootstrap` fail (non-zero) when `lui.load`/`lui.run` returns an error; consider a `-LuiStrict` switch. | G6, X6 | ⬜ open |
| B2. Add optional leading sign to the LUI `number` grammar rule so negative literals parse. | G7 | ⬜ open |
| B3. Make `commander_runner` location-independent: resolve `lui/`, theme and app assets relative to an install prefix / `-LuiPath` / environment, not the cwd. | G5, X2 | ⬜ open |
| B4. Add a per-app smoke-test helper so any project (not just `demos/`) can run the headless start check (generalise `cookbook/scripts/smoke-test.sh`). | G23, X5 | 🟡 partial (`cookbook/`) |

### Phase C — Core widgets (P1)

| Task | Addresses | Status |
|------|-----------|--------|
| C1. Fix `WlsMulti`/`WlistMulti` `BadAlloc` on `X_CreatePixmap` when nested in a `Gridbox` (size audit + regression test). | G12-adjacent, `know_bugs.md` | ⬜ open |
| C2. `WtableView`: multi-column, cell-based, editable, sortable (replaces the `Wlist4`/`WlistMulti` stopgaps). | G12 | 🟡 partial (`table-view` first slice: sort + row ids; editing/filtering open) |
| C3. `WtabWidget`: tab bar + content pages (the cookbook's screen switcher). | G8 | ✅ done |
| C4. `Wmenubar` container for `Wmenu`/`WmenuPopup`. | G9 | ✅ done (`WMenuBar`/`WPopupMenu`/`WAction`) |
| C5. `Wtoolbar` and `WstatusBar` application chrome. | G10, G11 | ✅ done (`WToolBar`, `WStatusBar`) |
| C6. List/model data binding: model change → widget refresh, selection → detail binding. | G19, X1 | 🟡 partial (`WListView.rowIds` + cookbook refresh/selection; shared `WModel` open) |
| C7. Harden `MessageBox` + a small standard-dialog set (alert/confirm/prompt are already partly in `gui_xt.lua`). | C13 | 🟡 partial (`WDialog` landed; `WMessageBox`/`WInputDialog` and focus trap open) |

### Phase D — Data & content (P1–P2)

| Task | Addresses | Status |
|------|-----------|--------|
| D1. JSON (or equivalent) read/write bindings for the Lua layer. | G18, C9 | ⬜ open (cookbook persists Lua chunks via `io`) |
| D2. General file I/O helpers (beyond the single `task_copy` C binding). | G18 | 🟡 partial (`persistence.lua` uses plain `io`/`os.execute`) |
| D3. Raster image widget (PNG/JPEG), complementing `IconSVG`. | G17, C10 | ⬜ open |
| D4. `WdatePicker` and `WsearchBox`. | G15, G16 | ⬜ open |
| D5. `WgridView` / `WtreeView`. | G13, G14 | 🟡 partial (`WTreeView` slice with scrolling/focus/keyboard; lazy/model open) |
| D6. Localisation / unit-conversion framework. | G20, C15 | ⬜ open |

### Phase E — Quality assurance (P0–P1)

| Task | Addresses | Status |
|------|-----------|--------|
| E1. Visual/geometry regression framework: screenshot baselines, `width>0`/`height>0` assertions, X11 error counter. | G21, X5 | 🟡 partial (geometry assertions + widget probes; no baselines) |
| E2. Widget verification matrix: TRACE(50) + property round-trip for every widget class. | G22 | 🟡 partial (probes for the new widgets; many legacy classes untested) |
| E3. Add the `cookbook/` smoke test to CI (build both modes, run `test-all.sh`). | G4, E1 | 🟡 partial (`tests/test_cookbook_widgets.sh` exists; CI wiring open) |

### Suggested order

1. **A3–A5, A4/E3** (build hygiene, CI, docs) — cheap and unblocks automated
   verification.
2. **B1–B3** (error propagation, parser sign, location independence) — makes the
   host trustworthy for deployed apps.
3. **C1** (list `BadAlloc`) — a concrete crash that blocks any list-heavy UI.
4. **C2–C7, D1–D2** — the widgets and data plumbing the cookbook app actually
   wants.
5. **E1–E2** — lock the gains in with automated visual/geometry tests.
6. **A6, D3–D6** — polish, distribution and nice-to-haves.

Every item maps back to a capability or gap in §1/§2, so progress on the roadmap
is measurable by whether the `cookbook/` app can do more without workarounds.

