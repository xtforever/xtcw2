
-- lui WPaned Demo

local lui = require("lui")

local function main()
    local interface = [[
RetexTest.WcChildren: paned

*weightx: 1
*weighty: 1

*paned.WcClass: WPaned
*paned.WcChildren: left_pane right_paned
*paned.orientation: 0
*paned.width: 800
*paned.height: 600

*left_pane.WcClass: Wlabel
*left_pane.label: {\huge \bf Sidebar}

This is the left pane of the primary horizontal splitter.
*left_pane.bg_norm: #34495e
*left_pane.preferredPaneSize: 200

*right_paned.WcClass: WPaned
*right_paned.orientation: 1
*right_paned.WcChildren: top_pane bottom_pane

*top_pane.WcClass: Wlabel
*top_pane.label: {\Large Content Area}

Top pane of the secondary vertical splitter.

${ \sum_{i=1}^n i = \frac{n(n+1)}{2} }$
*top_pane.bg_norm: #2c3e50
*top_pane.weighty: 2

*bottom_pane.WcClass: Wlabel
*bottom_pane.label: {\it Footer Area}

Bottom pane of the secondary vertical splitter.
*bottom_pane.bg_norm: #7f8c8d
*bottom_pane.preferredPaneSize: 100
]]

    lui.build(interface)
    lui.main_loop()
end

main()
