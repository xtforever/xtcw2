# Porting plan: `experimental/AdmPnl` → LUI launcher

> Status: revised planning draft  
> Source analyzed: `experimental/AdmPnl` (legacy), `experimental/adminpanel` (older sibling)  
> Target: LUI + `commander_runner`  
> Target app location: `apps/admpanel/`  
> Note: the user-facing name is “AdmPanel”; the actual legacy directory is `experimental/AdmPnl`.

This document incorporates the review decisions from §2. Keep adding thoughts under **Notes / decisions** at the end.

---

## 1. Goal

Port the useful parts of the legacy AdmPanel idea into the new LUI codebase:

1. A config-driven GUI launcher for a shell/bash script.
2. The GUI generates a form for the parameters the script needs.
3. Values from form fields and SQL lookups are stored in named variables.
4. Named variables are expanded into a command string.
5. When the running script asks for confirmation (`(Y/n?)`), the launcher shows a yes/no dialog and writes the answer back to the script.
6. SQL lookup fields behave like the legacy `Wsqlcombo.widget`: query a DB, show display values, store a key value.
7. Nice-to-have second step: read the parameter description directly from a machine-readable comment block inside the target shell script.
8. Follow-up: detect legacy `item list:` output and show a selection dialog.

### Non-goals for the first port

- Do not port `var5`, `svar2`, `mvar`, or the old variable engine.
- Do not port the WCL `.ad` generator (`njson-to-wcl.c`).
- Do not port unrelated experimental widgets (`Dartboard`, `Graph1`, `Ebox`, `Launcher.widget`) unless a concrete need appears.
- Do not keep an external xterm/FIFO terminal; output is embedded only.
- Do not change the legacy directories; build the new app independently.

---

## 2. Decisions locked in review

| Topic | Decision |
|---|---|
| App location | `apps/admpanel/` |
| Primary config format | Lua config file returning a table |
| Legacy `.json` config | Optional migration helper only, not the primary format |
| SQL support | Mandatory; link `libmysqlclient` |
| SQL credentials | `~/.my.cnf`, group `[adminpanel]`; no passwords in repo config |
| Output | Embedded log only; no “open terminal” action |
| Item-list selection | Follow-up, not part of the first port |
| Script-embedded description | Nice-to-have second step, after the config-driven launcher works |
| Script execution | Local process, pty-based, non-blocking; default `execvp`, optional `shell = true` |

### SQL credentials

Use a `~/.my.cnf` group named `adminpanel`:

```ini
[adminpanel]
host=cmdb.int.4hr.de
port=3306
database=copybox
user=copybox
password=<redacted>
```

The app must not copy this password into a repository config. It calls
`mysql_options(MYSQL_READ_DEFAULT_GROUP, "adminpanel")` (group configurable via
`sql.group`), then lets explicit `sql.host`/`sql.database`/`sql.user` values
override individual keys. `~/.my.cnf` should be mode `0600`.

---

## 3. What the legacy project actually does

| Legacy capability | Where it lives | What to preserve |
|---|---|---|
| Config → form | `experimental/AdmPnl/njson-to-wcl.c` | `title`, `script`, `fields[]`, `type`, `variable`, `values`, `extra_vars` |
| Field types | `admpanel.c` (`create_input_widget`, `create_option_widget`, `create_switch_widget`, `create_userid_widget`) | input, option/radio, switch, SQL userid/query |
| Variable expansion | `svexp_string`, `var5`/`svar2`/`mvar` | `$name`, values from fields, SQL row fields |
| Command execution | `exec_script_cb`, `Wexecgui.widget` | expanded command preview + execute button |
| Script process + prompts | `script_call.c`, `experimental/bash-request/subshell.c` | pty, line reading, `(Y/n?)` detection, answer to stdin |
| SQL lookup | `Wsqlcombo.widget` | `sql_query_str`, `sql_disp_field`, `sql_key_field`, `sql_result` |
| Item list | `script_call.c` (`parse_list`, `SelectReq`) | optional numbered-list selection |
| Example configs | `kunde-auswahl.json`, `freigabe.json`, `ap.ini` | behaviour examples for the port |

### Legacy config example

`experimental/AdmPnl/kunde-auswahl.json`:

