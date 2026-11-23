-- t1_menu.lua — xtcw demo launcher menu.
--
-- Run by commander_runner from the commander/ directory:
--   T1_MENU_INPUT=... T1_MENU_RESULT=... T1_ROOT=... \
--       ./commander_runner -Luafile ../tools/t1_menu.lua
--
-- Input:  T1_MENU_INPUT   TSV: key<TAB>title<TAB>short description
-- Output: T1_MENU_RESULT  TSV: key<TAB>trace   (empty/absent = cancel)
--         T1_ROOT         repository root (for demos/desc/<key>.retex)
--
-- Test hooks:
--   T1_MENU_INITIAL=<key>     initial selected demo
--   T1_MENU_AUTOSELECT=<key>  select and run that demo after startup
--   T1_MENU_AUTOTRACE=<n>     select that trace level
--   T1_MENU_LATE_SELECT=<n>   move down n rows, write the description, exit
--   T1_MENU_DESC_OUT=<path>   write the resolved description body
--   T1_MENU_LAYOUT_OUT=<path> write the splitter fraction (layout check)

local gui = require('gui_xt')
local lui = require('lui')
local rf = require('retex_file')

local input_path = os.getenv('T1_MENU_INPUT')
local result_path = os.getenv('T1_MENU_RESULT')
local root = os.getenv('T1_ROOT') or '.'
local desc_out = os.getenv('T1_MENU_DESC_OUT')

if not input_path or input_path == '' then
    io.stderr:write('t1_menu: T1_MENU_INPUT is not set\n')
    os.exit(2)
end
if not result_path or result_path == '' then
    io.stderr:write('t1_menu: T1_MENU_RESULT is not set\n')
    os.exit(2)
end

local demos = {}
local rows = {}
local trace_values = { 0, 2, 5, 50 }
local trace = 0
local current_desc = ''

local f, err = io.open(input_path, 'rb')
if not f then
    io.stderr:write('t1_menu: cannot read ' .. input_path .. ': ' .. tostring(err) .. '\n')
    os.exit(2)
