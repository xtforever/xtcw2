local parser = require('parser')
local lui_source = '(label :id "msg" :label "Hello")'
local ast = parser.parse(lui_source)

local function dump(t, indent)
    indent = indent or ''
    for k, v in pairs(t) do
        if type(v) == 'table' then
            print(indent .. tostring(k) .. ':')
            dump(v, indent .. '  ')
        else
            print(indent .. tostring(k) .. ': ' .. tostring(v))
        end
    end
end

dump(ast)
