local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "Gridbox Demo" :width 600 :height 400
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr" :label "Gridbox Layout Demo" :gridx 0 :gridy 0 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (label :id "name_lbl" :label "Name:" :gridx 0 :gridy 1 :weightx 0 :weighty 0 :fill 3)
    (edit :id "name_ed" :label "Enter name" :gridx 1 :gridy 1 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (label :id "age_lbl" :label "Age:" :gridx 0 :gridy 2 :weightx 0 :weighty 0 :fill 3)
    (grid :id "age_box" :gridx 1 :gridy 2 :weightx 1 :weighty 0 :fill 3 :borderWidth 0
      (WspinBox :id "age_spin" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :value 25 :min 0 :max 120)
      (button :id "age_down" :label "-" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :width 32 :leftGap 8 :rightGap 8 :alignment 1 :callback "LUA(on_age_down)")
      (button :id "age_up" :label "+" :gridx 2 :gridy 0 :weightx 0 :weighty 0 :fill 3 :width 32 :leftGap 8 :rightGap 8 :alignment 1 :callback "LUA(on_age_up)")
    )
    (grid :id "active_box" :gridx 2 :gridy 2 :weightx 1 :weighty 0 :fill 3 :borderWidth 0
      (label :id "active_lbl" :label "Active" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3)
      (WToggleSwitch :id "active_tog" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :fill 3 :checked FALSE :callback "LUA(on_toggle)")
    )
    (label :id "status" :label "Status: ready" :gridx 0 :gridy 3 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (button :id "ok_btn" :label "OK" :gridx 1 :gridy 4 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_ok)")
    (button :id "cancel_btn" :label "Cancel" :gridx 2 :gridy 4 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_cancel)")
  ))
]]

function on_toggle(data)
    local state = gui.get("active_tog", "checked")
    gui.set("status", "label", "Active: " .. tostring(state))
end

function on_ok(data)
    local name = gui.get("name_ed", "label")
    local age = gui.get("age_spin", "value")
    local active = gui.get("active_tog", "checked")
    gui.set("status", "label", "OK! Name=" .. tostring(name) .. " Age=" .. tostring(age) .. " Active=" .. tostring(active))
end

function on_age_up()
    xtaction(gui.get_widget("age_spin"), "spin_up")
end

function on_age_down()
    xtaction(gui.get_widget("age_spin"), "spin_down")
end

function on_cancel(data)
    gui.set("name_ed", "label", "")
    gui.set("age_spin", "value", 25)
    gui.set("age_spin", "label", "25")
    gui.set("active_tog", "checked", false)
    gui.set("status", "label", "Cancelled")
end

lui.run(ui)