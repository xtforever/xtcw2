local lui = require('lui')

-- Global callback for the Quit button
_G.quit_cb = function()
    print("Hello LUI: Terminating cleanly...")
    os.exit(0)
end

-- Create a data store with 3 columns
local list_store = gui.create_store(3)
gui.store_append(list_store, {"Item 1", "Active", "100"})
gui.store_append(list_store, {"Item 2", "Pending", "250"})
gui.store_append(list_store, {"Item 3", "Inactive", "50"})
gui.store_append(list_store, {"Item 4", "Active", "120"})
gui.store_append(list_store, {"Item 5", "Pending", "300"})
for i=6, 20 do
    gui.store_append(list_store, {"Item " .. i, "Status " .. i, tostring(i * 10)})
end

-- UI Definition with Multi-column List
local lui_source = [[(window :title "Hello LUI List" :width 500 :height 400
  (grid
    (label :label "LUI Multi-column List Example" 
           :gridx 0 :gridy 0 :weightx 100 :weighty 0 :fontSize 24)
    
    (list-view :id "my_list" :model $list_placeholder 
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3
               :columns (200 150 100) :retexCells true :fontSize 18
               :visible_lines 10)

    (button :label "Quit" 
            :gridx 0 :gridy 2 :weighty 0 :callback "quit_cb")))]]

-- Replace placeholder with the actual store handle
lui_source = lui_source:gsub("$list_placeholder", tostring(list_store.handle))

print("Hello LUI: Starting application with list...")
lui.run(lui_source)
lui.loop()
