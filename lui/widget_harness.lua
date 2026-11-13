-- Single-widget probe harness.
--
-- Builds a 700x600 window with a 2-row x 1-column Gridbox:
--
--   row 0: widget_host, fills all available space, contains the probe
--   row 1: Quit button, natural size, anchored bottom-right
--
-- The probe is supplied through LUI_WIDGET (inline LUI) or LUI_WIDGET_FILE.
-- Optional hooks:
--   LUI_WIDGET_TITLE      window title (default: "Widget Probe")
--   LUI_WIDGET_SETUP      Lua file executed before the probe is built
--   LUI_WIDGET_AFTER      Lua file executed after the probe is built
--   LUI_WIDGET_AUTOCLOSE  milliseconds until xtappexitflag()
--   LUI_WIDGET_RESULT     write probe/geometry data as key=value lines
--
-- This file is normally started through run_widget.sh. It can also be run
-- directly with commander_runner for tests:
--   LUI_WIDGET='(Wlabel :label "Hi")' \
--     commander_runner -Luafile ../lui/widget_harness.lua

local parser = require('parser')
local macros = require('macros')
local backend = require('backend_xt')
local gui = require('gui_xt')

local WINDOW_WIDTH = 700
local WINDOW_HEIGHT = 600

local widget_source = os.getenv('LUI_WIDGET')
local widget_file = os.getenv('LUI_WIDGET_FILE')
local setup_file = os.getenv('LUI_WIDGET_SETUP')
local after_file = os.getenv('LUI_WIDGET_AFTER')
local result_file = os.getenv('LUI_WIDGET_RESULT')
local autoclose_ms = tonumber(os.getenv('LUI_WIDGET_AUTOCLOSE') or '')
local title = os.getenv('LUI_WIDGET_TITLE')
if not title or title == '' then title = 'Widget Probe' end

local function fail(msg)
    io.stderr:write('widget_harness: ' .. tostring(msg) .. '\n')
    os.exit(2)
end

local function read_file(path)
    local f, err = io.open(path, 'rb')
    if not f then
        fail('cannot read ' .. tostring(path) .. ': ' .. tostring(err))
    end
    local s = f:read('*a')
    f:close()
    return s
end

local function run_lua_file(path, label)
    if not path or path == '' then return end
    local ok, err = pcall(dofile, path)
    if not ok then
        fail(label .. ' failed: ' .. tostring(err))
    end
end

if (not widget_source or widget_source == '') and
   (not widget_file or widget_file == '') then
    fail('missing widget fragment (set LUI_WIDGET or LUI_WIDGET_FILE)')
end

local source
if widget_file and widget_file ~= '' then
    source = read_file(widget_file)
else
    source = widget_source
end
if not source or source:match('^%s*$') then
    fail('widget fragment is empty')
end

-- Let the caller define callback globals before the widget is created.
run_lua_file(setup_file, 'setup file')

local ast = parser.parse(source)
if not ast then
    fail('could not parse widget fragment')
end

