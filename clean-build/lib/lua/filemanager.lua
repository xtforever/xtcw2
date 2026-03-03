
local lui = require('lui')

-- State
local panes = {
    left = { path = os.getenv("HOME") or ".", selection = nil },
    right = { path = "/tmp", selection = nil }
}
local active_pane = "left"

-- Helper: List files
local function list_files(path)
    local files = {}
    local p = io.popen('ls -p ' .. path)
    if not p then return files end
    
    table.insert(files, {"..", "Directory", ""})
    for line in p:lines() do
        local is_dir = line:sub(-1) == '/'
        local name = is_dir and line:sub(1, -2) or line
        local type = is_dir and "Directory" or "File"
        table.insert(files, {name, type, ""})
    end
    p:close()
    return files
end

-- Refresh a specific pane
local function refresh_pane(side)
    local p = panes[side]
    local store = (side == "left") and left_store or right_store
    local widget_id = side .. "_files"
    
    store:clear()
    local files = list_files(p.path)
    for _, f in ipairs(files) do
        gui.store_append(store, f)
    end
    gui.update(widget_id)
    gui.set(side .. "_path_label", "label", " " .. p.path)
end

-- Task Event Handler (Global for C-side)
function on_task_event(data)
    -- data format: "job_id,type,progress,message"
    local job_id, event_type, progress, msg = data:match("([^,]+),([^,]+),([^,]+),(.+)")
    if not job_id then return end
    
    progress = tonumber(progress)
    event_type = tonumber(event_type) -- 0: Progress, 1: Complete, 2: Error

    gui.set("status_label", "label", string.format(" [Job %s] %.0f%% - %s", job_id, progress, msg))
    
    if event_type == 1 or event_type == 2 then
        refresh_pane("left")
        refresh_pane("right")
    end
end

-- Callbacks
function on_left_click(data)
    active_pane = "left"
    local row = gui.store_get(left_store, tonumber(data))
    if row then
        panes.left.selection = row[1]
        if row[2] == "Directory" then
            if row[1] == ".." then
                panes.left.path = panes.left.path:gsub("/[^/]+/?$", "")
                if panes.left.path == "" then panes.left.path = "/" end
            else
                panes.left.path = panes.left.path .. "/" .. row[1]
            end
            refresh_pane("left")
        end
    end
end

function on_right_click(data)
    active_pane = "right"
    local row = gui.store_get(right_store, tonumber(data))
    if row then
        panes.right.selection = row[1]
        if row[2] == "Directory" then
            if row[1] == ".." then
                panes.right.path = panes.right.path:gsub("/[^/]+/?$", "")
                if panes.right.path == "" then panes.right.path = "/" end
            else
                panes.right.path = panes.right.path .. "/" .. row[1]
            end
            refresh_pane("right")
        end
    end
end

function on_copy()
    local src_side = active_pane
    local dst_side = (active_pane == "left") and "right" or "left"
    
    local src_file = panes[src_side].selection
    if not src_file or src_file == ".." then
        gui.set("status_label", "label", " Error: No file selected to copy")
        return
    end
    
    local src_full = panes[src_side].path .. "/" .. src_file
    local dst_full = panes[dst_side].path .. "/" .. src_file
    
    gui.set("status_label", "label", " Spawning copy task...")
    lui.copy_file(src_full, dst_full)
end

-- UI Setup
left_store = gui.create_store(3)
right_store = gui.create_store(3)

local lui_source = [[(window :title "Lui Dual-Pane File Manager" :width 1000 :height 700
  (grid
    ; Resizable Panes
    (splitter :id "main_split" :orientation "vertical" :fraction 0.5
              :gridx 0 :gridy 0 :weightx 100 :weighty 100 :fill 3
       ; Left Pane
       (grid
          (label :id "left_path_label" :label " /" :gridx 0 :gridy 0 :weightx 100 :bg "#34495e" :height 30)
          (list-view :id "left_files" :model $left_placeholder 
                     :columns (250 100 50) :fontSize 20 :gridx 0 :gridy 1 :weighty 100 :fill 3
                     :on-row-activated "on_left_click"))
       ; Right Pane
       (grid
          (label :id "right_path_label" :label " /" :gridx 0 :gridy 0 :weightx 100 :bg "#34495e" :height 30)
          (list-view :id "right_files" :model $right_placeholder 
                     :columns (250 100 50) :fontSize 20 :gridx 0 :gridy 1 :weighty 100 :fill 3
                     :on-row-activated "on_right_click")))
    
    ; Toolbar
    (grid :gridx 0 :gridy 1 :fill 1 :weighty 0 :bg "#2c3e50"
       (button :label "Copy (F5)" :gridx 0 :gridy 0 :callback "on_copy" :width 120)
       (button :label "Quit" :gridx 1 :gridy 0 :callback "WcQuit" :width 100)
       (label :id "status_label" :label " Status: Ready" :gridx 2 :gridy 0 :weightx 100 :align 1 :fg "#ecf0f1"))))]]

lui_source = lui_source:gsub("$left_placeholder", tostring(left_store.handle))
lui_source = lui_source:gsub("$right_placeholder", tostring(right_store.handle))

refresh_pane("left")
refresh_pane("right")

lui.run(lui_source)
lui.loop()
