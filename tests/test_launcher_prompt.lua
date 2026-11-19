-- Unit tests for apps/admpanel/prompt.lua
package.path = 'apps/admpanel/?.lua;' .. package.path
local prompt = require('prompt')

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

local p = prompt.parse_confirmation('Continue (Y/n)? ')
ok(p, 'Y/n detected')
eq(p.question, 'Continue', 'question')
eq(table.concat(p.options, ','), 'y,n', 'options')
eq(p.default, 'y', 'default Y')

p = prompt.parse_confirmation('Delete customer? (y/N)')
ok(p, 'y/N detected')
eq(p.question, 'Delete customer?', 'question with ?')
eq(p.default, 'n', 'default N')

p = prompt.parse_confirmation('Really do it (Y/n/a)?')
ok(p, 'Y/n/a detected')
eq(table.concat(p.options, ','), 'y,n,a', 'three options')
eq(p.default, 'y', 'three option default')

p = prompt.parse_confirmation('( y / n ) ?')
ok(p, 'spaced group detected')
eq(table.concat(p.options, ','), 'y,n', 'spaced options')
eq(p.default, 'y', 'spaced default')

p = prompt.parse_confirmation('[Y/n]?')
ok(p, 'bracket form detected')
eq(table.concat(p.options, ','), 'y,n', 'bracket options')

eq(prompt.parse_confirmation('nothing to see'), nil, 'plain line is not a prompt')
eq(prompt.parse_confirmation('Continue (Y/n)'), nil, 'missing ? is not a prompt')
eq(prompt.parse_confirmation('(x/z)?'), nil, 'unknown letters are not a prompt')

eq(prompt.answer_letter('yes'), 'y', 'yes -> y')
eq(prompt.answer_letter('NO'), 'n', 'NO -> n')
eq(prompt.answer_letter('abort'), 'a', 'abort -> a')
eq(prompt.label('a'), 'Abort', 'label abort')

local plan = prompt.dialog_plan(prompt.parse_confirmation('Continue (Y/n)?'))
eq(#plan, 2, 'two dialog buttons')
eq(plan[1].role, 'ok', 'first button ok')
eq(plan[1].letter, 'y', 'ok letter default')
eq(plan[2].role, 'cancel', 'second button cancel')
eq(plan[2].letter, 'n', 'cancel letter')

plan = prompt.dialog_plan(prompt.parse_confirmation('Continue (Y/n/a)?'))
eq(#plan, 3, 'three dialog buttons')
eq(plan[3].role, 'aux', 'third button aux')
eq(plan[3].letter, 'a', 'aux letter')

local list = prompt.parse_item_list({
    'item list:',
    '1. alpha',
    '2. beta',
    'select item (1/2)?',
})
ok(list, 'item list detected')
eq(#list.items, 2, 'item count')
eq(list.items[1], 'alpha', 'item 1')
eq(list.items[2], 'beta', 'item 2')
eq(list.question, 'select item (1/2)?', 'item question')

list = prompt.parse_item_list('item list:\n1) one\n2) two\nwhich one?')
ok(list, 'item list from string')
eq(#list.items, 2, 'string item count')
eq(list.items[2], 'two', 'string item 2')

eq(prompt.parse_item_list('no list here'), nil, 'no item list')

print('test_launcher_prompt: OK')