local expanded = macros.expand(ast)
local nodes = {}
for _, node in ipairs(expanded) do
    if type(node) == 'table' and type(node[1]) == 'string' then
        nodes[#nodes + 1] = node
    end
end

if #nodes == 0 then
    fail('widget fragment contains no widget')
end
if #nodes > 1 then
    fail('widget fragment must contain exactly one widget (found ' ..
         tostring(#nodes) .. ')')
end

local probe = nodes[1]
local probe_class = probe[1]
if probe_class == 'window' or
   probe_class == 'topLevelShellWidgetClass' or
   probe_class == 'applicationShellWidgetClass' then
    fail('probe must be a single widget, not a window/shell')
end

local function find_prop(node, key)
    for i = 2, #node do
        local item = node[i]
        if type(item) == 'table' and item.keyword == key then
            return true, node[i + 1]
        end
    end
    return false, nil
end

local function add_prop(node, key, value)
    if not find_prop(node, key) then
        table.insert(node, { keyword = key })
        table.insert(node, value)
    end
end

-- The host owns the layout. A probe that does not specify its own grid
-- placement gets the full host cell.
add_prop(probe, ':id', 'probe_widget')
add_prop(probe, ':gridx', 0)
add_prop(probe, ':gridy', 0)
add_prop(probe, ':weightx', 1)
add_prop(probe, ':weighty', 1)
add_prop(probe, ':fill', 3)

local _, probe_id = find_prop(probe, ':id')
if type(probe_id) ~= 'string' or probe_id == '' then
    probe_id = 'probe_widget'
end

local window_node = {
    'window',
    { keyword = ':id' }, 'harness_window',
    { keyword = ':title' }, title,
    { keyword = ':width' }, WINDOW_WIDTH,
    { keyword = ':height' }, WINDOW_HEIGHT,
    { keyword = ':defaultDistance' }, 0,
    { keyword = ':borderWidth' }, 0,
    {
        'grid',
        { keyword = ':id' }, 'harness_grid',
        { keyword = ':weightx' }, 1,
        { keyword = ':weighty' }, 1,
        { keyword = ':defaultDistance' }, 0,
        { keyword = ':borderWidth' }, 0,
        {
            'grid',
            { keyword = ':id' }, 'widget_host',
            { keyword = ':gridx' }, 0,
            { keyword = ':gridy' }, 0,
            { keyword = ':weightx' }, 1,
            { keyword = ':weighty' }, 1,
            { keyword = ':fill' }, 3,
            { keyword = ':defaultDistance' }, 0,
            { keyword = ':borderWidth' }, 0,
            probe,
        },
        {
            'button',
            { keyword = ':id' }, 'harness_quit',
            { keyword = ':label' }, 'Quit',
            { keyword = ':gridx' }, 0,
            { keyword = ':gridy' }, 1,
            { keyword = ':weightx' }, 1,
            { keyword = ':weighty' }, 0,
            { keyword = ':fill' }, 'none',
            { keyword = ':gravity' }, 'SouthEast',
            { keyword = ':margin' }, 8,
            { keyword = ':callback' }, 'LUA(harness_quit)',
        },
    },
}

_G.harness_quit = function()
    xtappexitflag()
end

local root = backend.build(window_node)
if not root then
    fail('failed to build harness window')
end
gui.manage('harness_window')

-- Let the caller populate models or inspect widgets after creation.
run_lua_file(after_file, 'after file')

local function write_result()
    if not result_file or result_file == '' then return end

    local function geometry(id)
        return gui.geometry(id) or {}
    end

    local win = geometry('harness_window')
    local grid = geometry('harness_grid')
    local host = geometry('widget_host')
    local quit = geometry('harness_quit')
    local probe_geom = geometry(probe_id)

    local f, err = io.open(result_file, 'wb')
    if not f then
        io.stderr:write('widget_harness: cannot write result file ' ..
                        tostring(result_file) .. ': ' .. tostring(err) .. '\n')
        return
    end

    local lines = {
        'window_width=' .. tostring(win.width or 0),
        'window_height=' .. tostring(win.height or 0),
        'grid_width=' .. tostring(grid.width or 0),
        'grid_height=' .. tostring(grid.height or 0),
        'host_width=' .. tostring(host.width or 0),
        'host_height=' .. tostring(host.height or 0),
        'quit_x=' .. tostring(quit.x or 0),
        'quit_y=' .. tostring(quit.y or 0),
        'quit_width=' .. tostring(quit.width or 0),
        'quit_height=' .. tostring(quit.height or 0),
        'probe_x=' .. tostring(probe_geom.x or 0),
        'probe_y=' .. tostring(probe_geom.y or 0),
        'probe_width=' .. tostring(probe_geom.width or 0),
        'probe_height=' .. tostring(probe_geom.height or 0),
        'probe_id=' .. probe_id,
        'probe_class=' .. probe_class,
    }
    f:write(table.concat(lines, '\n') .. '\n')
    f:close()
end

if result_file and result_file ~= '' then
    -- Let the first geometry pass complete before sampling.
    xtapptimeout(200, write_result)
end

if autoclose_ms and autoclose_ms > 0 then
    xtapptimeout(autoclose_ms, function()
        xtappexitflag()
    end)
end

io.stderr:write(string.format(
    'widget_harness: %s in %dx%d; Quit to exit\n',
    probe_class, WINDOW_WIDTH, WINDOW_HEIGHT))
