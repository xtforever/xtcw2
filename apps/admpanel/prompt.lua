-- Detection of interactive prompts emitted by a running shell script.

local M = {}

local VALID = { y = true, n = true, a = true, c = true, o = true }
local LABELS = {
    y = 'Yes',
    n = 'No',
    a = 'Abort',
    c = 'Cancel',
    o = 'OK',
}

local function trim(s)
    return (tostring(s or ''):match('^%s*(.-)%s*$'))
end

-- Parse a confirmation line such as "Continue (Y/n)? ".
-- Returns { question, options = {"y","n"}, default = "y", raw } or nil.
function M.parse_confirmation(text)
    if type(text) ~= 'string' then return nil end

    local question, group = text:match('^(.*)%(%s*([%a/? ]+)%s*%)%s*%??%s*$')
    if not group then
        question, group = text:match('^(.*)%[%s*([%a/? ]+)%s*%]%s*%??%s*$')
    end
    if not group then return nil end
    -- Require a question mark somewhere: after the group, inside it, or in
    -- the question text ("Delete customer? (y/N)").
    if not text:find('?', 1, true) then return nil end

    local options = {}
    local seen = {}
    local default
    for ch in group:gmatch('[%a]') do
        local lower = ch:lower()
        if VALID[lower] and not seen[lower] then
            seen[lower] = true
            options[#options + 1] = lower
            if ch == ch:upper() and not default then default = lower end
        end
    end
    if #options == 0 then return nil end

    default = default or options[1]
    question = trim(question)
    if question == '' then question = trim(text) end

    return {
        question = question,
        options = options,
        default = default,
        raw = text,
    }
end

function M.label(letter)
    return LABELS[tostring(letter or ''):lower()] or tostring(letter or '')
end

function M.answer_letter(word)
    local map = { yes = 'y', no = 'n', abort = 'a', cancel = 'c', ok = 'o' }
    return map[tostring(word or ''):lower()]
end

-- Map a parsed confirmation to WDialog buttons. WDialog supports three
-- buttons (ok, cancel, aux). The script's default option becomes OK so that
-- Return follows the script's own default.
function M.dialog_plan(parsed)
    if not parsed then return nil end

    local default = parsed.default
    local used = { [default] = true }
    local others = {}
    for _, letter in ipairs(parsed.options) do
        if not used[letter] then
            used[letter] = true
            others[#others + 1] = letter
        end
    end

    local plan = {
        { role = 'ok', letter = default, label = M.label(default) },
    }
    if others[1] then
        plan[#plan + 1] = { role = 'cancel', letter = others[1], label = M.label(others[1]) }
    end
    if others[2] then
        plan[#plan + 1] = { role = 'aux', letter = others[2], label = M.label(others[2]) }
    end
    return plan
end

local function split_lines(input)
    if type(input) == 'table' then return input end
    local lines = {}
    for line in (tostring(input or '') .. '\n'):gmatch('(.-)\n') do
        lines[#lines + 1] = line
    end
    return lines
end

-- Parse legacy "item list:" output:
--   item list:
--   1. alpha
--   2. beta
--   select item (1/2)?
function M.parse_item_list(input)
    local lines = split_lines(input)
    local start
    for i, line in ipairs(lines) do
        if trim(line):lower():match('^item%s+list%s*:?$') then
            start = i
            break
        end
    end
    if not start then return nil end

    local items = {}
    local i = start + 1
    while i <= #lines do
        local line = trim(lines[i])
        local num, text = line:match('^(%d+)[%.%)]%s*(.*)$')
        if not num then break end
        items[#items + 1] = trim(text)
        i = i + 1
    end
    if #items == 0 then return nil end

    local question
    while i <= #lines do
        local line = trim(lines[i])
        if line ~= '' then question = line; break end
        i = i + 1
    end

    return {
        question = question or 'Select item',
        items = items,
        raw = input,
    }
end

return M
