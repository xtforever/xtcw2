-- WmenuButton demo: a burger-menu button that opens a tabbed popup menu.
--
-- The popup content is a WtabWidget with three pages (File / Edit / Format);
-- each page is a small grid of buttons.  The menu closes when you click
-- outside the popup, on the burger button, or when a menu item is selected.
local gui = require('gui_xt')
local lui = require('lui')

local ui = [[
(window :id "win" :title "WmenuButton — burger menu over Wbravo" :width 760 :height 520
  (grid :id "main" :weightx 1 :weighty 1
    (menu-button :id "menu"
                 :gridx 0 :gridy 0 :buttonSize 40 :fill 0
                 :popupWidth 360 :popupHeight 260
      (tabs :id "menu_tabs" :weightx 1 :weighty 1 :fill 3
        (grid :id "file" :tab-title "File"
          (button :label "New"  :gridx 0 :gridy 0 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Open" :gridx 0 :gridy 1 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Save" :gridx 0 :gridy 2 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Quit" :gridx 0 :gridy 3 :weightx 1 :fill 3 :callback "LUA(on_quit)"))
        (grid :id "edit" :tab-title "Edit"
          (button :label "Undo"  :gridx 0 :gridy 0 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Redo"  :gridx 0 :gridy 1 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Copy"  :gridx 0 :gridy 2 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Paste" :gridx 0 :gridy 3 :weightx 1 :fill 3 :callback "LUA(on_item)"))
        (grid :id "format" :tab-title "Format"
          (button :label "Bold"   :gridx 0 :gridy 0 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Italic" :gridx 0 :gridy 1 :weightx 1 :fill 3 :callback "LUA(on_item)")
          (button :label "Roman"  :gridx 0 :gridy 2 :weightx 1 :fill 3 :callback "LUA(on_item)"))))
    (label :id "hdr"
           :label "Click the burger button; close with an outside click or the button."
           :gridx 1 :gridy 0 :weightx 1 :weighty 0 :fill 3)
    (Wbravo :id "ed"
            :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1 :fill 3
            :text "Wbravo editor with a burger menu.\n\nType here; the burger menu opens above.\n"
            :fontFace "Sans" :fontSize 16
            :bg_norm "#101018" :fg_norm "#e8e8e8"
            :bg_sel "#204080" :fg_sel "#ffffff")
  ))
]]

function on_item()
    gui.set("menu", "open", 0)
end

function on_quit()
    gui.set("menu", "open", 0)
    xtappexitflag()
end

local ok, err = lui.run(ui)
if not ok then error("demo_wmenubutton: " .. tostring(err)) end

xtapptimeout(12000, function() xtappexitflag() end)