```text
(
    title: 'Kundenauswahl',
    script: 'auswahl.sh $kk',
    fields: (
        ( type: 'kuerzel', title: 'Kuerzel', variable: 'kk',
          extra_vars: (
              sql_query_str: "select * from custdb",
              sql_disp_field: "$custid ($dbservid)",
              sql_key_field: "$custid" )
        )
   )
)
```

---

## 4. Assumptions

1. The canonical config format is a Lua module returning a table.
2. A legacy `.json` importer may be added later; it is not required for the first port.
3. MySQL is mandatory at build time (`libmysqlclient-dev`).
4. Credentials come from `~/.my.cnf` group `[adminpanel]`; no hardcoded defaults and no password logging.
5. Scripts are local commands. Default execution uses `execvp` with a tokenizer; `shell = true` is opt-in.
6. The first port targets Linux/X11 with the existing `commander_runner`.
7. `Y/n` is required. `Y/n/a` is supported via `WDialog`’s third button. Larger option sets are a later extension.

---

## 5. Target architecture

```text
apps/admpanel/
  main.lua          entry point: load config, build UI, wire callbacks
  config.lua        Lua config loader + normalizer
  vars.lua          named variable store, $var/${var}/$var[i], shell quoting
  fields.lua        config field -> LUI fragment; read field values back
  ui.lua            layout: header, form rows, command preview, log, actions
  script.lua        Lua wrapper around the C script-process binding
  prompt.lua        detect `(Y/n)?`, `(Y/n/a)?`, `item list:` + numbered items
  sql.lua           SQL lookup, display/key row expansion, combo population
  dialog.lua        yes/no/abort and choice dialogs
  scriptmeta.lua    NICE TO HAVE: parse `<launcher-begin>` block from a script
  compat/njson.lua  optional legacy config importer
  run.sh            wrapper: set config env, exec ../../run_lui.sh apps/admpanel/main.lua
  examples/
    kunde-auswahl.lua
    freigabe.lua
  README.md         schema, prompt protocol, SQL/security notes

commander/
  script_process.c/.h   forkpty-based non-blocking script runner + Lua binding
  db_mysql.c/.h         MySQL query binding + Lua registration (mandatory)
  commander_runner.c    register both bindings
  makefile              add objects and `-lmysqlclient`

tests/
  test_launcher_vars.lua
  test_launcher_config.lua
  test_launcher_prompt.lua
  test_launcher_scriptmeta.lua      NICE TO HAVE
  test_launcher_ui.lua
  test_launcher_script.sh
  fixtures/launcher_prompt.sh
  fixtures/launcher_described.sh    NICE TO HAVE
```

### Design decisions

- **No legacy variable engine.** `vars.lua` stores strings/arrays in a Lua table. `expand(template, scope)` supports `$name`, `${name}`, `$name[0]`, and `$$` for a literal `$`. Missing mandatory variables are reported before execute and block the run.
- **SQL rows are scopes.** For each row, expand `display` and `key` with the row merged over global vars. The selected key is written into the field’s `variable`.
- **The script runner is a C binding, not `io.popen`.** Use `forkpty` + `XtAppAddInput`, mirroring `experimental/bash-request/subshell.c`. This gives a real tty, non-blocking reads, and handles prompts without a trailing newline.
- **Prompt detection stays in Lua.** The C layer emits `line`, `partial`, `exit`, and `error` events; `prompt.lua` decides whether a line is a confirmation or item list.
- **Dialogs use existing `WDialog`.** Add a small reusable `gui.choice(message, opts)` helper in `lui/gui_xt.lua`, or implement the same logic in `apps/admpanel/dialog.lua` using `gui.build` + `gui.modal_loop` if core changes are unwanted. `WDialog` results: `1 = OK`, `0 = Cancel`, `2 = Aux`.

---

## 6. Canonical Lua config schema

```lua
return {
  title = "Kundenauswahl",
  script = "auswahl.sh $kk",
  -- shell = false,          -- optional: true -> /bin/sh -c
  -- cwd = ".",              -- optional: default = config directory
  fields = {
    { type = "input",  title = "Name",    variable = "name", default = "" },
    { type = "option", title = "Sandbox", variable = "sandbox",
      values = { 1, 2, 3, 4, 5, 6, 7 }, default = 1 },
    { type = "switch", title = "Force",   variable = "force",
      on = "-f", off = "" },
    { type = "sql",    title = "Kuerzel", variable = "kk",
      sql = {
        query   = "select * from custdb",
        display = "$custid ($dbservid)",
        key     = "$custid",
      } },
  },
  sql = {
    group = "adminpanel",    -- reads ~/.my.cnf
    -- host / port / database / user may override individual keys
  },
}
```

