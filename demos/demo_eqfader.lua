-- 10-band EQ demo for the EqFader widget.
-- Each fader is a vertical Cairo/Xft fader with a dB scale, tick labels and a
-- frequency label. Dragging (or scroll wheel / arrow keys) updates the readout.
local gui = require('gui_xt')
local lui = require('lui')

local bands = {
    { freq = "31 Hz",  db =  6 },
    { freq = "63 Hz",  db =  4 },
    { freq = "125 Hz", db =  1 },
    { freq = "250 Hz", db = -2 },
    { freq = "500 Hz", db = -3 },
    { freq = "1 kHz",  db = -1 },
    { freq = "2 kHz",  db =  2 },
    { freq = "4 kHz",  db =  4 },
    { freq = "8 kHz",  db =  3 },
    { freq = "16 kHz", db =  0 },
}

-- id -> frequency, so on_band() can show a friendly name
local freq_by_id = {}

local rows = {}
for i, b in ipairs(bands) do
    local id = "eq" .. (i - 1)
    freq_by_id[id] = b.freq
    rows[#rows + 1] = string.format(
        '    (EqFader :id "%s" :gridx %d :gridy 1 :weightx 1 :weighty 1 :fill 3' ..
        ' :label "%s" :xftFont "Sans-10" :callback "LUA(on_band, %s)")',
        id, i - 1, b.freq, id)
end

local ui = [[
(window :id "w" :title "10-Band EQ - EqFader" :width 1000 :height 470
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "hdr" :label "10-Band EQ" :gridx 0 :gridy 0 :gridWidth 10 :weightx 1 :weighty 0 :fill 3)
]] .. table.concat(rows, "\n") .. [[
    (label :id "rd" :label "Drag a fader (scroll / arrow keys also work)" :gridx 0 :gridy 2 :gridWidth 10 :weightx 1 :weighty 0 :fill 3)
    (button :id "q" :label "Quit" :gridx 0 :gridy 3 :gridWidth 10 :weightx 1 :weighty 0 :fill 3 :callback "quit_cb")
  ))
]]

function on_band(id)
    local v = tonumber(gui.get(id, "value")) or 0
    local name = freq_by_id[id] or id
    gui.set("rd", "label", string.format("%s: %+.2f dB", name, v / 100.0))
end

local ok, err = lui.run(ui)
if not ok then
    error("demo_eqfader: " .. tostring(err))
end

-- The LUI parser has no negative-number literals, so negative band gains
-- cannot be written in the S-expression above. Apply them programmatically
-- (set_values clamps and redraws).
for i, b in ipairs(bands) do
    gui.set("eq" .. (i - 1), "value", b.db * 100)
end
