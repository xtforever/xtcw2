---
-- @file src/parser.lua
-- @brief LPeg-based parser for the Lui language.
--
-- This module converts Lui source code (S-expressions) into a Lua table-based Abstract Syntax Tree (AST).
-- It handles:
-- - Basic atoms: symbols, keywords, numbers, strings, booleans, nil.
-- - Lists: (nested tables).
-- - Special forms: `(require ...)`, `(defmacro ...)`, `(lua ...)`.
-- - Comments: semicolons `;`.
---

local lpeg = require("lpeg")
local S, R, P, C, Ct, V, Cc = lpeg.S, lpeg.R, lpeg.P, lpeg.C, lpeg.Ct, lpeg.V, lpeg.Cc

-- Sentinels
local NIL = { type = "nil", tostring = function() return "nil" end }
setmetatable(NIL, { __tostring = NIL.tostring })

-- ... (grammar definition omitted for brevity in docs, but kept in code)

local comment = P(";") * (P(1) - P("\n"))^0
local white_char = S(" \t\r\n")
local skip = (white_char + comment)^0

local function T(p) return p * skip end

local digit = R("09")
local letter = R("az", "AZ") + P("_")
-- symbol-char = letter | digit | "-" | "_" | "." ;
-- Note: letter includes _, digit is 0-9. S("-_.") covers the rest.
local symchar = letter + digit + S("-_.")

-- symbol = letter , { symbol-char } ;
local symbol_raw = letter * symchar^0
local symbol = C(symbol_raw)

-- keyword = ":" , symbol ;
local keyword = P(":") * symbol / function(s) return {keyword=":"..s} end

-- number = digit , { digit } ;
-- Improved to handle floats: digits . digits
local number_raw = digit^1 * (P(".") * digit^1)^-1
local number = C(number_raw) / tonumber

-- String unescaping helper
local function unescape(s)
    s = s:gsub('\\"', '"')
    s = s:gsub('\\\\', '\\')
    s = s:gsub('\\n', '\n')
    s = s:gsub('\\t', '\t')
    return s
end

-- string = '"' , { character - '"' } , '"' ;
-- Body of string: handles escapes \"
local string_body = ((P("\\") * P(1)) + (P(1) - P("\"")))^0
-- Raw string with quotes (for Lua blocks) - keeps escapes
local string_raw = P("\"") * string_body * P("\"")
-- Captured string content (for AST) - unescapes
local string_val = P("\"") * C(string_body) * P("\"") / unescape

local boolean = (P("true") * Cc(true)) + (P("false") * Cc(false))
local nil_token = P("nil") * Cc(NIL)

local g = P{
  "program",
  program = skip * Ct(V("exp")^0),
  
  exp = T(V("macro") + V("require_block") + V("lua_block") + V("list") + V("atom")),
  
  atom = boolean + nil_token + number + string_val + keyword + symbol,

  list = P("(") * skip * Ct(V("exp")^0) * P(")"),

  require_block = P("(") * skip * P("require") * skip * 
                  T(string_val) * (T(number)^-1) * 
                  P(")") / function(name, version) 
                    return { require = name, version = version } 
                  end,

  -- Lua code matcher that respects balanced parentheses and strings
  lua_element = string_raw + 
                comment +
                (P("(") * V("lua_content") * P(")")) +
                (P(1) - S("()")),

  lua_content = V("lua_element")^0,

  lua_block = P("(") * skip * P("lua") * 
              C(V("lua_content")) * 
              P(")") / function(code) return { lua = code } end,
  
  -- Special handling for defmacro to produce distinct AST node
  macro = P("(") * skip * P("defmacro") * skip * T(symbol) *
          P("(") * skip * Ct(T(symbol)^0) * P(")") * skip *
          Ct(V("exp")^0) * P(")") / function(name, args, body)
            return { defmacro = name, args = args, body = body }
          end,
}

--- Parses the given Lui source code.
-- @param s (string) The source code string.
-- @return (table|nil) The AST (list of expressions) if successful, or nil on error.
-- @sideeffect Prints syntax error details to stdout if parsing fails.
function parse(s)
  local ast, pos = (g * lpeg.Cp()):match(s)
  
  if pos and pos <= #s then
      -- Parsing finished but didn't consume all input
      local line = 1
      local col = 1
      for i = 1, pos do
          if s:sub(i, i) == "\n" then
              line = line + 1
              col = 1
          else
              col = col + 1
          end
      end
      
      -- Get context
      local start_ctx = math.max(1, pos - 20)
      local end_ctx = math.min(#s, pos + 20)
      local context = s:sub(start_ctx, end_ctx):gsub("\n", "\\n")
      
      print(string.format("Syntax Error at line %d, column %d", line, col))
      print(string.format("Context: ...%s...", context))
      print(string.format("Stopped at character: '%s' (byte %d)", s:sub(pos, pos), pos))
      
      -- If we returned a partial AST, maybe we should return nil to indicate failure?
      -- Current behavior was silently ignoring tail. 
      -- Returning partial AST might be confusing if it missed half the file.
      -- Let's return nil to force error handling upstream.
      return nil
  end
  
  return ast
end

return { parse = parse, NIL = NIL }
