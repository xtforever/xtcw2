# xtcw2 — Project Overview (for LLM agents)

> Entry point for agents and developers working in this repository.
> Last updated: 2026-09-24

## 1. What this project is

`xtcw` is a **toolkit construction set for the X Window System**. It lets you write
Xt-based widget sets with modern capabilities (Fontconfig, optional Cairo drawing,
SVG) and build GUIs from resource files (`.ad`) or Lua scripts. It is *not* a
replacement for Qt or GTK — it is a set of tools for building such a toolkit.

Core pieces:

- **wbuild** — a transpiler that turns `.widget` files into standard Xt C code (`.c`/`.h`).
- **mls** — the "Multiple List System", a handle-based memory/string library used everywhere.
- **re-tex** — a TeX-inspired text/layout engine (nodes, glue, line breaking, Cairo/X11 backends).
- **LUI** — a Lua layer that builds widget trees from S-expressions and drives them at runtime, hosted by the `commander_runner` C application.

---

## 2. How to work here (agent workflow)

This repo is set up for an openCode multi-agent loop:

- **plan** (`.opencode/agent/plan.md`) — breaks work into small steps, reviewed by the `review` subagent until approved.
- **review** (`.opencode/agent/review.md`) — strict step-list reviewer; read-only.
- **build** (`.opencode/agent/build.md`) — implements exactly one approved step at a time and stops when its done-condition is met.

Conventions the agents expect:

- **TDD / Red-Green-Refactor.** For any feature or bug fix: write a failing test first,
  confirm it fails, implement the minimum to pass, then refactor with tests green.
- **Commit after each change.** Use `git add` + `git commit` once a change works.
- **Log fixes.** After fixing an error, write down the cause and solution in
  `experimental/errors.md`.
- **Prefer examples.** When a feature is unclear, find and analyze a matching example
  (on the internet or in this repo) before guessing.
- **Graph-first editing.** `AGENTS.md` / `CLAUDE.md` require a GitNexus **impact analysis**
  before editing any symbol, and a **change analysis** before committing. Treat a
  `risk: UNKNOWN` result as unresolved (confirm with a text search), never as "safe".

---

## 3. Prerequisites

Build tools:

```bash
gcc make bison flex pkg-config
```

Development libraries:

- X11 / Xt / Xaw / Xmu / Xft / Xpm / Xext / Xrender / fontconfig
- **cairo** (and cairo-xlib) — required by the root build (`pkg-config cairo cairo-xlib`)
- **lua5.3** development package — required by `commander_runner` (`pkg-config lua5.3`)
- **MySQL client dev** (e.g. `libmysqlclient-dev`) — required *only* to build
  `experimental/AdmPnl` (it links `-lmysqlclient`)

Git submodules — this repo declares **one** submodule, `LuaRunner/nanosvg`:

```bash
git submodule update --init --recursive
```

> Note: `mls` is **in-tree** (`utils/mls.c`); `experimental/lua/` and `experimental/swig/`
> are gitignored directories, not submodules.

---

## 4. Building

The root `makefile` orchestrates everything out-of-source into `./build`.

```bash
make                 # full debug build (debug_enable defaults to 1: -g -DMLS_DEBUG -O0)
make debug_enable=0  # release build (-O3, no -DMLS_DEBUG)
make libxtcw         # rebuild the widget library only
make widgets         # regenerate .c/.h from .widget and rebuild
make plainc          # build plain-C widgets (e.g. Gridbox)
make clean           # remove build/ and object files
```

> There is **no `rebuild` and no `distclean` target** — those do not exist.
> `make` is already the debug build; use `make debug_enable=0` for release.

**Critical:** always build with consistent `CFLAGS`. Mixing `-DMLS_DEBUG` and non-debug
objects causes crashes. Use the root `makefile` rather than ad-hoc `gcc` invocations.

**Build pipeline (abridged):**

```
.widget files ──wbuild──→ .c + .h ──cc──→ libxtcw.a
plainc_widgets/*.c ──cc──→ libplainc.a
utils/*.c          ──cc──→ libutils.a
wcl/*.c            ──cc──→ libwcl.a
re-tex/src/*.c     ──cc──→ libretex.a
        └──────── all linked into commander_runner ─────────┘
```

The `.widget` file is the **source of truth**. Edits to generated `.c` files under
`build/source/` are overwritten by `wbuild` — edit `wbuild_widgets/*.widget` instead.
(`WpixBtn.c` is the hand-written exception: it is copied, not regenerated.)

---

## 5. Testing

