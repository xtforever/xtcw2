-- Lua wrapper around the commander script_process binding.

local M = {}

function M.start(command, opts)
    opts = opts or {}
    if opts.shell then
        return script_start(command, opts)
    end
    local argv, err = require('vars').tokenize(command)
    if not argv then return nil, err end
    return script_start(argv, opts)
end

function M.start_argv(argv, opts)
    return script_start(argv, opts or {})
end

function M.write(job, text)
    return script_write(job, text)
end

function M.stop(job)
    return script_stop(job)
end

function M.is_running(job)
    return script_is_running(job)
end

return M
