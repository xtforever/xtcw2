-- WToggleSwitch demo: an animated oval switch with a sliding filled circle.
-- Click a switch (or press space/Return when it has focus) to flip it; the
-- knob slides across and the track colour changes from a muted "off" grey to
-- a highlight accent.  Programmatic `gui.set(id, "checked", true)` also
-- animates but does not fire the callback.
--
-- The gridbox background now follows the theme (gridbox_bg / gridbox_border),
-- so the toggle's rounded/transparent window corners reveal the themed grid
-- colour.  Switch themes (see demo_theme.lua) to recolour the container.
local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "w" :title "WToggleSwitch" :width 420 :height 300
  (grid :id "g" :weightx 1 :weighty 1
    (label :id "hdr" :label "WToggleSwitch" :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (label :id "l0" :label "Wi-Fi" :gridx 0 :gridy 1 :weightx 1 :weighty 0 :fill 3)
    (WToggleSwitch :id "t0" :gridx 1 :gridy 1 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_toggle, t0)")
    (label :id "l1" :label "Bluetooth" :gridx 0 :gridy 2 :weightx 1 :weighty 0 :fill 3)
    (WToggleSwitch :id "t1" :gridx 1 :gridy 2 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_toggle, t1)")
    (label :id "l2" :label "Airplane" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3)
    (WToggleSwitch :id "t2" :gridx 1 :gridy 3 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_toggle, t2)")
    (label :id "status" :label "status: t0=off t1=off t2=off" :gridx 0 :gridy 4 :gridWidth 3 :weightx 1 :weighty 1 :fill 3)
    (button :id "q" :label "Quit" :gridx 0 :gridy 5 :gridWidth 3 :weightx 1 :weighty 0 :fill 3 :callback "quit_cb")
  ))
]]

local function refresh()
    local parts = {}
    for _, id in ipairs({ "t0", "t1", "t2" }) do
        parts[#parts + 1] = id .. "=" .. (gui.get(id, "checked") and "on" or "off")
    end
    gui.set("status", "label", "status: " .. table.concat(parts, " "))
end

function on_toggle(id)
    refresh()
end

local ok, err = lui.run(ui)
if not ok then
    error("demo_wtoggleswitch: " .. tostring(err))
end

-- Start with one switch already on.  Resource assignment is silent (no
-- callback), so refresh the status line explicitly.
gui.set("t1", "checked", true)
refresh()