```bash
make run_tests                 # builds and runs ./tests/run_all_tests.sh
./tests/run_all_tests.sh       # underlying runner (MLS, re-tex, Wlabel layout checks)
tests/test_retex_hit           # individual test binaries live in tests/
```

`tests/run_all_tests.sh` uses `tests/verify_layout.sh` against baselines in
`tests/baselines/`, and runs some layout tests under `xvfb-run --auto-servernum`.

**Commander smoke tests (LUI demos, headless):**

```bash
commander/run_demo_tests.sh    # builds commander_runner, runs demos + Wbutton under Xvfb
```

**Running LUI scripts manually** (must run from `commander/`, whose parent contains `lui/`):

```bash
cd commander
./commander_runner -Luafile ../demos/demo_login.lua
./commander_runner -Luafile ../demos/demo_form.lua -Tracelevel 5
xvfb-run --auto-servernum ./commander_runner -Luafile ../demos/demo_form.lua
```

**TRACE instrumentation.** `TRACE(level, ...)` prints to stderr only when a global
`trace_level` (default `0`) is at least `level`:

```bash
./commander_runner -Tracelevel 50 -Luafile my_script.lua   # show TRACE(50) output
```

**Known-broken script.** `experimental/lui_test/run_widget_tests.sh` does **not** work
as-is: it computes `PROJECT_DIR=$SCRIPT_DIR/..` (i.e. `experimental/`) and then runs
`experimental/experimental/commander_runner`, which does not exist (the binary is at
`commander/commander_runner`). Prefer `make run_tests` and `commander/run_demo_tests.sh`;
fix that script's path if you need it.

---

## 6. Running applications / examples

The root `make all` builds the toolkit, `commander_runner`, and tests — it does **not**
build the sample applications. Those live under `experimental/` with their own makefiles:

| App | Location | Build & run |
|-----|----------|-------------|
| **AdmPnl** (admin panel) | `experimental/AdmPnl/` | `cd experimental/AdmPnl && make && ./admpanel` (needs MySQL client dev) |
| **todomgr** | `experimental/todomgr/` | `cd experimental/todomgr && make` |
| **test_file_sel** | `experimental/test_file_sel/` | `cd experimental/test_file_sel && make` |

LUI demo scripts that `commander_runner` loads directly live in `demos/`.

---

## 7. Directory structure & important files

```
xtcw2/
├── wbuild/                 # wbuild transpiler source (produces build/bin/wbuild)
├── wbuild_widgets/         # CANONICAL widget source (.widget + hand-written .c/.h)
├── plainc_widgets/         # Hand-written plain-C widgets (Gridbox, …) → libplainc.a
├── re-tex/src/             # TeX-inspired layout engine → libretex.a
│   ├── node.c              #   node types (CHAR, HBOX, VBOX, GLUE, …)
│   ├── retex.c             #   high-level API, layout, hit testing
│   ├── renderer.c          #   Cairo/X11 drawing (selection highlights)
│   └── backend_cairo.c, backend_xpixmap.c
├── utils/                  # Foundation library → libutils.a
│   ├── mls.c               #   Multiple List System (handle-based memory)
│   ├── m_tool.c            #   conststr, focus groups, timers, vars
│   ├── var5.c              #   variable system
│   └── sig_xt.c            #   signal dispatch
├── wcl/                    # Widget Creation Library (WcCreate, WcRegister) → libwcl.a
├── commander/              # C host application: Xt + embedded Lua 5.3
│   ├── commander_runner.c  #   main (Xt init, Lua bootstrap, event loop)
│   ├── xt_bridge.c         #   xtaction / xtapptimeout / xtcreate bindings
│   ├── lua_bridge.c        #   Lua function registration
│   ├── file_ops.c          #   file I/O bindings
│   ├── lua_task_bindings.c, task_manager.c  # cooperative task/threading
│   └── run_demo_tests.sh   #   headless demo smoke tests
├── lui/                    # LUI modules (Lua)
│   ├── parser.lua          #   LPeg S-expression parser
│   ├── macros.lua          #   macro expansion
│   ├── backend_xt.lua      #   AST → Xt widget translation
│   ├── gui_xt.lua          #   high-level API: set / get / get_widget
│   ├── registry.lua        #   tag → WidgetClass mapping
│   └── test_harness.lua    #   test helpers: click / toggle / action / expect
├── LuaRunner/              # Lua↔Xt bridge (luaxt.c) + nanosvg submodule
├── demos/                  # LUI demo scripts loaded by commander_runner
├── tests/                  # C unit/layout tests + run_all_tests.sh + baselines
├── experimental/           # Experiments, sample apps (AdmPnl, todomgr), docs, legacy
│   ├── docs/               #   CONSOLIDATED documentation (start at index.md)
│   ├── legacy/             #   MOVED superseded docs
│   ├── AdmPnl/, todomgr/, test_file_sel/
│   └── lui_test/           #   LUI test scripts and results
├── build/                  # Generated artifacts (bin/, include/xtcw/, lib/, source/)
├── makefile                # master build script
├── AGENTS.md / CLAUDE.md   # GitNexus code-intelligence rules
└── overview.md             # this file
```

