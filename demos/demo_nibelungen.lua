-- Render the Nibelungenlied text file (nb.txt) with Wmarkdown.
--
-- The full file has ~17k lines.  re-tex uses 32-bit fixed-point dimensions,
-- so one paragraph taller than ~32768pt cannot be represented reliably.
-- This demo therefore pages through 50-line windows; the Wmarkdown layout
-- cache makes scrolling inside a page cheap.  nb.txt is hard-wrapped prose,
-- so lines are reflowed with spaces (preserveLineBreaks is intentionally off).

local gui = require('gui_xt')
local lui = require('lui')

-- The runner's TRACE(2) node logging makes a 300-line page several times
-- slower and can flood the terminal; the viewer does not need it.
set_trace_level(0)

local PAGE_LINES = 50

-- run_lui.sh starts the runner in commander/, so resolve nb.txt relative to
-- this script rather than the current directory.
local src = debug.getinfo(1, 'S').source:sub(2)
if src:sub(1, 1) ~= '/' then src = '/' .. src end
local repo = src:match('^(.*)/demos/[^/]+$') or '.'
local f = io.open(repo .. '/nb.txt', 'rb')
if not f then f = io.open('nb.txt', 'rb') end
local text = f and f:read('*a') or ''
if f then f:close() end
if #text == 0 then
    text = "nb.txt not found. Put the Nibelungenlied text file in the repository root."
end

local lines = {}
for line in (text .. '\n'):gmatch('(.-)\n') do
    lines[#lines + 1] = line
end
local nlines = #lines
local page = 0
local npages = math.max(1, math.ceil(nlines / PAGE_LINES))
local first_chunk = table.concat(lines, '\n', 1,
                                 math.min(nlines, PAGE_LINES))
function show_page()
    local first = page * PAGE_LINES + 1
    local last = math.min(nlines, first + PAGE_LINES - 1)
    local chunk = table.concat(lines, '\n', first, last)
    gui.set('hdr', 'label', string.format(
        'Nibelungenlied - lines %d-%d of %d  (page %d/%d)',
        first, last, nlines, page + 1, npages))
    xtaction(gui.get_widget('text'), 'set_text', chunk)
end

function prev_page()
    if page > 0 then
        page = page - 1
        show_page()
    end
end

function next_page()
    if page + 1 < npages then
        page = page + 1
        show_page()
    end
end

local ui = [[
(window :id "win" :title "Nibelungenlied - nb.txt" :width 980 :height 760
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr"
           :label "Loading nb.txt ..."
           :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 1)
    (Wmarkdown :id "text"
               :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3
               :width 907
               :useThemeFont 0
               :fontFace "Noto Serif"           ; EB Garamond
               :fontSize 8
               :alignment 0
               :text INITIAL_TEXT)
    (button :id "prev" :label "Previous"
            :gridx 0 :gridy 2 :weightx 0 :weighty 0
            :callback "LUA(prev_page)")
    (button :id "next" :label "Next"
            :gridx 1 :gridy 2 :weightx 0 :weighty 0
            :callback "LUA(next_page)")))
]]

ui = ui:gsub('INITIAL_TEXT', string.format('%q', first_chunk))

local ok, err = lui.run(ui)
if not ok then error("demo_nibelungen: " .. tostring(err)) end

-- Let the window map first, then load the first page.
xtapptimeout(100, show_page)
