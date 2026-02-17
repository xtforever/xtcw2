local parser = require("parser")
local ast = parser.parse("(window :title \"test\")")
print("AST:", ast)
if ast then
    print("#AST:", #ast)
    if ast[1] then
        print("AST[1][1]:", ast[1][1])
    end
end
