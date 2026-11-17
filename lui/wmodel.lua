-- lui/wmodel.lua
-- Lightweight list/table model contract (WModel) plus a sort/filter proxy.
--
-- The contract is deliberately small and Lua-table friendly:
--
--   local WModel = require('wmodel')
--   local m = WModel.new{
--       columns = { {title="Name"}, {title="Qty", align=2} },
--       rows    = { {"Cherry", "3"}, {"Apple", "1"} },
--       ids     = { 10, 20 },          -- optional; generated when omitted
--   }
--   local p = WModel.proxy(m)
--   p:set_filter("app")
--   p:set_sort(1, true)
--   p:rows()   --> { "Apple\t1" }
--   p:ids()    --> { 20 }
--
-- `gui.set_model()` / `gui.set_model_filter()` in gui_xt.lua push a proxy into
-- WListView/table-view. Sorting here mirrors WListView's C comparator: numeric
-- when both cells parse as numbers, case-insensitive text otherwise.

local M = {}

local Model = {}
Model.__index = Model

local Proxy = {}
Proxy.__index = Proxy

local function cell_to_string(value)
    if value == nil then return "" end
    return tostring(value)
end

local function parse_number(value)
    if type(value) == 'number' then return value end
    local s = cell_to_string(value)
    local n = tonumber(s)
    if n ~= nil and s:match('^%s*[-+]?%d+%.?%d*%s*$') then return n end
    return nil
end

local function compare_cells(a, b)
    local na, nb = parse_number(a), parse_number(b)
    if na ~= nil and nb ~= nil then
        if na < nb then return -1 end
        if na > nb then return 1 end
        return 0
    end
    local sa, sb = cell_to_string(a):lower(), cell_to_string(b):lower()
    if sa < sb then return -1 end
    if sa > sb then return 1 end
    return 0
end

local function normalize_columns(columns)
    local out = {}
    for i, col in ipairs(columns or {}) do
        if type(col) == 'table' then
            out[i] = {
                title = col.title or col.label or "",
                width = col.width or 0,
                align = col.align or 0,
                kind  = col.kind or 0,
            }
        else
            out[i] = { title = tostring(col), width = 0, align = 0, kind = 0 }
        end
    end
    return out
end

function M.new(spec)
    spec = spec or {}
    local self = setmetatable({}, Model)
    self._columns = normalize_columns(spec.columns)
    self._rows = {}
    self._ids = {}
    self._next_id = 1

    local rows = spec.rows or {}
    for i = 1, #rows do
        self:add(rows[i], spec.ids and spec.ids[i])
    end
    return self
end

function Model:column_count()
    return #self._columns
end

function Model:columns()
    return self._columns
end

function Model:count()
    return #self._rows
end

function Model:get(row, col)
    local r = self._rows[row]
    if not r then return nil end
    return r[col]
end

function Model:set(row, col, value)
    local r = self._rows[row]
    if not r then return false end
    r[col] = value
    return true
end

