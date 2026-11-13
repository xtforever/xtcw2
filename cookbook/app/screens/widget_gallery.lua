-- cookbook/app/screens/widget_gallery.lua
-- Screen 5: integration gallery for the improved widget set.

local M = {}

M.id = "screen_gallery"
M.title = "Gallery"

function M.fragment()
    return table.concat({
        '(grid :id "screen_gallery" :tab-title "Gallery" :defaultDistance 8',
        '  (label :id "gal_hdr" :label "Improved widget gallery" :gridx 0 :gridy 0 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :fontSize 16)',
        '  (label :id "gal_intro" :label "New widgets integrated into the cookbook as a testing ground." :gridx 0 :gridy 1 :gridWidth 4 :weightx 1 :weighty 0 :fill 3)',
        '  (WFormLayout :id "gal_form" :gridx 0 :gridy 2 :gridWidth 4 :weightx 1 :weighty 0 :fill 3',
        '                :labelWidth 120 :columnGap 12 :rowGap 10 :margin 12',
        '    (label :role "label" :label "Checkbox:")',
        '    (Wcheckbox :id "gal_check" :role "field" :label "Publish recipe" :checked TRUE)',
        '    (label :role "label" :label "Progress:")',
        '    (WProgressBar :id "gal_progress" :role "field" :value 62 :min 0 :max 100)',
        '    (label :role "label" :label "Category:")',
        '    (WComboBox :id "gal_combo" :role "field" :placeholder "Main" :linesHeight 4)',
        '    (label :role "label" :label "Size:")',
        '    (WRadioGroup :id "gal_radio" :role "field" :items "Small,Medium,Large" :selected 1 :itemHeight 26)',
        '  )',
        '  (label :id "gal_hint" :label "Use this tab to exercise new widgets before they get their own probes." :gridx 0 :gridy 3 :gridWidth 4 :weightx 1 :weighty 0 :fill 3 :useSmallFont TRUE)',
        ')',
    }, "\n")
end

return M