**Hand-written widget sources of note:** `wbuild_widgets/WpixBtn.c`,
`wbuild_widgets/Wlabel.c`, `wbuild_widgets/canvas-draw-cb.c` (copied into the build).

---

## 8. Key technologies in brief

- **wbuild** — reads `.widget` files with `@class`, `@public`, `@private`, `@methods`,
  `@utilities`, `@exports`, `@imports`, `@actions`, `@translations` sections.
  `$var` accesses instance state, `$old$var` reads the previous value in `set_values`,
  `$child$width` reaches into children, `#method(args)` calls the superclass method.
- **MLS** — handle-based lists/strings; see `experimental/docs/mls.md`.
- **re-tex** — node/glue layout and rendering; see `experimental/docs/retex.md`.
- **LUI** — S-expression UI syntax + `gui.get` / `gui.set` / `gui.get_widget`; see
  `experimental/docs/lui.md` and `commander/COMMANDER_RUNNER_TUTORIAL.md`.
- **wcl** — declarative creation from `.ad` resource files (`*WcChildren`, `*WcClass`).
- **Newer widgets**: `WfileSelector` (popup file dialog) and `SelectReq` (generic
  list-based selection dialog it uses).

---

## 9. Conventions & gotchas

1. **Consistent `CFLAGS`** — never mix `-DMLS_DEBUG` and non-debug objects; crashes result.
2. **`.widget` is the source of truth** — edits to `build/source/*.c` are overwritten;
   put TRACE instrumentation in the `.widget` file so it survives rebuilds.
3. **`@exports` needs its own includes** — types like `uint32_t` require
   `@incl <stdint.h>` inside `@exports`; putting it only in `@imports`/`@utilities`
   breaks consumers.
4. **Prefix exported functions** with the widget name (e.g. `dartboard_calculate_size`)
   to avoid linker/type collisions across the flat generated C namespace.
5. **Constraint resources**: `@constraints` has segfaulted wbuild; the reliable
   workaround is `@private-constraints` plus manual registration in `class_initialize`.
6. **Header mismatches**: if a widget complains about missing types, check that its
   `_types.h` was copied into `build/include/xtcw`.
7. **Callback timing**: `commander_runner` dispatches Lua callbacks via a polling timer,
   so tests should wait (~300 ms) after `xtaction` before asserting results.
8. **Command widget callbacks** require the chain `set → notify → unset`; calling
   `notify` alone does nothing.
9. **Graph-first editing** (GitNexus): run impact analysis before editing a symbol and
   change analysis before committing (see `AGENTS.md`).

---

## 10. Where to read more

Detailed, consolidated documentation lives under `experimental/docs/`:

| File | Contents |
|------|----------|
| [`experimental/docs/index.md`](experimental/docs/index.md) | Master index / navigation hub |
| [`experimental/docs/architecture.md`](experimental/docs/architecture.md) | Structure, build pipeline, wbuild, C host, TRACE |
| [`experimental/docs/widgets.md`](experimental/docs/widgets.md) | Widget class reference, LUI registry, known bugs |
| [`experimental/docs/lui.md`](experimental/docs/lui.md) | LUI framework, S-expressions, callbacks, C bindings |
| [`experimental/docs/mls.md`](experimental/docs/mls.md) | MLS lists, strings, debugging |
| [`experimental/docs/testing.md`](experimental/docs/testing.md) | Test plan / categories, infrastructure |
| [`experimental/docs/retex.md`](experimental/docs/retex.md) | Layout engine API |
| [`experimental/docs/xt-geometry.md`](experimental/docs/xt-geometry.md) | Xt geometry protocol |
| [`experimental/docs/gridbox.md`](experimental/docs/gridbox.md) | Gridbox layout widget |
| [`experimental/docs/widget-demos.md`](experimental/docs/widget-demos.md) | Pastable demo commands |
| [`commander/COMMANDER_RUNNER_TUTORIAL.md`](commander/COMMANDER_RUNNER_TUTORIAL.md) | Full commander_runner / LUI tutorial |

Historical documents superseded by the above are kept in `experimental/legacy/`.
