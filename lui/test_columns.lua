local lui = require('lui')

-- Global callback for the Quit button
_G.quit_cb = function()
    print("Test Columns: Terminating...")
    os.exit(0)
end

-- Create a data store with 3 columns
local store = gui.create_store(3)

-- Add rows with tab-separated values
gui.store_append(store, {"Column1", "Col2", "C3"})
gui.store_append(store, {"First", "Second", "Third"})
gui.store_append(store, {"A", "B", "C"})

-- UI Definition
local lui_source = [[(window :title "Test Columns" :width 600 :height 400
  (grid
    (label :label "Three Column Test" 
           :gridx 0 :gridy 0 :weightx 100 :weighty 0 :fontSize 24)
    
    (list-view :id "list" :model $STORE
               :gridx 0 :gridy 1 :weightx 100 :weighty 100 :fill 3
               :columns (200 150 100) :retexCells true :fontSize 18)))
]]

lui_source = lui_source:gsub("$STORE", tostring(store.handle))

print("Starting test...")
print("Store handle:", store.handle)
lui.run(lui_source)
lui.loop()
