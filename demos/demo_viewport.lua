-- Demo: a scrollable view of child widgets using the native Wviewport.
--
-- (viewport ...) is the native wbuild Wviewport widget: it scrolls a single
-- child when the window is smaller than that child.  It provides a vertical
-- scrollbar on the right (useRight) and a horizontal scrollbar at the bottom
-- (useBottom); allowVert/allowHoriz enable them.
--
-- The viewport holds EXACTLY ONE child widget, so all content is wrapped in a
-- single grid whose natural size exceeds the window, which makes both
-- scrollbars appear.
local gui = require('gui_xt')
local lui = require('lui')

local rows, cols = 14, 5
local cells = {}
for r = 0, rows - 1 do
    for c = 0, cols - 1 do
        cells[#cells + 1] = string.format(
            '(button :label "Cell %d,%d" :gridx %d :gridy %d '
            .. ':weightx 1 :weighty 0 :fill 3 :callback "LUA(on_cell)")',
            r + 1, c + 1, c, r + 1)
    end
end

local ui = [[
(window :id "win" :title "Viewport (scrollable view) Demo" :width 420 :height 320
  (viewport :id "vp" :weightx 1 :weighty 1 :fill 3
            :allowVert 1 :allowHoriz 1 :useRight 1 :useBottom 1
    (grid :id "content"
      (label :id "hdr" :label "Scrollable view: drag the bars to reach the other cells"
             :gridx 0 :gridy 0 :gridWidth ]] .. cols .. [[
             :weightx 1 :weighty 0 :fill 3)
      (label :id "status" :label "Clicks: 0"
             :gridx 0 :gridy 1 :gridWidth ]] .. cols .. [[
             :weightx 1 :weighty 0 :fill 3)
      ]] .. table.concat(cells, "\n      ") .. [[
    )))
]]

local clicks = 0
function on_cell()
    clicks = clicks + 1
    gui.set("status", "label", "Clicks: " .. clicks)
end

lui.run(ui)