### Legacy → new mapping

| Legacy | New |
|---|---|
| `type: "input"` | `{ type = "input" }` |
| `type: "option"` + `values` | `{ type = "option", values = { ... } }` |
| `option: "a,b,c"` (ini style) | `values = { "a", "b", "c" }` |
| `switch: "-f"` / `switch: "on,off"` | `on` / `off` |
| `type: "kuerzel"` / `"query"` | `{ type = "sql" }` |
| `extra_vars.sql_query_str` | `field.sql.query` |
| `extra_vars.sql_disp_field` | `field.sql.display` |
| `extra_vars.sql_key_field` | `field.sql.key` |
| `extra_vars.sql_host/user/pwd/db` | `field.sql.*` or top-level `sql` |
| `title` | `title` |
| `script` | `script` |
| `init` / `initscript` | optional `init` command |

---

## 7. NICE TO HAVE: script-embedded parameter descriptions

After the config-driven launcher works, support a machine-readable block inside
the target shell script. The launcher then derives `title`, fields, SQL lookups
and the command line from that block.

### Example

```bash
#!/bin/bash
#<launcher-begin>
# title: Disable Customer via Ansible
# version: 1
# args:
#   -c --customer $custid (
#       type: string mandatory
#       default: ""
#       widget-type: sql-query
#       widget-label: Customer
#       sql_query_str: "select custid,dbservid from cust"
#       sql_disp_field: "$custid ($dbservid)"
#       sql_return_string: "$custid"
#   )
#   -l --logfile $logfile (
#       type: string mandatory
#       default: "/tmp/log"
#       widget-type: none
#   )
#   -v --verbose (
#       type: switch optional
#       widget-type: checkbox
#       widget-label: "Verbose"
#   )
#<launcher-end>
```

### Parsing rules

- The block starts at a line whose first non-space characters are `#<launcher-begin>` (also accept `# <launcher-begin>`), and ends at `#<launcher-end>`.
- Strip leading whitespace and one `#`; trim one optional following space.
- Top-level directives are `key: value` (`title`, `version`, `args`).
- An argument starts with one or more option tokens (`-c`, `--customer`), an optional `$variable`, then `(`. It ends at the matching `)`.
- Argument keys:
  - `type: string|switch` plus `mandatory|optional`
  - `widget-type: text|checkbox|dropdown|sql-query|none`
  - `default`, `widget-label`
  - SQL: `sql_query_str`, `sql_disp_field`, `sql_return_string`
- `widget-type: none` hides the field. If it is mandatory, it must have a `default`; otherwise it is a config error.
- `type: switch` with `widget-type: checkbox` maps to `WToggleSwitch`. Checked means the flag is appended to the command; unchecked means it is omitted.
- Normalize the block into the same internal field structure as §6, so the UI code is shared.

### Command building in script mode

- The script path is the target file.
- Iterate the declared arguments in order.
- Use `--long` if present, otherwise `-s`.
- String argument: append option + value (default used if the user leaves it empty).
- Hidden string argument: append option + default silently.
- Switch argument: append the option only when checked.
- Mandatory validation happens before execution; missing values keep the dialog/form open and show an inline error.
- Optional positional arguments are a later extension.

---

## 8. Implementation phases

### Phase 1 — Config, variables, prompt parsing (pure Lua)

Build `config.lua`, `vars.lua`, `prompt.lua`; keep `compat/njson.lua` optional.

Acceptance:

- Lua configs load and normalize correctly.
- `$kk`, `${srv}`, `$optlist[0]`, `$$` expand correctly.
- `(Y/n)?`, `(Y/n/a)?`, `( y / n ) ?` and a trailing partial line are detected.
- Unit tests pass under `lua5.3`.

### Phase 2 — Dynamic form UI and command preview

Build `ui.lua`, `fields.lua`, `main.lua`.

