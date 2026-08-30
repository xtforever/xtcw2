local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "Login" :width 350 :height 200
  (grid :id "main" :weightx 1 :weighty 1
    (label :id "t1" :label "Please sign in" :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (label :id "user_lbl" :label "Username:" :gridx 0 :gridy 1 :weightx 0 :weighty 0 :fill 3)
    (Wpassword :id "user_ed" :label "" :gridx 1 :gridy 1 :weightx 1 :weighty 0 :fill 3)
    (label :id "pw_lbl" :label "Password:" :gridx 0 :gridy 2 :weightx 0 :weighty 0 :fill 3)
    (Wpassword :id "pw_ed" :label "" :gridx 1 :gridy 2 :weightx 1 :weighty 0 :fill 3)
    (toggle :id "remember" :label "Remember me" :gridx 0 :gridy 3 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (label :id "msg" :label "" :gridx 0 :gridy 4 :gridWidth 2 :weightx 1 :weighty 1 :fill 3)
    (button :id "login_btn" :label "Login" :gridx 0 :gridy 5 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_login)")
    (button :id "cancel_btn" :label "Cancel" :gridx 1 :gridy 5 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_cancel)")
  ))
]]

local attempts = 0

function on_login()
    local user = gui.get("user_ed", "label") or ""
    local pw = gui.get("pw_ed", "label") or ""
    attempts = attempts + 1
    if user == "" then
        gui.set("msg", "label", "Please enter a username")
    elseif user == "admin" and pw == "secret" then
        gui.set("msg", "label", "Login successful!")
    else
        gui.set("msg", "label", "Invalid credentials (attempt " .. attempts .. ")")
    end
end

function on_cancel()
    gui.set("user_ed", "label", "")
    gui.set("pw_ed", "label", "")
    gui.set("msg", "label", "")
end

lui.run(ui)