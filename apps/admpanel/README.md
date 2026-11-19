# AdmPanel launcher (LUI port)

A config-driven LUI launcher for shell scripts. It replaces the legacy
`experimental/AdmPnl` idea with Lua config, embedded output, pty-based script
execution and SQL lookup.

## Run

```bash
# Lua config
./apps/admpanel/run.sh apps/admpanel/examples/freigabe.lua

# Script with an embedded <launcher-begin> block
./apps/admpanel/run.sh tests/fixtures/launcher_described.sh
```

`run.sh` sets `ADMPANEL_CONFIG` or `ADMPANEL_SCRIPT` and starts
`run_lui.sh`. The runner changes into `commander/`, so the app uses
`../apps/admpanel/?.lua` for its modules.

### Guarded run (recommended while debugging)

A runaway popup/layout loop can allocate until the session OOM killer fires.
Use the guarded wrapper while clicking around:

```bash
./apps/admpanel/run_guarded.sh apps/admpanel/examples/freigabe.lua
./apps/admpanel/run_guarded.sh --mem 512 --timeout 300 --gdb \
    apps/admpanel/examples/freigabe.lua
```

It runs the app in a transient cgroup with a hard `MemoryMax` (default
1024 MiB), a wall-clock timeout (default 600 s), captures stdout/stderr and
prints the artifact directory (`run.log`, `meta.txt`, optional
`backtrace.txt`). If the process is killed at the cap the exit code is 137
and the log tail is shown. `--gdb` keeps a full backtrace on a fatal signal.
The generic runner is `tools/run_guarded.sh`:

```bash
./tools/run_guarded.sh --mem 256 --timeout 60 -- ./run_widget.sh \
    '(WComboBox :linesHeight 8)'
```

`tests/test_launcher_dropdown_guard.sh` automates the "open the combo
drop-down and quit" flow under the same guard.

## Lua config

```lua
return {
    title = 'Kundenauswahl',
    script = 'auswahl.sh $kk',
    fields = {
        { type = 'input',  title = 'Name', variable = 'name', default = '' },
        { type = 'option', title = 'Sandbox', variable = 'sandbox',
          values = { 1, 2, 3, 4, 5, 6, 7 }, default = 1 },
        { type = 'switch', title = 'Force', variable = 'force',
          on = '-f', off = '' },
        { type = 'sql', title = 'Kuerzel', variable = 'kk',
          sql = {
              query   = 'select * from custdb',
              display = '$custid ($dbservid)',
              key     = '$custid',
          } },
    },
    sql = { group = 'adminpanel' },
}
```

Field types:

- `input` — `WLineEdit`; value is the text.
- `option` — `WComboBox`; value is the selected string.
- `switch` — `WToggleSwitch`; value is `on` or `off`.
- `sql` — editable `WComboBoxEdit` populated from a query; typing filters the
  dropdown by prefix. The lookup value is the expanded `key` field via the
  widget's `selectedKey` (the display field is only shown in the edit/list).
  Free text is passed through as the key if it does not match an item. The
  lookup field supports normal editing: mouse-drag/Shift+arrow selection,
  `Ctrl+A`/`Ctrl+C`/`Ctrl+X`/`Ctrl+V`, Home/End, and a small x button (or
  `Ctrl+U`) to clear it and show the `Type to filter...` hint. After the
  second typed character the first matching entry is filled in with its suffix
  selected; typing replaces the suffix, Right/End accepts it, and
  Backspace/Delete/Escape remove it. After a lookup item is chosen (and for
  the initial value at startup) the whole value is selected, so the first
  typed character replaces it.

Variables support `$name`, `${name}`, `$arr[0]` (legacy 0-based arrays) and
`$$` for a literal dollar. Values are shell-quoted when building the command.

## SQL credentials

SQL is mandatory. Credentials are read from `~/.my.cnf` via the
`[adminpanel]` group:

```ini
[adminpanel]
host=cmdb.int.4hr.de
port=3306
database=copybox
user=copybox
password=<redacted>
```

Keep the file mode `0600`. Individual `host`/`database`/`user`/`password`
values in the Lua config override the group. Passwords are never logged.

Build dependency: `libmysqlclient-dev` (the commander links `-lmysqlclient`).

The shipped examples use the `[adminpanel]` group (`copybox` database, `cust`
table).  For the local `[client]` test database, override the group and
database:

```lua
sql = { group = 'client', database = 'custsrv' },
```

The real-DB test defaults to `[adminpanel]` / `copybox` / `cust`.  For the
local test DB:

```bash
ADMPANEL_TEST_GROUP=client ADMPANEL_TEST_DB=custsrv \
ADMPANEL_TEST_TABLE=custdb ADMPANEL_TEST_EXPECT_FIRST=4kstaffer \
  ./tests/test_launcher_sql_real.sh
```

## Script-embedded description

A shell script can describe its arguments in a comment block:

```bash
#!/bin/bash
#<launcher-begin>
# title: Disable Customer
# args:
#   -c --customer $custid (
#       type: string mandatory
#       default: ""
#       widget-type: sql-query
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

The launcher builds `script --customer <key> --logfile /tmp/log --verbose`
from the argument definitions. `widget-type: none` hides a mandatory argument;
it must have a default. `widget-type: checkbox` maps to a toggle switch.

## Interactive prompts

The script runner uses a pty and streams output into the embedded log.
Confirmation prompts are detected in Lua:

- `Continue (Y/n)?` → Yes/No dialog; Return follows the uppercase default.
- `Delete? (y/N)` → the question mark may be in the question text.
- `Continue (Y/n/a)?` → Yes/No/Abort (the third `WDialog` button).
- A prompt without a trailing newline is emitted as a `partial` event.

The answer is written back to the child's stdin.

## Tests

Pure Lua:

```bash
lua5.3 tests/test_launcher_vars.lua
lua5.3 tests/test_launcher_prompt.lua
lua5.3 tests/test_launcher_config.lua
lua5.3 tests/test_launcher_sql.lua
lua5.3 tests/test_launcher_scriptmeta.lua
```

Headless Xvfb tests:

```bash
./tests/test_launcher_ui.sh
./tests/test_launcher_script.sh
./tests/test_launcher_prompt_flow.sh
./tests/test_launcher_dialog.sh
./tests/test_launcher_sql_ui.sh
./tests/test_launcher_db.sh
./tests/test_launcher_sql_real.sh   # real DB via ~/.my.cnf (skips if absent)
./tests/test_launcher_described.sh
./tests/test_launcher_required.sh
```

## Current limitations

- SQL queries are synchronous; a slow query blocks the UI.
- The real-DB test uses `~/.my.cnf` group `[adminpanel]`, database `copybox`
  and table `cust` by default; override with `ADMPANEL_TEST_GROUP`,
  `ADMPANEL_TEST_DB`, `ADMPANEL_TEST_TABLE` and the `ADMPANEL_TEST_EXPECT_*`
  variables for a different test database.
- Item-list selection (`item list:` output) is parsed but not yet wired to a
  dialog. This is the follow-up from the porting plan.
- The command preview is a single-line `WLineEdit`; long commands scroll
  horizontally instead of wrapping.
- No external terminal action; output stays in the launcher window.
