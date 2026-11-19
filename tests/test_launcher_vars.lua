-- Unit tests for apps/admpanel/vars.lua
package.path = 'apps/admpanel/?.lua;' .. package.path
local vars = require('vars')

local function fail(msg)
    io.stderr:write('FAIL: ' .. msg .. '\n')
    os.exit(1)
end
local function eq(actual, expected, msg)
    if actual ~= expected then
        fail((msg or 'value') .. ': expected ' .. tostring(expected) ..
             ', got ' .. tostring(actual))
    end
end
local function ok(cond, msg)
    if not cond then fail(msg or 'condition failed') end
end

-- Basic field expansion
local s = vars.expand('auswahl.sh $kk', { kk = 'jens' })
eq(s, 'auswahl.sh jens', 'simple variable')

-- Braces and adjacent variables
s = vars.expand('${srv}/$kk$sandbox', { srv = 'sandbox7', kk = 'jeh', sandbox = '10' })
eq(s, 'sandbox7/jeh10', 'braces and adjacency')

-- Literal dollar
s = vars.expand('cost $$5 and $$kk', {})
eq(s, 'cost $5 and $kk', 'dollar escape')

-- Unknown variable is reported and becomes empty
local text, missing = vars.expand('x=$nope y=$kk', { kk = 'k' })
eq(text, 'x= y=k', 'missing becomes empty')
eq(#missing, 1, 'one missing')
eq(missing[1], 'nope', 'missing name')

-- Legacy 0-based array indices
s = vars.expand('$arr[0]-$arr[1]-$arr[2]', { arr = { 'a', 'b', 'c' } })
eq(s, 'a-b-c', 'array indices')

-- Non-variable shell characters stay literal
s = vars.expand('$? $1 $-', {})
eq(s, '$? $1 $-', 'shell specials stay literal')

-- Shell quoting for command construction
eq(vars.shell_quote('abc'), 'abc', 'safe word unquoted')
eq(vars.shell_quote('a b'), "'a b'", 'space quoted')
eq(vars.shell_quote("it's"), "'it'\\''s'", 'single quote escaped')
eq(vars.shell_quote(''), "''", 'empty quoted')

s = vars.expand('cmd --name $name', { name = 'a b' }, { quote_values = true })
eq(s, "cmd --name 'a b'", 'quoted values')

-- Tokenizer for execvp mode
local args, err = vars.tokenize("cmd --name 'a b' \"c d\" e\\ f")
ok(args, err)
eq(#args, 5, 'token count')
eq(args[1], 'cmd', 'token 1')
eq(args[2], '--name', 'token 2')
eq(args[3], 'a b', 'token 3')
eq(args[4], 'c d', 'token 4')
eq(args[5], 'e f', 'token 5')

local bad, terr = vars.tokenize("cmd 'unterminated")
ok(not bad, 'unterminated quote rejected')
ok(terr and terr:find('unterminated'), 'unterminated error text')

-- Strict expansion
local strict, serr = vars.expand_strict('$a $b', { a = '1' })
ok(not strict, 'strict fails on missing')
ok(serr and serr:find('b'), 'strict reports missing name')

print('test_launcher_vars: OK')
