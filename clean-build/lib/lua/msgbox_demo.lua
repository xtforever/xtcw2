-- lui MessageBox Demo

local lui = require("lui")

-- Lua function to handle info button
function on_info()
    lui.set_resource("*cancel*icon", "filename", "SVG/info.svg")
    lui.set_resource("*cancel*txt", "label", "{\\bf Info:}\nThis is a standard information message.")
    lui.set_resource("*shell2", "title", "Information")
    lui.msgbox()
end

-- Lua function to handle warning button
function on_warn()
    lui.set_resource("*cancel*icon", "filename", "SVG/warning.svg")
    lui.set_resource("*cancel*txt", "label", "{\\bf Warning:}\nBe careful! This action might have consequences.")
    lui.set_resource("*shell2", "title", "Warning")
    lui.msgbox()
end

-- Lua function to handle error button
function on_error()
    lui.set_resource("*cancel*icon", "filename", "SVG/error.svg")
    lui.set_resource("*cancel*txt", "label", "{\\bf Error:}\nAn unexpected error occurred while processing.")
    lui.set_resource("*shell2", "title", "Error")
    lui.msgbox()
end

-- Lua function to handle success button
function on_success()
    lui.set_resource("*cancel*icon", "filename", "SVG/success.svg")
    lui.set_resource("*cancel*txt", "label", "{\\bf Success:}\nThe operation completed successfully.")
    lui.set_resource("*shell2", "title", "Success")
    lui.msgbox()
end

local function main()
    local interface = [[
RetexTest.WcChildren: grid

*grid.wcClass: Gridbox
*grid.WcChildren: title info_btn warn_btn error_btn success_btn quit_btn
*grid.weightx: 1
*grid.weighty: 1

*title.gridx: 0
*title.gridy: 0
*title.gridwidth: 2
*title.wcClass: Wlabel
*title.label: {\huge \bf MessageBox Demo}
*title.bg_norm: #34495e
*title.height: 60

*info_btn.gridx: 0
*info_btn.gridy: 1
*info_btn.wcClass: Wbutton
*info_btn.label: Info
*info_btn.callback: on_info

*warn_btn.gridx: 1
*warn_btn.gridy: 1
*warn_btn.wcClass: Wbutton
*warn_btn.label: Warning
*warn_btn.callback: on_warn

*error_btn.gridx: 0
*error_btn.gridy: 2
*error_btn.wcClass: Wbutton
*error_btn.label: Error
*error_btn.callback: on_error

*success_btn.gridx: 1
*success_btn.gridy: 2
*success_btn.wcClass: Wbutton
*success_btn.label: Success
*success_btn.callback: on_success

*quit_btn.gridx: 0
*quit_btn.gridy: 3
*quit_btn.gridwidth: 2
*quit_btn.wcClass: Wbutton
*quit_btn.label: Quit
*quit_btn.callback: WcQuit

-- Configuration for the popup MessageBoxes
*shell2*cancel.callback: WcPopdown
*cancel*txt.fontSize: 14
*cancel*txt.bg_norm: #2c3e50
*cancel*Button.bg_norm: #3498db
*cancel*Button.label: Close
]]

    lui.run(interface)
    lui.loop()
end

main()
