local lui = require('lui')

_G.quit_cb = function()
    print("Quit requested from Lui")
    os.exit(0)
end

-- 1. Create a store with multi-column data
local store = gui.create_store(3)
gui.store_append(store, {"Item 1", "Description A", "Active"})
gui.store_append(store, {"Item 2", "Description B", "Inactive"})
gui.store_append(store, {"Item 3", "Description C", "Pending"})
for i=4,20 do
    gui.store_append(store, {"Item " .. i, "Description " .. i, "Status " .. i})
end

-- 2. Define UI in Lui
local lui_source = [[
(window :title "Multi-column List Test" :width 600 :height 400
  (grid
    (list-view :id "mylist" 
               :gridx 0 :gridy 0 :weightx 100 :weighty 100 :fill 3
               :columns (150 300 100)
               :retexCells true
               :fontFace "Serif"
               :fontSize 18
               :model {handle=$store_placeholder})
    (button :label "Quit" :gridx 0 :gridy 1 :callback "quit_cb")))
]]

-- 3. Replace model placeholder with real handle
lui_source = lui_source:gsub("$store_placeholder", tostring(store.handle))

-- 4. Run and Build
lui.run(lui_source)
lui.loop()
