-- IconEditor — a small XPM icon editor built on DrawCanvas.
--
-- Left: a pixel grid that scales with the window.  Right: a live preview,
-- the tool palette and the icon size.  File → Open uses the WfileSelector /
-- WfileChooser; Save writes a plain XPM file.

local gui = require('gui_xt')
local lui = require('lui')

local canvas, preview, fs, save_dlg, quit_dlg
local dirty = false
local save_path = ""
local current_color = "#000000"
local current_tool = "pencil"
local pending_quit = false

local function set_status(msg)
    gui.set("status", "message", msg)
end

local function tool_name(t)
    if t == "line" then return "Line" end
    if t == "rect" then return "Rect" end
    if t == "ellipse" then return "Circle" end
    if t == "fill" then return "Fill" end
    return "Pencil"
end

local function xpm_size(path)
    local f = io.open(path, "rb")
    if not f then return nil end
    local data = f:read("*a") or ""
    f:close()
    local w, h = data:match('"(%d+)%s+(%d+)%s+%d+%s+%d+"')
    return tonumber(w), tonumber(h)
end

local function do_save(path)
    if not path or path == "" then return false end
    if canvas then xtaction(canvas, "iconeditor_save", path) end
    local f = io.open(path, "rb")
    if not f then
        set_status("Save failed: " .. path)
        return false
    end
    f:close()
    save_path = path
    dirty = false
    set_status("Saved " .. path)
    return true
end

function on_tool(t)
    current_tool = t
    gui.set("tool_lbl", "label", "Tool: " .. tool_name(t))
    if canvas then xtaction(canvas, "iconeditor_set_tool", t) end
    set_status("Tool: " .. tool_name(t))
end

function on_color(hex)
    local c = "#" .. hex
    current_color = c
    if canvas then xtaction(canvas, "iconeditor_set_color", c) end
    set_status("Color: " .. c)
end

function on_undo()
    if canvas then xtaction(canvas, "iconeditor_undo") end
    dirty = true
    set_status("Undo")
end

function on_clear()
    if canvas then xtaction(canvas, "iconeditor_clear") end
    dirty = true
    set_status("Cleared")
end

function on_resize()
    local w = tonumber(gui.get("sp_w", "value")) or 32
    local h = tonumber(gui.get("sp_h", "value")) or 32
    if canvas then xtaction(canvas, "iconeditor_resize", w, h) end
    dirty = true
    set_status(string.format("Size: %dx%d (cleared)", w, h))
end

function cb_new()
    if canvas then xtaction(canvas, "iconeditor_clear") end
    save_path = ""
    dirty = true
    set_status("New icon")
end

function cb_open()
    if fs then xtaction(fs, "popup_files") end
end

function on_open_selected()
    local path = gui.get("fs", "selectedPath") or ""
    if path == "" then return end
    if canvas then xtaction(canvas, "iconeditor_load", path) end
    local w, h = xpm_size(path)
    if w and h then
        gui.set("sp_w", "value", w)
        gui.set("sp_h", "value", h)
    end
    save_path = path
    dirty = false
    set_status("Opened " .. path)
end

function cb_save()
    if save_path ~= "" then
        do_save(save_path)
    else
        cb_save_as()
    end
end

function cb_save_as()
    gui.set("save_dlg", "inputValue", save_path ~= "" and save_path or "icon.xpm")
    gui.set("save_dlg", "open", true)
end

function on_save_result(ok)
    ok = tonumber(ok) or tonumber(gui.get("save_dlg", "result")) or 0
    if ok == 1 then
        local path = gui.get("save_dlg", "inputValue") or ""
        if do_save(path) and pending_quit then xtappexitflag() end
    end
    pending_quit = false
end

function cb_quit()
    if not dirty then
        xtappexitflag()
        return
    end
    gui.set("quit_dlg", "open", true)
end

function on_quit_result(ok)
    ok = tonumber(ok) or tonumber(gui.get("quit_dlg", "result")) or 0
    if ok == 1 then
        if save_path ~= "" then
            if do_save(save_path) then xtappexitflag() end
        else
            pending_quit = true
            cb_save_as()
        end
    elseif ok == 0 then
        xtappexitflag()
    else
        set_status("Quit cancelled")
    end
end

function cb_about()
    set_status("IconEditor — draw an icon, preview it, save as XPM")
end

