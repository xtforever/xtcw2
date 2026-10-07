-- KaroEd demo: a multiline text editor widget (Wheel subclass).
--
-- Exercised features: text insertion, Return (auto-indent), Backspace/Delete
-- line joins, arrows, Home/End, PageUp/PageDown, flow selection, Tab stops,
-- and Ctrl+A/C/X/V clipboard operations.  The widget's change callback is
-- wired to LUA(on_edit) via the Wheel `callback` resource.

local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "KaroEd Multiline Editor" :width 820 :height 560
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr"
           :label "KaroEd — multiline text editor"
           :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (KaroEd :id "ed"
            :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1 :fill 3
            :xftFont "Monospace-14"
            :grid_width 80 :grid_height 9 :auto_resize 0
            :bg_norm "#101018" :fg_norm "#e8e8e8"
            :callback "LUA(on_edit)")
    (label :id "status"
           :label "Type text. Return/Backspace join lines. Home/End, Tab, Ctrl+A/C/X/V."
           :gridx 0 :gridy 2 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
  ))
]]

local edits = 0

function on_edit(data)
    edits = edits + 1
    gui.set("status", "label", "edits: " .. edits)
end

lui.run(ui)
