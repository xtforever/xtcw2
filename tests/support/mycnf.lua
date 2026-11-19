-- Minimal ~/.my.cnf parser for tests.
-- Returns a table: groups[section][key] = value.  Passwords are not printed.
local M = {}

function M.parse(path)
    local groups = {}
    local current
    local f, err = io.open(path, 'r')
    if not f then return nil, err end
    for line in f:lines() do
        line = line:gsub('^%s+', ''):gsub('%s+$', '')
        if line ~= '' and not line:match('^[#;]') then
            local section = line:match('^%[(.+)%]$')
            if section then
                current = section
                groups[current] = groups[current] or {}
            elseif current and line:find('=', 1, true) then
                local key, value = line:match('^([^=]+)=(.*)$')
                if key then
                    key = key:gsub('%s+$', '')
                    value = value:gsub('^%s+', '')
                    groups[current][key] = value
                end
            end
        end
    end
    f:close()
    return groups
end

return M