function Model:add(row, id)
    if id == nil then
        id = self._next_id
        self._next_id = self._next_id + 1
    else
        local n = tonumber(id)
        if n and n >= self._next_id then self._next_id = n + 1 end
    end
    self._rows[#self._rows + 1] = row
    self._ids[#self._ids + 1] = id
    return #self._rows
end

function Model:remove(row)
    if not self._rows[row] then return false end
    table.remove(self._rows, row)
    table.remove(self._ids, row)
    return true
end

function Model:ids()
    return self._ids
end

function Model:id_at(row)
    return self._ids[row]
end

function Model:row_at(row)
    return self._rows[row]
end

function Model:index_of_id(id)
    for i = 1, #self._ids do
        if tostring(self._ids[i]) == tostring(id) then return i end
    end
    return nil
end

function Model:to_rows()
    local out = {}
    for i = 1, #self._rows do
        local cells = {}
        for c = 1, #self._rows[i] do
            cells[c] = cell_to_string(self._rows[i][c])
        end
        out[i] = table.concat(cells, "\t")
    end
    return out
end

-- Stable sort of the source rows (keeps ids attached).
function Model:sort(col, ascending)
    local asc = ascending ~= false
    local order = {}
    for i = 1, #self._rows do order[i] = i end
    table.sort(order, function(a, b)
        local cmp = compare_cells(self._rows[a][col], self._rows[b][col])
        if cmp == 0 then return a < b end
        return asc and (cmp < 0) or (not asc and cmp > 0)
    end)
    local rows, ids = {}, {}
    for i, src in ipairs(order) do
        rows[i] = self._rows[src]
        ids[i] = self._ids[src]
    end
    self._rows, self._ids = rows, ids
end

-- Return the source indices whose selected columns contain `text`
-- (case-insensitive substring). `cols` is an optional array of 1-based
-- column indices; nil means "all columns".
function Model:match(text, cols)
    local needle = (text or ""):lower()
    local out = {}
    if needle == "" then
        for i = 1, #self._rows do out[i] = i end
        return out
    end
    local columns = cols
    if not columns or #columns == 0 then
        columns = {}
        for c = 1, math.max(1, self:column_count()) do columns[c] = c end
    end
    for i = 1, #self._rows do
        local row = self._rows[i]
        for _, c in ipairs(columns) do
            local value = cell_to_string(row[c]):lower()
            if value:find(needle, 1, true) then
                out[#out + 1] = i
                break
            end
        end
    end
    return out
end

function M.proxy(model)
    local self = setmetatable({}, Proxy)
    self._model = model
    self._filter = ""
    self._filter_columns = nil
    self._sort_col = nil
    self._sort_asc = true
    self._view = nil
    return self
end

function Proxy:set_source(model)
    self._model = model
    self._view = nil
end

function Proxy:model()
    return self._model
end

function Proxy:columns()
    return self._model:columns()
end

function Proxy:set_filter(text, cols)
    self._filter = text or ""
    self._filter_columns = cols
    self._view = nil
end

function Proxy:filter_text()
    return self._filter
end

function Proxy:set_sort(col, ascending)
    if col ~= nil and col < 1 then col = nil end
    self._sort_col = col
    self._sort_asc = ascending ~= false
    self._view = nil
end

function Proxy:sort_column()
    return self._sort_col
end

function Proxy:sort_ascending()
    return self._sort_asc
end

function Proxy:_rebuild()
    if self._view then return self._view end
    local view = self._model:match(self._filter, self._filter_columns)
    if self._sort_col and self._sort_col >= 1 then
        local asc = self._sort_asc
        table.sort(view, function(a, b)
            local cmp = compare_cells(self._model:get(a, self._sort_col),
                                      self._model:get(b, self._sort_col))
            if cmp == 0 then return a < b end
            return asc and (cmp < 0) or (not asc and cmp > 0)
        end)
    end
    self._view = view
    return view
end

function Proxy:count()
    return #self:_rebuild()
end

function Proxy:source_index(view_row)
    return self:_rebuild()[view_row]
end

function Proxy:get(view_row, col)
    local src = self:source_index(view_row)
    if not src then return nil end
    return self._model:get(src, col)
end

function Proxy:rows()
    local view = self:_rebuild()
    local out = {}
    for i, src in ipairs(view) do
        local row = self._model:row_at(src)
        local cells = {}
        for c = 1, #row do cells[c] = cell_to_string(row[c]) end
        out[i] = table.concat(cells, "\t")
    end
    return out
end

function Proxy:ids()
    local view = self:_rebuild()
    local out = {}
    for i, src in ipairs(view) do out[i] = self._model:id_at(src) end
    return out
end

-- View row (1-based) whose id matches, or nil when filtered out.
function Proxy:view_of_id(id)
    local ids = self:ids()
    for i = 1, #ids do
        if tostring(ids[i]) == tostring(id) then return i end
    end
    return nil
end

return M
