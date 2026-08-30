local lui = require('lui')

print("LUI Test: Starting...")

_G.quit_cb = function()
    print("LUI Test: Quit pressed, exiting...")
    os.exit(0)
end

_G.lui_source = [[(window :title "LUI Test" :width 300 :height 200
  (grid
  (label :label "Hello from LUI!" :fontSize 18)
  (button :label "Quit" :callback "quit_cb" :gridy 1 )
  ))]]

print("LUI Test: Starting event loop...")
lui.loop()