local ui = [[
(window :id "win" :title "IconEditor — XPM" :width 920 :height 820
  (app-window :id "app" :windowMargin 10 :rowGap 6
              :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3
    (menu-bar :id "menubar" :slot "menu" :barHeight 28
      (menu-button :id "m_file" :label "File" :buttonWidth 64 :buttonHeight 28
        (popup-menu :id "m_file_popup"
          (button :id "mi_new" :label "New" :callback "LUA(cb_new)")
          (button :id "mi_open" :label "Open…" :callback "LUA(cb_open)")
          (button :id "mi_save" :label "Save" :callback "LUA(cb_save)")
          (button :id "mi_save_as" :label "Save As…" :callback "LUA(cb_save_as)")
          (button :id "mi_quit" :label "Quit" :callback "LUA(cb_quit)")))
      (menu-button :id "m_edit" :label "Edit" :buttonWidth 64 :buttonHeight 28
        (popup-menu :id "m_edit_popup"
          (action :id "mi_undo" :label "Undo" :shortcut "Ctrl+Z"
                  :callback "LUA(on_undo)")
          (button :id "mi_clear" :label "Clear" :callback "LUA(on_clear)")))
      (menu-button :id "m_help" :label "Help" :buttonWidth 64 :buttonHeight 28
        (popup-menu :id "m_help_popup"
          (button :id "mi_about" :label "About" :callback "LUA(cb_about)"))))

    (tool-bar :id "toolbar" :slot "toolbar" :toolbarHeight 42
              :itemGap 6 :toolbarPadding 4
      (button :id "tb_new" :label "New" :tooltip "New icon" :callback "LUA(cb_new)")
      (file-selector :id "fs" :label "Open…" :pattern "*.xpm" :directory "."
                     :tooltip "Open an XPM icon" :callback "LUA(on_open_selected)")
      (button :id "tb_save" :label "Save" :tooltip "Save XPM" :callback "LUA(cb_save)")
      (button :id "tb_undo" :label "Undo" :tooltip "Undo the last edit"
              :callback "LUA(on_undo)"))

    (grid :id "content" :slot "content" :weightx 1 :weighty 1 :fill 3
          :defaultDistance 8
      (DrawCanvas :id "canvas" :gridx 0 :gridy 0 :weightx 1 :weighty 1 :fill 3
                  :xftFont "Sans-11" :bg "#303030" :foreground "#ffffff"
                  :callback "iconeditor_draw_cb"
                  :mouseCallback "iconeditor_mouse_cb"
                  :keyCallback "iconeditor_key_cb"
                  :updateInterval 20 :doubleBuffer TRUE :takeFocus TRUE)

      (grid :id "side" :gridx 1 :gridy 0 :weightx 0 :weighty 1 :fill 3
            :width 260 :defaultDistance 4
        (label :id "preview_lbl" :label "Preview" :gridx 0 :gridy 0
               :weightx 1 :weighty 0 :fill 3 :fontSize 12 :topGap 1 :bottomGap 1)
        (DrawCanvas :id "preview" :gridx 0 :gridy 1 :width 120 :height 120
                    :bg "#202020" :foreground "#ffffff"
                    :callback "iconeditor_preview_cb"
                    :updateInterval 20 :doubleBuffer TRUE)

        (label :id "tool_lbl" :label "Tool: Pencil" :gridx 0 :gridy 2
               :weightx 1 :weighty 0 :fill 3 :fontSize 12 :topGap 1 :bottomGap 1)
        (grid :id "tools" :gridx 0 :gridy 3 :weightx 1 :weighty 0 :fill 3
              :defaultDistance 4
          (button :id "tool_pencil" :label "Pencil" :gridx 0 :gridy 0
                  :weightx 1 :fill 3 :callback "LUA(on_tool, pencil)")
          (button :id "tool_line" :label "Line" :gridx 1 :gridy 0
                  :weightx 1 :fill 3 :callback "LUA(on_tool, line)")
          (button :id "tool_rect" :label "Rect" :gridx 0 :gridy 1
                  :weightx 1 :fill 3 :callback "LUA(on_tool, rect)")
          (button :id "tool_ellipse" :label "Circle" :gridx 1 :gridy 1
                  :weightx 1 :fill 3 :callback "LUA(on_tool, ellipse)")
          (button :id "tool_fill" :label "Fill" :gridx 0 :gridy 2
                  :weightx 1 :fill 3 :callback "LUA(on_tool, fill)")
          (button :id "tool_undo" :label "Undo" :gridx 1 :gridy 2
                  :weightx 1 :fill 3 :callback "LUA(on_undo)"))

        (label :id "color_lbl" :label "Color" :gridx 0 :gridy 4
               :weightx 1 :weighty 0 :fill 3 :fontSize 12 :topGap 1 :bottomGap 1)
        (DrawCanvas :id "swatch" :width 60 :height 28
                    :gridx 0 :gridy 5 :weightx 0 :weighty 0 :fill 0
                    :bg "#202020" :callback "iconeditor_swatch_cb"
                    :updateInterval 20 :doubleBuffer TRUE)
        (grid :id "colors" :gridx 0 :gridy 6 :weightx 1 :weighty 0 :fill 3
              :defaultDistance 4
          (button :id "col_black" :label "Black" :gridx 0 :gridy 0
                  :weightx 1 :fill 3 :callback "LUA(on_color, 000000)")
          (button :id "col_white" :label "White" :gridx 1 :gridy 0
                  :weightx 1 :fill 3 :callback "LUA(on_color, ffffff)")
          (button :id "col_red" :label "Red" :gridx 0 :gridy 1
                  :weightx 1 :fill 3 :callback "LUA(on_color, ff0000)")
          (button :id "col_green" :label "Green" :gridx 1 :gridy 1
                  :weightx 1 :fill 3 :callback "LUA(on_color, 00aa00)")
          (button :id "col_blue" :label "Blue" :gridx 0 :gridy 2
                  :weightx 1 :fill 3 :callback "LUA(on_color, 0000ff)")
          (button :id "col_yellow" :label "Yellow" :gridx 1 :gridy 2
                  :weightx 1 :fill 3 :callback "LUA(on_color, ffff00)")
          (button :id "col_magenta" :label "Magenta" :gridx 0 :gridy 3
                  :weightx 1 :fill 3 :callback "LUA(on_color, ff00ff)")
          (button :id "col_cyan" :label "Cyan" :gridx 1 :gridy 3
                  :weightx 1 :fill 3 :callback "LUA(on_color, 00ffff)"))

        (label :id "size_lbl" :label "Size" :gridx 0 :gridy 7
               :weightx 1 :weighty 0 :fill 3 :fontSize 12 :topGap 1 :bottomGap 1)
        (grid :id "size_row" :gridx 0 :gridy 8 :weightx 1 :weighty 0 :fill 3
              :defaultDistance 4
          (spinbox :id "sp_w" :value 32 :min 4 :max 64 :step 1
                   :gridx 0 :gridy 0 :weightx 1 :fill 3)
          (spinbox :id "sp_h" :value 32 :min 4 :max 64 :step 1
                   :gridx 1 :gridy 0 :weightx 1 :fill 3)
          (button :id "resize_btn" :label "Resize" :gridx 2 :gridy 0
                  :weightx 1 :fill 3 :callback "LUA(on_resize)"))
      ))

    (status-bar :id "status" :slot "status" :message "Ready — draw with the selected tool"
                :itemGap 8 :barPadding 6)
  )

  (dialog :id "save_dlg" :managed FALSE :title "Save XPM"
          :message "File name:" :inputKind 1 :inputPlaceholder "icon.xpm"
          :okLabel "Save" :cancelLabel "Cancel"
          :callback "LUA(on_save_result)")
  (dialog :id "quit_dlg" :managed FALSE :title "Unsaved changes"
          :message "Save the icon before quitting?"
          :okLabel "Save" :cancelLabel "Don't save"
          :auxLabel "Cancel" :showAux TRUE
          :callback "LUA(on_quit_result)")
)
]]

local ok, err = lui.run(ui)
if not ok then error("demo_iconeditor: " .. tostring(err)) end

canvas = gui.get_widget("canvas")
preview = gui.get_widget("preview")
fs = gui.get_widget("fs")
save_dlg = gui.get_widget("save_dlg")
quit_dlg = gui.get_widget("quit_dlg")

if canvas then
    xtaction(canvas, "iconeditor_set_tool", "pencil")
end
on_color("000000")
set_status("Ready — draw with the selected tool")

local last_change = 0
local function poll_canvas()
    local n = tonumber(gui.get("canvas", "changeCount")) or 0
    if n ~= last_change then
        last_change = n
        dirty = true
        set_status("Modified — " .. tool_name(current_tool) .. " " .. current_color)
    end
    xtapptimeout(100, poll_canvas)
end
xtapptimeout(100, poll_canvas)

local smoke = os.getenv("ICONEDITOR_SMOKE")
if smoke and smoke ~= "" then
    xtapptimeout(400, function()
        if canvas then
            xtaction(canvas, "iconeditor_set_color", "#ff0000")
            xtaction(canvas, "iconeditor_test_stroke")
        end
        do_save(smoke)
        xtappexitflag()
    end)
end