- Generate one LUI source string from the config.
- Use stable widget IDs (`fld_1`, `lbl_1`, ...).
- Define global callbacks before `lui.run(ui)`.
- Include header, form rows, read-only command preview, Execute/Quit, status line.
- On every field callback, update the variable store and re-render the preview.

Acceptance:

- `apps/admpanel/examples/kunde-auswahl.lua` renders all fields.
- Selecting a value updates the preview.
- The preview matches the expected `auswahl.sh $kk` expansion.
- No Lua callback or widget errors under Xvfb.

### Phase 3 — Script process binding and embedded log

Add `commander/script_process.c/.h`:

- `script_start(command, opts)` → job id; `opts = { args, shell, cwd, on_event }`
- `script_write(job, text)` → write to child stdin
- `script_stop(job)` → terminate
- `script_is_running(job)`

Implementation notes:

- `forkpty`, non-blocking master fd, `XtAppAddInput`.
- Buffer output; emit complete lines.
- Emit `partial` when the buffer ends with `?`.
- Emit `exit` with `WEXITSTATUS`/signal.
- Reap children with `waitpid`; poll with `xtapptimeout` if needed.
- Merge stdout/stderr through the pty; strip ANSI CSI sequences before display.
- Set a sensible pty window size (`TIOCSWINSZ`).
- `TERM=dumb` by default to simplify parsing, configurable.

LUI side:

- One-column `WListView` log (`showHeader 0`, `visibleRows` ~15).
- Append rows in batches via `xtapptimeout`; auto-scroll to the last row.
- While running: disable Execute, enable Abort; show status/exit code.

Acceptance:

- Fake script prints arguments, sleeps, and exits; output appears line-by-line.
- Exit code is correct; no zombie process; UI stays responsive.

### Phase 4 — Confirmation dialogs

Wire `script.lua` events into `prompt.lua` and `dialog.lua`.

- `(Y/n)?` → `WDialog` with OK = default answer, Cancel = the other.
- `(Y/n/a)?` → OK = default, Cancel = No, Aux = Abort.
- On answer: `script_write(job, answer .. "\n")`.
- Keep a `prompt_active` flag; queue a prompt if one is already open.
- Return follows the script’s uppercase default.
- Map `c/o` to Cancel/OK for now.

Acceptance:

- `tests/fixtures/launcher_prompt.sh` prints `Continue (Y/n)? `, reads the answer, prints it.
- The launcher opens a dialog, writes the chosen letter, and the log shows the expected answer.
- Test both `y` and `n`.

### Phase 5 — SQL lookup (mandatory)

Add `commander/db_mysql.c/.h` and register `db_query(opts, sql)`.

API:

- returns `{ columns = { ... }, rows = { { col = val, ... }, ... } }` or `nil, err`
- supports `group` (default `adminpanel`), `host`, `port`, `user`, `password`, `database`, `socket`, `connect_timeout`
- uses `mysql_options(MYSQL_READ_DEFAULT_GROUP, group)` when `group` is set
- explicit `host`/`database`/`user`/`password` override individual `~/.my.cnf` keys
- never logs the password

`apps/admpanel/sql.lua`:

- expand `sql.query` with global vars
- run query after UI realize (`xtapptimeout(0, ...)`) to avoid blocking startup
- build `items` = expanded `sql.display` per row
- keep `display -> key` and `display -> row` maps
- on `WComboBox` callback, set `variable` to the key and update the preview
- show query errors in the status line; keep old items
- cap displayed rows (e.g. 500)

Acceptance:

- With the `[adminpanel]` group available, the combo lists display values.
- Selecting one writes the key into the variable.
- The preview updates.
- With a missing group or unreachable DB, the field shows a clear error and does not crash.

### Phase 6 — NICE TO HAVE: script-embedded descriptions

Build `apps/admpanel/scriptmeta.lua` and the script-mode command builder.

Acceptance:

- `tests/fixtures/launcher_described.sh` contains a valid `<launcher-begin>` block.
- The parser produces the same internal field structure as a Lua config.
- The generated command matches the expected `--customer`, `--logfile`, `--verbose` invocation.
- Hidden mandatory argument with default works; hidden mandatory without default is rejected.
- Unknown keys produce a warning, not a crash.

### Phase 7 — Follow-up: item lists, polish, docs

