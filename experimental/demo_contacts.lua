local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "Contacts" :width 500 :height 350
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "hdr" :label "Contact Manager" :gridx 0 :gridy 0 :gridWidth 4 :weightx 1 :weighty 0 :fill 3)
    (label :id "fn_lbl" :label "First Name:" :gridx 0 :gridy 1 :weightx 0 :weighty 0 :fill 3)
    (edit :id "fn_ed" :label "" :gridx 1 :gridy 1 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (label :id "ln_lbl" :label "Last Name:" :gridx 0 :gridy 2 :weightx 0 :weighty 0 :fill 3)
    (edit :id "ln_ed" :label "" :gridx 1 :gridy 2 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (label :id "em_lbl" :label "Email:" :gridx 0 :gridy 3 :weightx 0 :weighty 0 :fill 3)
    (edit :id "em_ed" :label "" :gridx 1 :gridy 3 :gridWidth 3 :weightx 1 :weighty 0 :fill 3)
    (label :id "ph_lbl" :label "Phone:" :gridx 0 :gridy 4 :weightx 0 :weighty 0 :fill 3)
    (WspinBox :id "ph_area" :gridx 1 :gridy 4 :weightx 1 :weighty 0 :fill 3 :value 1 :min 200 :max 999)
    (label :id "ph_dash" :label "-" :gridx 2 :gridy 4 :weightx 0 :weighty 0 :fill 3)
    (edit :id "ph_num" :label "0000000" :gridx 3 :gridy 4 :weightx 1 :weighty 0 :fill 3)
    (label :id "list_lbl" :label "Saved Contacts:" :gridx 0 :gridy 5 :gridWidth 4 :weightx 1 :weighty 0 :fill 3)
    (Wlabel :id "contact_list" :label "(none)" :gridx 0 :gridy 6 :gridWidth 4 :weightx 1 :weighty 1 :fill both)
    (button :id "save_btn" :label "Save" :gridx 1 :gridy 7 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_save)")
    (button :id "clear_btn" :label "Clear" :gridx 2 :gridy 7 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_clear)")
    (button :id "quit_btn" :label "Quit" :gridx 3 :gridy 7 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_quit)")
  ))
]]

local contacts = {}

function on_save()
    local fn = gui.get("fn_ed", "label") or ""
    local ln = gui.get("ln_ed", "label") or ""
    local em = gui.get("em_ed", "label") or ""
    local area = tostring(gui.get("ph_area", "value") or "")
    local ph = gui.get("ph_num", "label") or ""
    if fn == "" and ln == "" then
        gui.set("contact_list", "label", "Please enter a name")
        return
    end
    local entry = ln .. ", " .. fn .. "  " .. em .. "  " .. area .. "-" .. ph
    table.insert(contacts, entry)
    gui.set("contact_list", "label", table.concat(contacts, "\n"))
    on_clear()
end

function on_clear()
    gui.set("fn_ed", "label", "")
    gui.set("ln_ed", "label", "")
    gui.set("em_ed", "label", "")
    gui.set("ph_num", "label", "0000000")
    gui.set("ph_area", "value", 1)
end

function on_quit()
    xtappexitflag()
end

lui.run(ui)