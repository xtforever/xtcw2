-- Wbravo rich-text demo: the piece-table editor with per-run fonts/styles and
-- per-paragraph formatting.
--
-- The burger menu (top-left) has two tabs:
--   Style     – bold/italic/roman, font face and size
--   Paragraph – alignment, indentation and paragraph spacing
-- Paragraphs are separated by blank lines; a single newline is a soft break
-- inside the same paragraph.  The caret/selection determines the target
-- paragraph(s).  Ctrl+B / Ctrl+I / Ctrl+R also toggle bold/italic/roman.

local gui = require('gui_xt')
local lui = require('lui')

local function fmt(action, param)
    local ed = gui.get_widget("ed")
    if not ed then return end
    if param then
        xtaction(ed, action, param)
    else
        xtaction(ed, action)
    end
    xtaction(ed, "SetFocus")
end

-- Run an editor action, keep focus, then dismiss the burger menu.
local function menu_fmt(action, param)
    fmt(action, param)
    gui.set("fmt", "open", 0)
end

function on_bold()   menu_fmt("bold")   end
function on_italic() menu_fmt("italic") end
function on_roman()  menu_fmt("roman")  end
function on_serif()  menu_fmt("set_font", "Serif") end
function on_sans()   menu_fmt("set_font", "Sans")  end
function on_small()  menu_fmt("set_size", "12")    end
function on_large()  menu_fmt("set_size", "24")    end

function on_align_left()    menu_fmt("align_left")    end
function on_align_center()  menu_fmt("align_center")  end
function on_align_right()   menu_fmt("align_right")   end
function on_align_justify() menu_fmt("align_justify") end

function on_indent()        menu_fmt("indent")        end
function on_outdent()       menu_fmt("outdent")       end
function on_indent_right()  menu_fmt("indent_right")  end
function on_outdent_right() menu_fmt("outdent_right") end
function on_indent_first()  menu_fmt("indent_first")  end
function on_outdent_first() menu_fmt("outdent_first") end

function on_space_before()  menu_fmt("set_space_before", "6") end
function on_space_after()   menu_fmt("set_space_after", "6")  end

local ui = [[
(window :id "win" :title "Wbravo — rich text" :width 900 :height 640
  (grid :id "main" :weightx 1 :weighty 1
    (menu-button :id "fmt"
                 :gridx 0 :gridy 0 :buttonSize 40 :fill 0
                 :popupWidth 360 :popupHeight 340
      (tabs :id "fmt_tabs" :weightx 1 :weighty 1 :fill 3
        (grid :id "style" :tab-title "Style"
          (button :label "Bold"   :gridx 0 :gridy 0 :weightx 1 :fill 3 :callback "LUA(on_bold)")
          (button :label "Italic" :gridx 0 :gridy 1 :weightx 1 :fill 3 :callback "LUA(on_italic)")
          (button :label "Roman"  :gridx 0 :gridy 2 :weightx 1 :fill 3 :callback "LUA(on_roman)")
          (button :label "Serif"  :gridx 0 :gridy 3 :weightx 1 :fill 3 :callback "LUA(on_serif)")
          (button :label "Sans"   :gridx 0 :gridy 4 :weightx 1 :fill 3 :callback "LUA(on_sans)")
          (button :label "12pt"   :gridx 0 :gridy 5 :weightx 1 :fill 3 :callback "LUA(on_small)")
          (button :label "24pt"   :gridx 0 :gridy 6 :weightx 1 :fill 3 :callback "LUA(on_large)"))
        (grid :id "para" :tab-title "Paragraph"
          (button :label "Left"    :gridx 0 :gridy 0 :weightx 1 :fill 3 :callback "LUA(on_align_left)")
          (button :label "Center"  :gridx 1 :gridy 0 :weightx 1 :fill 3 :callback "LUA(on_align_center)")
          (button :label "Right"   :gridx 0 :gridy 1 :weightx 1 :fill 3 :callback "LUA(on_align_right)")
          (button :label "Justify" :gridx 1 :gridy 1 :weightx 1 :fill 3 :callback "LUA(on_align_justify)")
          (button :label "Indent +" :gridx 0 :gridy 2 :weightx 1 :fill 3 :callback "LUA(on_indent)")
          (button :label "Indent -" :gridx 1 :gridy 2 :weightx 1 :fill 3 :callback "LUA(on_outdent)")
          (button :label "Right +"  :gridx 0 :gridy 3 :weightx 1 :fill 3 :callback "LUA(on_indent_right)")
          (button :label "Right -"  :gridx 1 :gridy 3 :weightx 1 :fill 3 :callback "LUA(on_outdent_right)")
          (button :label "First +"  :gridx 0 :gridy 4 :weightx 1 :fill 3 :callback "LUA(on_indent_first)")
          (button :label "First -"  :gridx 1 :gridy 4 :weightx 1 :fill 3 :callback "LUA(on_outdent_first)")
          (button :label "Before 6pt" :gridx 0 :gridy 5 :weightx 1 :fill 3 :callback "LUA(on_space_before)")
          (button :label "After 6pt"  :gridx 1 :gridy 5 :weightx 1 :fill 3 :callback "LUA(on_space_after)"))))
    (label :id "hdr"
           :label "Use the burger menu (Style / Paragraph). Paragraphs are separated by blank lines."
           :gridx 1 :gridy 0 :weightx 1 :weighty 0 :fill 3)
    (Wbravo :id "ed"
            :gridx 0 :gridy 1 :gridWidth 2 :weightx 1 :weighty 1 :fill 3
            :text "First paragraph: put the caret here and pick Center in the Paragraph menu.\nThis second line is a soft break in the same paragraph.\n\nSecond paragraph: select it and try Indent + or Justify.\n\nThird paragraph: space before/after adds vertical gaps around a paragraph."
            :fontFace "Sans" :fontSize 16)
  ))
]]

local ok, err = lui.run(ui)
if not ok then error("demo_rich_bravo: " .. tostring(err)) end
