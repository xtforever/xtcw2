local lui = require('lui')

_G.quit_cb = function()
    print("Exiting File Manager")
    os.exit(0)
end

local current_path = '.'

local function list_files(path)
    local files = {}
    local p = io.popen('ls -p ' .. path)
    if not p then return files end
    
    -- Add parent directory if not at root
    if path ~= '/' and path ~= '.' then
        table.insert(files, {"..", "Directory", ""})
    end

    for line in p:lines() do
        local is_dir = line:sub(-1) == '/'
        local name = is_dir and line:sub(1, -2) or line
        local type = is_dir and "Directory" or "File"
        local size = "" 
        table.insert(files, {name, type, size})
    end
    p:close()
    return files
end

local sidebar_store = gui.create_store(1)
gui.store_append(sidebar_store, {"Home"})
gui.store_append(sidebar_store, {"Documents"})
gui.store_append(sidebar_store, {"Downloads"})

local files_store = gui.create_store(3)

local function refresh_files()
    files_store:clear()
    local files = list_files(current_path)
    for _, f in ipairs(files) do
        gui.store_append(files_store, f)
    end
    gui.update('files')
    gui.set('status_label', 'label', ' Path: ' .. current_path)
end

refresh_files()

function files_cb(data)
    local lineno = tonumber(data)
    if not lineno then return end
    
    local row = gui.store_get(files_store, lineno)
    if not row then return end
    
    local name = row[1]
    local type = row[2]
    
    if type == "Directory" then
        if name == ".." then
            current_path = current_path:gsub("/[^/]+/?$", "")
            if current_path == "" then current_path = "/" end
        else
            if current_path == "/" then
                current_path = "/" .. name
            else
                current_path = current_path .. "/" .. name
            end
        end
        refresh_files()
    else
        print("Selected file:", name)
    end
end

local lui_source = [[(window :title "Lui File Manager" :width 800 :height 600
  (grid
    (splitter :id "main_split" :orientation "vertical" :fraction 0.25
              :gridx 0 :gridy 0 :weightx 100 :weighty 100 :fill 3
       (list-view :id "sidebar" :model $sidebar_placeholder :fontSize 24)
       (list-view :id "files" :model $files_placeholder 
                  :columns (300 150 100) :retexCells true :fontSize 24
                  :on-row-activated "files_cb"))
    (grid :gridx 0 :gridy 1 :fill 1 :weighty 0
       (button :label "Quit" :gridx 0 :gridy 0 :callback "quit_cb")
       (label :id "status_label" :label " Status: Ready" :gridx 1 :gridy 0 :weightx 100 :align 1))))]]

lui_source = lui_source:gsub("$sidebar_placeholder", tostring(sidebar_store.handle))
lui_source = lui_source:gsub("$files_placeholder", tostring(files_store.handle))

lui.run(lui_source)
lui.loop()
