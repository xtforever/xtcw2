-- Unit tests for apps/admpanel/scriptmeta.lua.
package.path = 'apps/admpanel/?.lua;' .. package.path
local scriptmeta = require('scriptmeta')
local config = require('config')

local function fail(msg)
    io.stderr:write('FAIL: ' .. msg .. '\n')
    os.exit(1)
end
local function eq(actual, expected, msg)
    if actual ~= expected then
        fail((msg or 'value') .. ': expected ' .. tostring(expected) ..
             ', got ' .. tostring(actual))
    end
end
local function ok(cond, msg)
    if not cond then fail(msg or 'condition failed') end
end

local block = [[
title: Described Demo
version: 1
args:
  -c --customer $custid (
      type: string mandatory
      default: ""
      widget-type: sql-query
      widget-label: Customer
      sql_query_str: "select custid,dbservid from cust"
      sql_disp_field: "$custid ($dbservid)"
      sql_return_string: "$custid"
  )
  -l --logfile $logfile (
      type: string mandatory
      default: "/tmp/log"
      widget-type: none
  )
  -v --verbose (
      type: switch optional
      widget-type: checkbox
      widget-label: "Verbose"
  )
]]

local raw, err = scriptmeta.parse_block(block, '/tmp/demo.sh')
ok(raw, err)
eq(raw.title, 'Described Demo', 'title')
eq(raw.command_mode, 'args', 'command mode')
eq(#raw.fields, 3, 'field count')

local cfg, nerr = config.normalize(raw)
ok(cfg, nerr)
eq(cfg.command_mode, 'args', 'normalized command mode')
eq(cfg.fields[1].type, 'sql', 'sql type')
eq(cfg.fields[1].variable, 'custid', 'sql variable')
eq(cfg.fields[1].short, '-c', 'sql short')
eq(cfg.fields[1].long, '--customer', 'sql long')
eq(cfg.fields[1].mandatory, true, 'sql mandatory')
eq(cfg.fields[1].sql.query, 'select custid,dbservid from cust', 'sql query')
eq(cfg.fields[1].sql.display, '$custid ($dbservid)', 'sql display')
eq(cfg.fields[1].sql.key, '$custid', 'sql key')

eq(cfg.fields[2].hidden, true, 'hidden field')
eq(cfg.fields[2].default, '/tmp/log', 'hidden default')
eq(cfg.fields[2].long, '--logfile', 'hidden long')

eq(cfg.fields[3].type, 'switch', 'switch type')
eq(cfg.fields[3].widget_type, 'checkbox', 'switch widget')
eq(cfg.fields[3].variable, 'verbose', 'switch variable')
eq(cfg.fields[3].on, '--verbose', 'switch on')
eq(cfg.fields[3].title, 'Verbose', 'switch label')

local command = scriptmeta.build_command(cfg, {
    custid = 'jens',
    logfile = '/tmp/log',
    verbose = '--verbose',
})
eq(command, '/tmp/demo.sh --customer jens --logfile /tmp/log --verbose', 'command')

command = scriptmeta.build_command(cfg, {
    custid = 'jens',
    logfile = '/tmp/log',
    verbose = '',
})
eq(command, '/tmp/demo.sh --customer jens --logfile /tmp/log', 'command without switch')

-- Extraction from a full script
local full = '#!/bin/bash\n#<launcher-begin>\n# title: T\n# args:\n#   -x --x $x (\n#     type: string optional\n#     widget-type: text\n#   )\n#<launcher-end>\necho hi\n'
local extracted, eerr = scriptmeta.extract(full)
ok(extracted, eerr)
ok(extracted:find('title: T'), 'extracted title')
ok(extracted:find('%-x %-%-x'), 'extracted arg')

-- Missing block
local none, nerr2 = scriptmeta.extract('#!/bin/bash\necho hi\n')
ok(not none, 'missing block rejected')
ok(nerr2 and nerr2:find('launcher%-begin'), 'missing block error')

-- Hidden field without default
local bad, berr = scriptmeta.parse_block([[
args:
  -l --logfile $logfile (
      type: string mandatory
      widget-type: none
  )
]], '/tmp/x.sh')
ok(not bad, 'hidden without default rejected')
ok(berr and berr:find('default'), 'hidden error text')

print('test_launcher_scriptmeta: OK')
