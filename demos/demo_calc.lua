local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "Calculator" :width 280 :height 320
  (grid :id "calc" :weightx 1 :weighty 1
    (Wlabel :id "display" :label "0" :gridx 0 :gridy 0 :gridWidth 4 :weightx 1 :weighty 0 :fill 3)
    (command :id "c_btn" :label "C" :gridx 0 :gridy 1 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_clear)")
    (command :id "neg_btn" :label "+/-" :gridx 1 :gridy 1 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_negate)")
    (command :id "pct_btn" :label "%" :gridx 2 :gridy 1 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_percent)")
    (command :id "div_btn" :label "/" :gridx 3 :gridy 1 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_op,div)")
    (command :id "7_btn" :label "7" :gridx 0 :gridy 2 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,7)")
    (command :id "8_btn" :label "8" :gridx 1 :gridy 2 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,8)")
    (command :id "9_btn" :label "9" :gridx 2 :gridy 2 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,9)")
    (command :id "mul_btn" :label "x" :gridx 3 :gridy 2 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_op,mul)")
    (command :id "4_btn" :label "4" :gridx 0 :gridy 3 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,4)")
    (command :id "5_btn" :label "5" :gridx 1 :gridy 3 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,5)")
    (command :id "6_btn" :label "6" :gridx 2 :gridy 3 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,6)")
    (command :id "sub_btn" :label "-" :gridx 3 :gridy 3 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_op,sub)")
    (command :id "1_btn" :label "1" :gridx 0 :gridy 4 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,1)")
    (command :id "2_btn" :label "2" :gridx 1 :gridy 4 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,2)")
    (command :id "3_btn" :label "3" :gridx 2 :gridy 4 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,3)")
    (command :id "add_btn" :label "+" :gridx 3 :gridy 4 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_op,add)")
    (command :id "0_btn" :label "0" :gridx 0 :gridy 5 :gridWidth 2 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_digit,0)")
    (command :id "dot_btn" :label "." :gridx 2 :gridy 5 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_dot)")
    (command :id "eq_btn" :label "=" :gridx 3 :gridy 5 :weightx 1 :weighty 1 :fill 3 :callback "LUA(on_equals)")
  ))
]]

local current = "0"
local accumulator = 0
local pending_op = nil
local new_entry = true

local function update_display()
    gui.set("display", "label", current)
end

function on_digit(arg)
    local d = arg
    if new_entry then
        current = d
        new_entry = false
    else
        if current == "0" and d ~= "0" then current = d
        elseif current ~= "0" then current = current .. d
        end
    end
    update_display()
end

function on_dot()
    if new_entry then current = "0."; new_entry = false
    elseif not current:find("%.") then current = current .. "."
    end
    update_display()
end

function on_clear()
    current = "0"
    accumulator = 0
    pending_op = nil
    new_entry = true
    update_display()
end

function on_negate()
    if current ~= "0" then
        if current:sub(1,1) == "-" then current = current:sub(2)
        else current = "-" .. current
        end
        update_display()
    end
end

function on_percent()
    current = tostring(tonumber(current) / 100)
    update_display()
end

function on_op(arg)
    local op = arg
    local val = tonumber(current) or 0
    if pending_op and not new_entry then
        if pending_op == "add" then accumulator = accumulator + val
        elseif pending_op == "sub" then accumulator = accumulator - val
        elseif pending_op == "mul" then accumulator = accumulator * val
        elseif pending_op == "div" then accumulator = val ~= 0 and (accumulator / val) or 0
        end
    else
        accumulator = val
    end
    pending_op = op
    new_entry = true
    current = tostring(accumulator)
    update_display()
end

function on_equals()
    local val = tonumber(current) or 0
    if pending_op then
        if pending_op == "add" then accumulator = accumulator + val
        elseif pending_op == "sub" then accumulator = accumulator - val
        elseif pending_op == "mul" then accumulator = accumulator * val
        elseif pending_op == "div" then accumulator = val ~= 0 and (accumulator / val) or 0
        end
        current = tostring(accumulator)
        pending_op = nil
        new_entry = true
    end
    update_display()
end

lui.run(ui)