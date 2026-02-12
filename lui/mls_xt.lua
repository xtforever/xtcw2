local M = {}

-- These will be bound to C functions in luarunner.c
-- We use a bridge since we can't easily add to luaxt.i right now

function M.create(size, width)
    return mls_create(size, width)
end

function M.put_string(handle, s)
    mls_put_string(handle, s)
end

function M.clear(handle)
    mls_clear(handle)
end

function M.len(handle)
    return mls_len(handle)
end

return M
