-- File chooser demo: WfileSelector opens the WListView-based WfileChooser.
-- Picking a plain UTF-8 text file loads it into a Wbravo editor.
--
-- The chooser shows Name (icon + text), Size, Date and Type columns; folders
-- are listed first.  Only the text is imported, so rich formatting is reset.

local gui = require('gui_xt')
local lui = require('lui')

function on_pick()
    local path = gui.get("fs", "selectedPath") or ""
    if path == "" then return end
    local f = io.open(path, "rb")
    if not f then
        gui.set("status", "label", "Cannot open " .. path)
        return
    end
    local data = f:read("*a") or ""
    f:close()
    data = data:gsub("\r\n", "\n"):gsub("\r", "\n")
    local ed = gui.get_widget("ed")
    if ed then
        xtaction(ed, "set_text", data)
        xtaction(ed, "SetFocus")
    end
    gui.set("status", "label", "Loaded " .. path)
end

local ui = [[
(window :id "win" :title "File chooser + Wbravo" :width 820 :height 600
  (grid :id "main" :weightx 1 :weighty 1
    (file-selector :id "fs" :label "Open…" :title "Open UTF-8 Text File"
       :gridx 0 :gridy 0 :weightx 0 :weighty 0 :fill 3
       :directory "."
       :pattern "*"
       :folderIcon "../LuaRunner/folder.svg"
       :fileIcon "../LuaRunner/file.svg"
       :callback "LUA(on_pick)")
    (label :id "status" :label "Pick a text file to load it into the editor"
       :gridx 1 :gridy 0 :weightx 1 :weighty 0 :fill 3)
    (Wbravo :id "ed"
       :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1 :fill 3
       :text "Use the button above to choose a plain UTF-8 text file.\n\nBlank lines separate paragraphs."
       :fontFace "Sans" :fontSize 16)
  ))
]]

local ok, err = lui.run(ui)
if not ok then error("demo_filechooser: " .. tostring(err)) end
