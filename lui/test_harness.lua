local M = {}

local trace_log = {}
local expectations = {}
local capturing = false
local trace_level = 50

function M.set_trace_level(level)
    trace_level = level or 50
end

function M.start_capture()
    trace_log = {}
    capturing = true
end

function M.stop_capture()
    capturing = false
end

function M.log_trace(widget, action, args)
    if capturing then
        table.insert(trace_log, {
            widget = widget or "",
            action = action or "",
            args = args or ""
        })
    end
end

function M.get_log()
    return trace_log
end

function M.clear_log()
    trace_log = {}
end

function M.expect(widget, action, args)
    table.insert(expectations, {
        widget = widget,
        action = action,
        args = args or ""
    })
end

function M.click(widget_id)
    local w = M.get_widget(widget_id)
    if w then
        M.log_trace(widget_id, "set", "")
        xtaction(w, "set", nil, nil, 0)          -- Command widget: set flag
        M.log_trace(widget_id, "notify", "")
        xtaction(w, "notify", nil, nil, 0)       -- Invokes callbacks (guarded by set flag)
        M.log_trace(widget_id, "highlight", "")
        xtaction(w, "highlight", nil, nil, 0)    -- Visual highlight (for non-Command widgets)
        M.log_trace(widget_id, "unset", "")
        xtaction(w, "unset", nil, nil, 0)        -- Command widget: clear flag
        return true
    end
    return false
end

function M.toggle(widget_id, state)
    local w = M.get_widget(widget_id)
    if w then
        if state then
            M.log_trace(widget_id, "set", "")
            xtaction(w, "set", nil, nil, 0)
        else
            M.log_trace(widget_id, "reset", "")
            xtaction(w, "reset", nil, nil, 0)
        end
        M.log_trace(widget_id, "notify", "")
        xtaction(w, "notify", nil, nil, 0)
        return true
    end
    return false
end

function M.focus_in(widget_id)
    local w = M.get_widget(widget_id)
    if w then
        M.log_trace(widget_id, "focus_in", "")
        xtaction(w, "focus_in", nil, nil, 0)
        return true
    end
    return false
end

function M.focus_out(widget_id)
    local w = M.get_widget(widget_id)
    if w then
        M.log_trace(widget_id, "focus_out", "")
        xtaction(w, "focus_out", nil, nil, 0)
        return true
    end
    return false
end

function M.action(widget_id, action_name, ...)
    local w = M.get_widget(widget_id)
    if w then
        M.log_trace(widget_id, action_name, "")
        xtaction(w, action_name, ...)
        return true
    end
    return false
end

function M.get_widget(widget_id)
    local backend = require('backend_xt')
    return backend.get_widget(widget_id)
end

function M.verify()
    local errors = {}
    local matched = 0
    local log_idx = 1

    for _, exp in ipairs(expectations) do
        local found = false
        while log_idx <= #trace_log do
            local entry = trace_log[log_idx]
            log_idx = log_idx + 1
            if entry.widget == exp.widget and entry.action == exp.action then
                if exp.args == "" or entry.args == exp.args then
                    found = true
                    matched = matched + 1
                    break
                end
            end
        end
        if not found then
            table.insert(errors, string.format(
                "FAIL: expected %s.%s(%s) not found in trace log",
                exp.widget, exp.action, exp.args
            ))
        end
    end

    expectations = {}
    return #errors == 0, errors, matched
end

function M.verify_exact()
    local errors = {}
    if #trace_log ~= #expectations then
        table.insert(errors, string.format(
            "FAIL: log has %d entries, expected %d",
            #trace_log, #expectations
        ))
    end

    local count = math.min(#trace_log, #expectations)
    for i = 1, count do
        local exp = expectations[i]
        local entry = trace_log[i]
        if entry.widget ~= exp.widget or entry.action ~= exp.action then
            table.insert(errors, string.format(
                "FAIL: entry %d: got %s.%s, expected %s.%s",
                i, entry.widget, entry.action, exp.widget, exp.action
            ))
        end
    end

    expectations = {}
    return #errors == 0, errors
end

function M.timeout(ms, fn)
    xtapptimeout(ms, fn)
end

function M.exit(code)
    xtappexitflag()
end

function M.sequence(steps)
    local idx = 1

    local function run_step()
        if idx > #steps then
            M.exit(0)
            return
        end

        local step = steps[idx]
        idx = idx + 1

        if step.action == "capture_start" then
            M.start_capture()
            xtapptimeout(50, run_step)
        elseif step.action == "click" then
            M.click(step.widget)
            xtapptimeout(50, run_step)
        elseif step.action == "toggle" then
            M.toggle(step.widget, step.state)
            xtapptimeout(50, run_step)
        elseif step.action == "focus_in" then
            M.focus_in(step.widget)
            xtapptimeout(50, run_step)
        elseif step.action == "focus_out" then
            M.focus_out(step.widget)
            xtapptimeout(50, run_step)
        elseif step.action == "action" then
            M.action(step.widget, step.name, unpack(step.args or {}))
            xtapptimeout(50, run_step)
        elseif step.action == "expect" then
            M.expect(step.widget, step.event, step.args)
            xtapptimeout(0, run_step)
        elseif step.action == "verify" then
            local pass, errors = M.verify()
            if pass then
                print("PASS")
                M.exit(0)
            else
                print("FAIL:")
                for _, e in ipairs(errors) do print("  " .. e) end
                M.exit(1)
            end
        elseif step.action == "verify_exact" then
            local pass, errors = M.verify_exact()
            if pass then
                print("PASS")
                M.exit(0)
            else
                print("FAIL:")
                for _, e in ipairs(errors) do print("  " .. e) end
                M.exit(1)
            end
        elseif step.action == "wait" then
            xtapptimeout(step.ms or 100, run_step)
        elseif step.action == "set" then
            local gui = require('gui_xt')
            gui.set(step.widget, step.property, step.value)
            xtapptimeout(50, run_step)
        elseif step.action == "capture_stop" then
            M.stop_capture()
            xtapptimeout(0, run_step)
        else
            print("Unknown step action: " .. tostring(step.action))
            M.exit(1)
        end
    end

    xtapptimeout(500, run_step)
end

return M