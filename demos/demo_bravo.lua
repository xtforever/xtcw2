-- Wbravo demo: a plain-text editor built on a piece table (Bravo/Moore),
-- rendered with the re-tex engine.
--
-- Try: type text, arrows, Home/End, PageUp/PageDown, Backspace/Delete,
-- Return, Ctrl+Z / Ctrl+Y (undo/redo), Ctrl+C / Ctrl+X / Ctrl+V.

local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "Wbravo — piece-table editor" :width 820 :height 560
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr"
           :label "Wbravo — piece table + retex  (Ctrl+Z/Y undo/redo, Ctrl+C/X/V)"
           :gridx 0 :gridy 0 :gridWidth 1 :weightx 1 :weighty 0 :fill 3)
    (Wbravo :id "ed"
            :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3
            :text "Hello, Bravo!\n\nThis is a piece-table editor.\nType here: "
            :fontFace "Sans" :fontSize 16
            :bg_norm "#101018" :fg_norm "#e8e8e8"
            :bg_sel "#204080" :fg_sel "#ffffff")
  ))
]]

local ok, err = lui.run(ui)
if not ok then error("demo_bravo: " .. tostring(err)) end
