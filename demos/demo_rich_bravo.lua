-- Wbravo rich-text demo: the piece-table editor with per-run fonts/styles.
--
-- Select text and use the toolbar (or Ctrl+B / Ctrl+I / Ctrl+R) to toggle
-- bold/italic/roman, or click a font/size button and keep typing.

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

function on_bold()   fmt("bold")   end
function on_italic() fmt("italic") end
function on_roman()  fmt("roman")  end
function on_serif()  fmt("set_font", "Serif") end
function on_sans()   fmt("set_font", "Sans")  end
function on_small()  fmt("set_size", "12")    end
function on_large()  fmt("set_size", "24")    end

local ui = [[
(window :id "win" :title "Wbravo — rich text" :width 860 :height 580
  (grid :id "main" :weightx 1 :weighty 1
    (Wbutton :id "b_bold"   :label "Bold"   :gridx 0 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_bold)")
    (Wbutton :id "b_italic" :label "Italic" :gridx 1 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_italic)")
    (Wbutton :id "b_roman"  :label "Roman"  :gridx 2 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_roman)")
    (Wbutton :id "b_serif"  :label "Serif"  :gridx 3 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_serif)")
    (Wbutton :id "b_sans"   :label "Sans"   :gridx 4 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_sans)")
    (Wbutton :id "b_small"  :label "12pt"   :gridx 5 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_small)")
    (Wbutton :id "b_large"  :label "24pt"   :gridx 6 :gridy 0 :weightx 0 :weighty 0 :callback "LUA(on_large)")
    (Wbravo :id "ed"
            :gridx 0 :gridy 1 :gridWidth 7 :weightx 1 :weighty 1 :fill 3
            :text "Rich text: select text and press Bold/Italic, or click a font/size and keep typing.\n\nCtrl+B bold, Ctrl+I italic, Ctrl+R roman."
            :fontFace "Sans" :fontSize 16)
  ))
]]

local ok, err = lui.run(ui)
if not ok then error("demo_rich_bravo: " .. tostring(err)) end