- Detect `item list:` + numbered lines + question; show a choice dialog.
- Use `WDialog` `inputKind=4` for small lists; add a `WListView`-based dialog for larger lists if needed.
- Add a `Refresh` button for SQL fields.
- Add `apps/admpanel/run.sh` and `apps/admpanel/README.md`.
- Convert `kunde-auswahl.json` and `freigabe.json` to Lua examples.
- Add optional `init` command support (legacy `initscript`).

Acceptance:

- Legacy examples run unchanged or via the converter.
- Docs cover schema, prompt protocol, SQL credentials, and build/run commands.

---

## 9. Test plan

### Pure Lua unit tests

- `test_launcher_vars.lua`: `$x`, `${x}`, `$arr[0]`, `$$`, unknown variable, shell quoting.
- `test_launcher_config.lua`: Lua config normalization; field type mapping.
- `test_launcher_prompt.lua`: confirmation variants, partial line, item-list parsing, false positives.
- `test_launcher_scriptmeta.lua` (nice-to-have): block extraction, argument parsing, hidden fields, SQL keys, command building.

### Headless integration tests

- `test_launcher_ui.lua`: build UI from `kunde-auswahl`, set field values, assert command preview, assert widgets exist.
- `test_launcher_script.sh`: run under `xvfb-run`; fake script prints args, asks `Continue (Y/n)? `, reads answer; test driver auto-answers via `script_write`; assert log and exit code.
- `test_launcher_sql.sh`: optional; skip unless `MYSQL_TEST_GROUP` or a temporary `--defaults-file` is available. Unit-test `sql.lua` with an injected `query_fn` for row expansion and key mapping.

Add relevant tests to `tests/run_all_tests.sh` and/or `commander/run_demo_tests.sh`.

---

## 10. Risks and mitigations

| Risk | Mitigation |
|---|---|
| SQL query blocks the UI | Start synchronous with short timeouts; add async worker later if needed. |
| Mandatory `libmysqlclient` dependency | Document `libmysqlclient-dev` in `README.md` and the build prerequisites. |
| Missing `~/.my.cnf` group | Clear status error; never fall back to hardcoded credentials. |
| `forkpty`/Xt integration bugs | Port the proven `subshell.c`/`script_call.c` approach; test with fake scripts under Xvfb. |
| Prompt false positives | Only match `(...)?` at end of line/partial; allow config override patterns; ignore `item list:` content. |
| Shell injection via variable values | Default `execvp` tokenizer; `shell=true` applies `shquote` to substituted values. |
| Script block grammar ambiguity | Keep the block versioned and line-oriented; validate unknown keys; reject hidden mandatory fields without defaults. |
| LUI dynamic-generation limits | Generate a single source string, use stable IDs, define callbacks as globals before `lui.run`. |
| `WDialog` only has 3 buttons | Covers `Y/n` and `Y/n/a`; map `c/o` to Cancel/OK; extend later for arbitrary sets. |

---

## 11. Suggested first commit slice

Start with Phase 1 only:

- `apps/admpanel/config.lua`
- `apps/admpanel/vars.lua`
- `apps/admpanel/prompt.lua`
- `tests/test_launcher_config.lua`
- `tests/test_launcher_vars.lua`
- `tests/test_launcher_prompt.lua`

This gives a fast, reviewable foundation with no C or UI risk, and proves the config and prompt behaviour before touching `commander_runner`.

---

## 12. Notes / decisions

Review notes incorporated above:

- SQL credentials come from `~/.my.cnf` group `[adminpanel]`.
- The app is controlled by a Lua config file.
- NICE TO HAVE: in a second step, load the script description directly from the shell script.
- The script description is a machine-readable comment block; syntax is defined in §7 and still needs field validation.
- Mandatory parameters may be hidden (`widget-type: none`) and then require a default.
- Open questions resolved: SQL mandatory, Lua canonical, embedded output only, item-list selection follow-up, app in `apps/admpanel/`.

Add further thoughts below:

- 

---

## 13. Remaining open questions

- Should the script-embedded block use exactly the syntax in §7, or should it be YAML/TOML embedded in the comment block?
- Should script mode support positional arguments, or only option flags?
- Should `widget-type: dropdown` use a static `values` list in the block, and what is the exact key name?
- Should the app keep a small recent-config list, or always start from `ADMPANEL_CONFIG`?
