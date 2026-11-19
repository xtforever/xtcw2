-- Dynamic LUI form for the AdmPanel launcher.

local gui = require('gui_xt')
local lui = require('lui')
local vars = require('vars')
local script = require('script')
local prompt = require('prompt')
local dialog = require('dialog')
local sql = require('sql')
local scriptmeta = require('scriptmeta')

local M = {
    cfg = nil,
    store = {},
    values = {},
    fields_by_id = {},
    log_rows = {},
    sql_maps = {},
    command = '',
    job = nil,
    prompt_active = false,
    prompt_queue = {},
}

local MAX_LOG_ROWS = 2000
local log_flush_scheduled = false

local function esc(s)
    s = tostring(s or '')
    s = s:gsub('\\', '\\\\')
    s = s:gsub('"', '\\"')
    s = s:gsub('\n', '\\n')
    s = s:gsub('\r', '\\r')
    s = s:gsub('\t', '\\t')
    return s
end

local function wid(field) return 'fld_' .. tostring(field.index) end
local function lid(field) return 'lbl_' .. tostring(field.index) end

local function set_status(msg)
    gui.set('status', 'label', tostring(msg or ''))
end

local function schedule_log_flush()
    if log_flush_scheduled then return end
    log_flush_scheduled = true
    xtapptimeout(0, function()
        log_flush_scheduled = false
        gui.set_string_list('log', 'rows', M.log_rows)
        gui.set('log', 'selected', #M.log_rows - 1)
    end)
end

function M.add_log(line)
    M.log_rows[#M.log_rows + 1] = tostring(line or '')
    while #M.log_rows > MAX_LOG_ROWS do
        table.remove(M.log_rows, 1)
    end
    schedule_log_flush()
end

function M.get_log()
    local out = {}
    for i, line in ipairs(M.log_rows) do out[i] = line end
    return out
end

local function field_fragment(field, row)
    local label = field.title
    if field.mandatory then label = label .. ' *' end

    local parts = {
        string.format(
            '(label :id "%s" :label "%s" :gridx 0 :gridy %d :weightx 0 :weighty 0 :fill 3)',
            lid(field), esc(label), row),
    }

    local id = wid(field)
    if field.type == 'input' then
        parts[#parts + 1] = string.format(
            '(WLineEdit :id "%s" :label "%s" :placeholder "%s" :gridx 1 :gridy %d ' ..
            ':weightx 1 :weighty 0 :fill 3 :showClear FALSE ' ..
            ':callback "LUA(on_field_changed, %s)")',
            id, esc(field.default), esc(field.title), row, id)
    elseif field.type == 'option' then
        parts[#parts + 1] = string.format(
            '(WComboBox :id "%s" :gridx 1 :gridy %d :weightx 1 :weighty 0 :fill 3 ' ..
            ':placeholder "Select..." :linesHeight 8 ' ..
            ':callback "LUA(on_field_changed, %s)")',
            id, row, id)
    elseif field.type == 'switch' then
        parts[#parts + 1] = string.format(
            '(grid :id "%s_box" :gridx 1 :gridy %d :weightx 1 :weighty 0 :fill 3 :borderWidth 0\n' ..
            '        (WToggleSwitch :id "%s" :gridx 0 :gridy 0 :weightx 0 :weighty 0 :fill 3 ' ..
            ':checked FALSE :callback "LUA(on_field_changed, %s)"))',
            id, row, id, id)
    elseif field.type == 'sql' then
        parts[#parts + 1] = string.format(
            '(WComboBoxEdit :id "%s" :gridx 1 :gridy %d :weightx 1 :weighty 0 :fill 3 ' ..
            ':placeholder "Loading SQL..." :linesHeight 8 ' ..
            ':callback "LUA(on_field_changed, %s)" ' ..
            ':changed "LUA(on_field_changed, %s)")',
            id, row, id, id)
    end

    return table.concat(parts, '\n      ')
end

function M.build_source()
    local rows = {}
    local row = 0
    for _, field in ipairs(M.cfg.fields) do
        if not field.hidden then
            rows[#rows + 1] = field_fragment(field, row)
            row = row + 1
        end
    end

    return string.format([[
(window :id "admpanel" :title "%s" :width 860 :height 700
  (grid :id "root" :weightx 1 :weighty 1 :defaultDistance 8 :borderWidth 0
    (label :id "hdr" :label "%s" :gridx 0 :gridy 0 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :fontSize 16)
    (grid :id "form" :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :defaultDistance 6 :borderWidth 0
      %s
    )
    (label :id "cmd_lbl" :label "Command" :gridx 0 :gridy 2 :gridWidth 2 :weightx 1 :weighty 0 :fill 3)
    (WLineEdit :id "cmd_preview" :label "" :gridx 0 :gridy 3 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :showClear FALSE)
    (WListView :id "log" :gridx 0 :gridy 4 :gridWidth 2 :weightx 1 :weighty 1 :fill 3 :columnCount 1 :showHeader 0 :visibleRows 12)
    (grid :id "actions" :gridx 0 :gridy 5 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :borderWidth 0
      (Wbutton :id "btn_exec" :label "Execute" :gridx 0 :gridy 0 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_execute)")
      (Wbutton :id "btn_abort" :label "Abort" :gridx 1 :gridy 0 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_abort)")
      (Wbutton :id "btn_quit" :label "Quit" :gridx 2 :gridy 0 :weightx 1 :weighty 0 :fill 3 :callback "LUA(on_quit)")
    )
    (label :id "status" :label "Ready" :gridx 0 :gridy 6 :gridWidth 2 :weightx 1 :weighty 0 :fill 3 :useSmallFont TRUE)
  ))
]], esc(M.cfg.title), esc(M.cfg.title), table.concat(rows, '\n      '))
end

function M.collect_values()
    local values = {}
    for _, field in ipairs(M.cfg.fields) do
        local value
        if field.hidden then
            value = field.default
        elseif field.type == 'input' then
            value = gui.get(wid(field), 'label') or ''
        elseif field.type == 'option' then
            local idx = tonumber(gui.get(wid(field), 'selected')) or -1
            value = (idx >= 0 and field.values[idx + 1]) or field.default
        elseif field.type == 'switch' then
            local checked = gui.get(wid(field), 'checked')
            value = checked and field.on or field.off
        elseif field.type == 'sql' then
            -- WComboBoxEdit returns the parallel key for a selected item, or
            -- the edit text for free text. Resolve a display string to its key
            -- as a convenience; otherwise pass the text through as the key.
            local map = M.sql_maps[field.variable]
            local key = tostring(gui.get(wid(field), 'selectedKey') or '')
            value = nil
            if key ~= '' then
                if map then
                    for i = 1, #map do
                        local entry = map[i]
                        if entry.key == key or entry.display == key then
                            key = entry.key
                            break
                        end
                    end
                end
                value = key
            end
            value = value or field.default
        end
        values[field.variable] = value
    end
    M.values = values
    M.store = values
    return values
end

function M.refresh_preview()
    local values = M.collect_values()
    local missing = {}
    local seen = {}
    local function add_missing(name)
        if name and not seen[name] then
            seen[name] = true
            missing[#missing + 1] = name
        end
    end

    for _, field in ipairs(M.cfg.fields) do
        if field.mandatory then
            local v = values[field.variable]
            if v == nil or tostring(v) == '' then add_missing(field.variable) end
        end
    end

    local text
    if M.cfg.command_mode == 'args' then
        text = scriptmeta.build_command(M.cfg, values)
    else
        local expand_missing
        text, expand_missing = vars.expand(M.cfg.script, values,
                                           { quote_values = true })
        for _, name in ipairs(expand_missing or {}) do add_missing(name) end
    end
    if #missing > 0 then
        M.command = nil
        gui.set('cmd_preview', 'label',
            '(missing: ' .. table.concat(missing, ', ') .. ')')
        set_status('Missing: ' .. table.concat(missing, ', '))
    else
        M.command = text
        gui.set('cmd_preview', 'label', text)
        set_status('Ready')
    end
    return text, missing
end

function M.on_field_changed(id)
    M.refresh_preview()
end

function M.is_running()
    return M.job ~= nil and script.is_running(M.job)
end

function M.on_execute()
    if M.is_running() then
        set_status('A script is already running')
        return
    end

    local text, missing = M.refresh_preview()
    if #missing > 0 then
        M.add_log('Cannot execute: missing ' .. table.concat(missing, ', '))
        set_status('Missing: ' .. table.concat(missing, ', '))
        return
    end

    local argv, err
    if M.cfg.shell then
        argv = text
    else
        argv, err = vars.tokenize(text)
        if not argv then
            M.add_log('Cannot parse command: ' .. tostring(err))
            set_status('Cannot parse command: ' .. tostring(err))
            return
        end
    end

    local job, jerr = script.start_argv(argv, {
        on_event = 'on_script_event',
        cwd = M.cfg.cwd,
    })
    if not job then
        M.add_log('Cannot start script: ' .. tostring(jerr))
        set_status('Cannot start script: ' .. tostring(jerr))
        return
    end

    M.job = job
    M.add_log('$ ' .. text)
    gui.set('btn_exec', 'sensitive', false)
    gui.set('btn_abort', 'sensitive', true)
    set_status('Running...')
end

function M.handle_prompt(parsed)
    if M.prompt_active then
        M.prompt_queue[#M.prompt_queue + 1] = parsed
        return
    end

    M.prompt_active = true
    local letter = dialog.ask(parsed)
    M.prompt_active = false

    if letter and M.job and script.is_running(M.job) then
        local ok, err = script.write(M.job, letter .. '\n')
        if ok then
            M.add_log('[answer ' .. letter .. ']')
        else
            M.add_log('[answer error] ' .. tostring(err))
        end
    end

    local next_prompt = table.remove(M.prompt_queue, 1)
    if next_prompt then M.handle_prompt(next_prompt) end
end

function M.on_script_event(ev)
    if not ev then return end
    if ev.kind == 'line' or ev.kind == 'partial' then
        local text = ev.text or ''
        M.add_log(text)
        local parsed = prompt.parse_confirmation(text)
        if parsed then M.handle_prompt(parsed) end
    elseif ev.kind == 'error' then
        M.add_log('[error] ' .. tostring(ev.text or ''))
        set_status('Script error: ' .. tostring(ev.text or ''))
    elseif ev.kind == 'exit' then
        local code = tonumber(ev.code) or -1
        M.add_log(string.format('[exit %d]', code))
        set_status('Finished with exit code ' .. tostring(code))
        M.job = nil
        M.prompt_active = false
        M.prompt_queue = {}
        gui.set('btn_exec', 'sensitive', true)
        gui.set('btn_abort', 'sensitive', false)
    end
end

function M.on_abort()
    if M.job and script.is_running(M.job) then
        script.stop(M.job)
        set_status('Stopping...')
    end
end

function M.on_quit()
    if M.job and script.is_running(M.job) then
        script.stop(M.job)
    end
    xtappexitflag()
end

function M.load_sql_field(field)
    local items, map, err = sql.load_field(field, M.cfg.sql, M.values)
    if not items then
        M.add_log('[sql] ' .. tostring(err))
        set_status('SQL error: ' .. tostring(err))
        gui.set(wid(field), 'placeholder', 'SQL error')
        return
    end

    M.sql_maps[field.variable] = map
    gui.set_string_list(wid(field), 'items', items)
    local keys = {}
    for i = 1, #map do keys[i] = map[i].key end
    gui.set_string_list(wid(field), 'keys', keys)
    if #items > 0 then
        gui.set(wid(field), 'placeholder', 'Type to filter...')
        local w = gui.get_widget(wid(field))
        if w then xtaction(w, 'select_index', 0) end
    else
        gui.set(wid(field), 'placeholder', 'No rows')
    end
    M.refresh_preview()
end

function M.post_build()
    for _, field in ipairs(M.cfg.fields) do
        if field.hidden then
            M.store[field.variable] = field.default
        elseif field.type == 'option' then
            gui.set_string_list(wid(field), 'items', field.values)
            local idx = -1
            for i, value in ipairs(field.values) do
                if value == field.default then idx = i - 1; break end
            end
            if idx >= 0 then
                local w = gui.get_widget(wid(field))
                if w then xtaction(w, 'select_index', idx) end
            end
        elseif field.type == 'switch' then
            local on = field.default == field.on and field.default ~= ''
            gui.set(wid(field), 'checked', on)
        end
    end

    -- Focus the first editable field and select its initial value, so the
    -- first typed character replaces it.
    for _, field in ipairs(M.cfg.fields) do
        if not field.hidden and
           (field.type == 'sql' or field.type == 'input' or field.type == 'option') then
            local w = gui.get_widget(wid(field))
            if w then
                xtaction(w, 'focus')
                if field.type == 'input' then xtaction(w, 'select_all') end
            end
            break
        end
    end

    gui.set('btn_abort', 'sensitive', false)
    M.refresh_preview()

    for _, field in ipairs(M.cfg.fields) do
        if field.type == 'sql' and not field.hidden then
            xtapptimeout(0, function() M.load_sql_field(field) end)
        end
    end
end

function M.init(cfg)
    M.cfg = cfg
    M.store = {}
    M.values = {}
    M.fields_by_id = {}
    M.log_rows = {}
    M.sql_maps = {}
    M.job = nil
    M.prompt_active = false
    M.prompt_queue = {}
    for _, field in ipairs(cfg.fields) do
        if not field.hidden then
            M.fields_by_id[wid(field)] = field
        end
    end
end

function M.run()
    local source = M.build_source()
    local ok, err = lui.run(source)
    if not ok then
        error('AdmPanel UI error: ' .. tostring(err))
    end
    M.post_build()
end

return M
