-- Minimal wrapper around commander's pty script_process binding.
-- Deliberately no dependency on apps/admpanel/vars.lua or scriptmeta.lua.

local M = {}

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
