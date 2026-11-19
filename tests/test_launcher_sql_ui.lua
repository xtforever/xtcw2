-- Headless probe for SQL combo population and preview updates.
package.path = (os.getenv('ADMPANEL_LUA_PATH') or '../apps/admpanel/?.lua')
    .. ';' .. package.path

local gui = require('gui_xt')
local sql = require('sql')

if os.getenv('ADMPANEL_SQL_FAIL') == '1' then
    sql.set_query_fn(function() return nil, 'access denied' end)
else
    sql.set_query_fn(function(opts, query)
        return {
            columns = { 'custid', 'dbservid' },
            rows = {
                { custid = 'jens', dbservid = 'db1' },
                { custid = 'anna', dbservid = 'db2' },
            },
        }
    end)
end

local config = require('config')
local ui = require('ui')

local out_path = os.getenv('ADMPANEL_TEST_OUT') or '/tmp/admpanel_sql_ui.txt'
local cfg, err = config.normalize({
    title = 'sql ui',
    script = 'echo $kk',
    fields = {
        {
            type = 'sql',
            title = 'Customer',
            variable = 'kk',
            sql = {
                query = 'select custid,dbservid from cust',
                display = '$custid ($dbservid)',
                key = '$custid',
            },
        },
    },
})
if not cfg then error('config: ' .. tostring(err)) end

ui.init(cfg)
function on_field_changed(id) ui.on_field_changed(id) end
function on_execute() ui.on_execute() end
function on_abort() ui.on_abort() end
function on_script_event(ev) ui.on_script_event(ev) end
function on_quit() xtappexitflag() end
ui.run()

xtapptimeout(800, function()
    local f = assert(io.open(out_path, 'w'))
    f:write('selected=' .. tostring(gui.get('fld_1', 'selected')) .. '\n')
    f:write('label=' .. tostring(gui.get('fld_1', 'label')) .. '\n')
    f:write('selected_key=' .. tostring(gui.get('fld_1', 'selectedKey')) .. '\n')
    f:write('value=' .. tostring(ui.values.kk) .. '\n')
    f:write('preview=' .. tostring(gui.get('cmd_preview', 'label')) .. '\n')
    f:write('status=' .. tostring(gui.get('status', 'label')) .. '\n')

    -- Editable lookup: free text and display strings resolve to the SQL key.
    gui.set('fld_1', 'selected', -1)
    gui.set('fld_1', 'text', 'anna')
    on_field_changed('fld_1')
    f:write('typed_key=' .. tostring(ui.values.kk) .. '\n')
    f:write('typed_selected_key=' .. tostring(gui.get('fld_1', 'selectedKey')) .. '\n')
    gui.set('fld_1', 'text', 'anna (db2)')
    on_field_changed('fld_1')
    f:write('typed_display=' .. tostring(ui.values.kk) .. '\n')
    f:write('typed_display_selected_key=' .. tostring(gui.get('fld_1', 'selectedKey')) .. '\n')
    gui.set('fld_1', 'text', 'custom')
    on_field_changed('fld_1')
    f:write('typed_free=' .. tostring(ui.values.kk) .. '\n')
    f:write('typed_preview=' .. tostring(gui.get('cmd_preview', 'label')) .. '\n')

    for i, line in ipairs(ui.get_log()) do
        f:write('log' .. tostring(i) .. '=' .. tostring(line) .. '\n')
    end
    f:close()
    xtappexitflag()
end)
