local lui = require('lui')

-- Global callback for the Quit button
_G.quit_cb = function()
    print("Three Column Demo: Terminating...")
    os.exit(0)
end

-- Create a data store with 3 columns
local process_store = gui.create_store(3)

local function add_process(name, pid, cpu)
    gui.store_append(process_store, {name, tostring(pid), cpu .. "%"})
end

-- Populate with some mock data
add_process("Luarunner", 1234, 0.5)
add_process("Xorg Server", 980, 12.4)
add_process("Gnome Shell", 2105, 8.1)
add_process("Vim Editor", 4567, 1.2)
add_process("Firefox Browser", 8892, 25.7)
add_process("Systemd", 1, 0.1)
add_process("Network Manager", 543, 0.2)
add_process("PulseAudio", 1102, 1.5)
add_process("Bash Shell", 3321, 0.0)
add_process("GCC Compiler", 15678, 98.2)
for i=1, 20 do
    add_process("Background Task " .. i, 20000 + i, "0.1")
end

-- UI Definition
-- Columns are: Process Name (250px), PID (100px), CPU (100px)
local lui_source = [[(window :title "Three Column Demo" :width 600 :height 600
  (grid
    (label :label "System Process Monitor" 
           :gridx 0 :gridy 0 :weightx 100 :weighty 0 :fontSize 28 
           :topGap 20 :bottomGap 20 :leftGap 20 :rightGap 20 
           :autoHeight true :alignment 1)
    
    (list-view :id "processes" :model $list_handle 
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3
               :columns (250 100 100) :retexCells true :fontSize 20
               :visible_lines 15)

    (grid :gridx 0 :gridy 2 :weighty 0 :fill 1
       (button :label "Quit" :gridx 0 :gridy 0 :callback "quit_cb")
       (label :label " Status: Monitoring active" :gridx 1 :gridy 0 :weightx 100 :align 1))))]]

lui_source = lui_source:gsub("$list_handle", tostring(process_store.handle))

print("Three Column Demo: Starting...")
lui.run(lui_source)
lui.loop()
