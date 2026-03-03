
-- lui Wmenu Demo

local lui = require("lui")

-- Lua function to handle menu selection
function on_menu_select(widget, selection)
    print("Menu selected index: " .. tostring(selection))
end

local function main()
    local interface = [[
RetexTest.WcChildren: grid

*grid.wcClass: Gridbox
*grid.WcChildren: file_menu edit_menu content
*grid.weightx: 1
*grid.weighty: 1

*file_menu.gridx: 0
*file_menu.gridy: 0
*file_menu.wcClass: Wmenu
*file_menu.label: File
*file_menu.lst: <String> "New,Open,Save,Quit"
*file_menu.menu_cb: on_menu_select
*file_menu.weightx: 0
*file_menu.weighty: 0

*edit_menu.gridx: 1
*edit_menu.gridy: 0
*edit_menu.wcClass: Wmenu
*edit_menu.label: Edit
*edit_menu.lst: <String> "Undo,Redo,Cut,Copy,Paste"
*edit_menu.menu_cb: on_menu_select
*edit_menu.weightx: 0
*edit_menu.weighty: 0

*content.gridx: 0
*content.gridy: 1
*content.gridwidth: 2
*content.wcClass: Wlabel
*content.label: {\huge \bf Menu Demo}

Click on 'File' or 'Edit' to see the pull-down menus.
Selection index will be printed to stdout.
*content.width: 600
*content.height: 400
*content.bg_norm: #2c3e50
]]

    lui.build(interface)
    lui.main_loop()
end

main()