end
for line in f:lines() do
    local key, title, desc = line:match('^([^\t]*)\t([^\t]*)\t(.*)$')
    if key and key ~= '' then
        demos[#demos + 1] = { key = key, title = title, desc = desc }
        rows[#rows + 1] = key .. '\t' .. title .. ' - ' .. desc
    end
end
f:close()

if #demos == 0 then
    io.stderr:write('t1_menu: no demos in ' .. input_path .. '\n')
    os.exit(2)
end

local function write_result(key, level)
    local out = io.open(result_path, 'wb')
    if out then
        out:write(key .. '\t' .. tostring(level) .. '\n')
        out:close()
    end
    if desc_out and desc_out ~= '' then
        local d = io.open(desc_out, 'wb')
        if d then
            -- Read back the widget resource so the test also covers the
            -- gui.set() path, not just the string we computed.
            local shown = gui.get('desc', 'text')
            if type(shown) ~= 'string' then shown = current_desc end
            d:write(shown or '')
            d:close()
        end
    end
end

-- Load demos/desc/<key>.retex and resolve \includesvg paths against the
-- file's directory.  Fall back to the short registry description.
local function load_description(demo)
    local path = root .. '/demos/desc/' .. demo.key .. '.retex'
    local text, base = rf.load(path)
    if text then
        return rf.resolve_includes(text, base)
    end
    return demo.desc or ''
end

local function select_demo(i)
    i = tonumber(i)
    local demo = i and demos[i + 1]
    if not demo then return end
    current_desc = load_description(demo)
    gui.set('desc', 'text', current_desc)
end

local function run_demo(i)
    i = tonumber(i)
    local demo = i and demos[i + 1]
    if not demo then return end
    write_result(demo.key, trace)
    xtappexitflag()
end

function _G.on_select(_)
    select_demo(gui.get('lv', 'selected'))
end

function _G.on_activate(_)
    run_demo(gui.get('lv', 'selected'))
end

function _G.on_trace(_)
    local i = tonumber(gui.get('trace_lv', 'selected')) or 0
    trace = trace_values[i + 1] or 0
end

function _G.on_run()
    local i = gui.get('lv', 'selected')
    if type(i) == 'number' and i >= 0 then
        run_demo(i)
    end
end

function _G.on_quit()
    xtappexitflag()
end

local ui = [[
(window :id "win" :title "XTCW Demo Launcher" :width 900 :height 680
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr"
           :label "Select a demo, choose a trace level, then Run (Return activates)."
           :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (splitter :id "pane_split" :orientation "vertical" :fraction 0.72
       :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 5 :fill 3
      (listview :id "lv"
         :columnCount 2
         :columnTitles "Demo\tDescription"
         :columnWidths "150,0"
         :columnAligns "0,0"
         :columnKinds "0,0"
         :showHeader 1
         :gridLines 1
         :alternateRows 1
         :visibleRows 10
         :on-row-activated "LUA(on_select)"
         :on-activate "LUA(on_activate)")
      (listview :id "trace_lv"
         :columnCount 1
         :columnTitles "Trace level"
         :columnWidths "0"
         :showHeader 1
         :gridLines 0
         :visibleRows 4
         :on-row-activated "LUA(on_trace)"
         :on-activate "LUA(on_trace)"))
    (retex :id "desc"
       :gridx 0 :gridy 2 :gridWidth 2 :weightx 1 :weighty 1 :fill 3
       :alignment 0 :fontSize 13 :autoHeight FALSE
       :height 160)
    (button :id "run" :label "Run" :alignment 1
       :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3
       :callback "LUA(on_run)")
    (button :id "quit" :label "Quit" :alignment 1
       :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3
       :callback "LUA(on_quit)")
  ))
]]

local ok, lerr = lui.run(ui)
if not ok then
    io.stderr:write('t1_menu: ' .. tostring(lerr) .. '\n')
    os.exit(1)
end

gui.set_string_list('lv', 'rows', rows)
gui.set_string_list('trace_lv', 'rows', {
    '0 - no trace output',
    '2 - high-level flow',
    '5 - detailed',
    '50 - widget-level TRACE',
})
gui.set('trace_lv', 'selected', 0)
_G.on_trace(0)

local initial_i = 0
local initial = os.getenv('T1_MENU_INITIAL')
if initial and initial ~= '' then
    for i, demo in ipairs(demos) do
        if demo.key == initial then
            initial_i = i - 1
            break
        end
    end
end
gui.set('lv', 'selected', initial_i)
select_demo(initial_i)

local layout_out = os.getenv('T1_MENU_LAYOUT_OUT')
if layout_out and layout_out ~= '' then
    local f = io.open(layout_out, 'wb')
    if f then
        f:write(tostring(gui.get('pane_split', 'fraction') or '') .. '\n')
        f:close()
    end
end

local late = tonumber(os.getenv('T1_MENU_LATE_SELECT') or '')
if late then
    xtapptimeout(1000, function()
        local lv = gui.get_widget('lv')
        for _ = 1, late do xtaction(lv, 'move_down') end
        if desc_out and desc_out ~= '' then
            local shown = gui.get('desc', 'text')
            local f = io.open(desc_out, 'wb')
            if f then f:write(shown or ''); f:close() end
        end
        xtappexitflag()
    end)
end

local autoselect = os.getenv('T1_MENU_AUTOSELECT')
if autoselect and autoselect ~= '' then
    local autotrace = tonumber(os.getenv('T1_MENU_AUTOTRACE') or '')
    xtapptimeout(500, function()
        if autotrace then
            for i, v in ipairs(trace_values) do
                if v == autotrace then
                    gui.set('trace_lv', 'selected', i - 1)
                    _G.on_trace(i - 1)
                    break
                end
            end
        end
        for i, demo in ipairs(demos) do
            if demo.key == autoselect then
                gui.set('lv', 'selected', i - 1)
                select_demo(i - 1)
                run_demo(i - 1)
                return
            end
        end
        io.stderr:write('t1_menu: autoselect key not found: ' .. autoselect .. '\n')
        xtappexitflag()
    end)
end
