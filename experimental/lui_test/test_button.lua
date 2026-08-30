local gui = require('gui_xt')

local ui = [[
(window :id "tw" :title "Button Test" :width 200 :height 100
  (grid :id "gr"
    (command :id "btn1" :label "Click Me" :callback "LUA(test_click)")
    (label :id "result" :label "Not clicked")
  ))
]]

local click_count = 0

function test_click()
    click_count = click_count + 1
    gui.set("result", "label", "Clicked " .. click_count .. " times")
end

lui.run(ui)

-- Callbacks dispatched via 100ms polling timer in commander_runner.
-- Must wait for callback to be processed after xtaction calls.
xtapptimeout(500, function()
    print("=== Button Test Start ===")

    local b = gui.get_widget("btn1")
    if b then
        xtaction(b, "set")
        xtaction(b, "notify")
        xtaction(b, "unset")
    else
        print("FAIL: btn1 widget not found")
    end

    -- Wait for callback dispatch (process_lua_cbs polls every 100ms)
    xtapptimeout(300, function()
        if click_count > 0 then
            print("PASS: button callback fired, count=" .. click_count)
        else
            print("FAIL: button callback did not fire")
        end

        local label = gui.get("btn1", "label")
        if label == "Click Me" then
            print("PASS: button label correct")
        else
            print("FAIL: button label wrong: " .. tostring(label))
        end

        local result_label = gui.get("result", "label")
        if result_label and tostring(result_label):match("Clicked") then
            print("PASS: result label updated after click")
        else
            print("FAIL: result label not updated: " .. tostring(result_label))
        end

        gui.set("btn1", "label", "Changed")
        local new_label = gui.get("btn1", "label")
        if new_label == "Changed" then
            print("PASS: label set/get round-trip works")
        else
            print("FAIL: label set/get failed, got: " .. tostring(new_label))
        end

        print("=== Button Test End ===")
        xtapptimeout(200, function() xtappexitflag() end)
    end)
end)