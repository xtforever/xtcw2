-- GUI Test with Label for commander_runner
print("GUI Test Starting...")

-- Create a label widget using xtcreate
-- Format: xtcreate(name, class, parent, resource, value, ...)
local label = xtcreate("testLabel", "label", "paned", "label", "Hello from Lua!")
print("Created label: " .. tostring(label))

-- Get the label value back
local text = xtgetvalue(label, "label")
print("Label text: " .. tostring(text))

-- Create another button
local button = xtcreate("testButton", "command", "paned", "label", "Click Me!")
print("Created button: " .. tostring(button))

print("GUI Test Completed Successfully!")
