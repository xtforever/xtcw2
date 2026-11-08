-- Demo: WtabWidget — a tab group whose page area is a native Wviewport.
--
-- Each child with a `:tab-title` becomes a tab; clicking a tab title switches
-- the page shown in the scrollable content area.  The "Long list" page is
-- taller than the content area, so the Wviewport shows a scrollbar.
local gui = require('gui_xt')
local lui = require('lui')

local rows = {}
for r = 0, 24 do
    rows[#rows + 1] = string.format(
        '(label :label "Row %02d %s" :gridx 0 :gridy %d :weightx 1 :weighty 0 :fill 3)',
        r, string.rep("-", 60), r)
end

local ui = [[
(window :id "w" :title "WtabWidget Demo" :width 460 :height 320
  (tabs :id "tabs" :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_tab)"
    (grid :id "long" :tab-title "Long list"
      ]] .. table.concat(rows, "\n      ") .. [[
    )
    (grid :id "short" :tab-title "Short"
      (label :label "This tab is short; no scrollbars are needed."
             :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)
      (label :label "Click a tab title to switch pages."
             :gridx 0 :gridy 1 :weightx 1 :weighty 0 :fill 3)
    )))
]]

function on_tab() end

local ok, err = lui.run(ui)
if not ok then error("demo_wtabwidget: " .. tostring(err)) end
