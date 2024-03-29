#     wbuild
#     Copyright (C) 1996  Joel N. Weber II <nemo@koa.iolani.honolulu.hi.us>
#     
#     This program is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public License
#     as published by the Free Software Foundation; either version 2
#     of the License, or (at your option) any later version.
#     
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#     
#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# This file contains some knownlege of the base classes.  Some of
# it is however hardcoded into generatec.c; as wbuild gets cleaned
# up some more this may change.

@class Object				@nocode @nodoc @file="X11/Object"

@private
@ pointer to widget itself		@var Widget self
@ pointer to Widget's ClassRec		@var WidgetClass widget_class
@ parent widget				@var Widget parent
@ widget resource name quarkified	@var XrmName xrm_name
@ marked for destroy			@var Boolean being_destroyed
@ who to call when widget destroyed	@var XtCallbackList destroy_callbacks
@ constraint record			@var XtPointer constraints

@methods
@proc class_initialize() {}
@proc class_part_initialize(WidgetClass class) {}
@proc initialize(Widget request, $, ArgList args, Cardinal *num_args) {}
@proc initialize_hook($, ArgList args, Cardinal *num_args) {}
@proc destroy($) {}
@proc Boolean set_values(Widget old, Widget request, $, ArgList args,
      Cardinal *num_args) {}
@proc Boolean set_values_hook($, ArgList args, Cardinal *num_args) {}
@proc get_values_hook($, ArgList args, Cardinal *num_args) {}

@classvars
@ pointer to superclass ClassRec	@var WidgetClass superclass = NULL
@ widget resource class name		@var String class_name = "Object"
@ size in bytes of widget record @var Cardinal widget_size = sizeof(ObjectRec)
@ has class been initialized?		@var XtEnum class_inited = FALSE
@ resources for subclass fields		@var XtResourceList resources = NULL
@ number of entries in resources	@var Cardinal num_resources = 0
@ resource class quarkified		@var XrmClass xrm_class = NULLQUARK
@ version of intrinsics used		@var XtVersionType version = XtVersion
@ list of callback offsets		@var XtPointer callback_private = NULL
@ pointer to extension record		@var XtPointer extension = NULL


@class RectObj (Object)			@nocode @nodoc @file="X11/RectObj"

@public
@var Position x = 0
@var Position y = 0
@var Dimension width = 0
@var Dimension height = 0
@var Dimension border_width <borderWidth> = 0
@var Boolean sensitive = True
@var Boolean ancestor_sensitive <ancestorSensitive> = False

@private
@ is widget geometry managed?		@var Boolean managed

@methods
@proc class_initialize() {}
@proc class_part_initialize(WidgetClass class) {}
@proc initialize(Widget request, $, ArgList args, Cardinal *num_args) {}
@proc initialize_hook($, ArgList args, Cardinal *num_args) {}
@proc destroy($) {}
@proc Boolean set_values(Widget old, Widget request, $, ArgList args,
      Cardinal *num_args) {}
@proc Boolean set_values_hook($, ArgList args, Cardinal *num_args) {}
@proc set_values_almost(Widget old, $, XtWidgetGeometry *request,
      XtWidgetGeometry *reply) {}
@proc get_values_hook($, ArgList args, Cardinal *num_args) {}
@proc XtGeometryResult query_geometry($, XtWidgetGeometry *request,
      XtWidgetGeometry *reply) {}
@proc expose($, XEvent *event, Region region) {}


@classvars
@ pointer to superclass ClassRec	@var WidgetClass superclass = NULL
@ widget resource class name		@var String class_name = "Object"
@ size in bytes of widget record @var Cardinal widget_size = sizeof(ObjectRec)
@ has class been initialized?		@var XtEnum class_inited = FALSE
@ resources for subclass fields		@var XtResourceList resources = NULL
@ number of entries in resources	@var Cardinal num_resources = 0
@ resource class quarkified		@var XrmClass xrm_class = NULLQUARK
@ version of intrinsics used		@var XtVersionType version = XtVersion
@ list of callback offsets		@var XtPointer callback_private = NULL
@ pointer to extension record		@var XtPointer extension = NULL



@class Core @nocode @nodoc @file="X11/Core"

@public
@var Position x = 0
@var Position y = 0
@var Dimension width = 0
@var Dimension height = 0
@var Dimension border_width <borderWidth> = 0
@var Colormap colormap = NULL
@var int depth = 0
@var XtCallbackList destroy_callback <destroyCallback> = NULL
@var Boolean sensitive = True
@var XtTMRec tm = NULL
@var Boolean ancestor_sensitive <ancestorSensitive> = False
@var XtTranslations accelerators <accelerators> = NULL
@var Pixel border_pixel <borderColor> = 0
@var Pixmap border_pixmap <borderPixmap> = NULL
@var Pixel background_pixel <background> = 0
@var Pixmap background_pixmap <backgroundPixmap> = NULL
@var Boolean mapped_when_managed <mappedWhenManaged> = True
@var Screen * screen = NULL

@private
@ pointer to widget itself		@var Widget self
@ pointer to Widget's ClassRec		@var WidgetClass widget_class
@ parent widget				@var Widget parent
@ widget resource name quarkified	@var XrmName xrm_name
@ marked for destroy			@var Boolean being_destroyed
@ who to call when widget destroyed	@var XtCallbackList destroy_callbacks
@ constraint record			@var XtPointer constraints
@ translation management		@var XtTMRec tm
@ is widget geometry managed?		@var Boolean managed
@ list of popups			@var WidgetList popup_list
@ how many popups			@var Cardinal num_popups
@ widget resource name			@var String name
@ window's screen			@var Screen *screen
@ window ID				@var Window window
@ is window mapped and not occluded?	@var Boolean visible

@methods
@proc class_initialize() {}
@proc class_part_initialize(WidgetClass class) {}
@proc initialize(Widget request, $, ArgList args, Cardinal *num_args) {}
@proc initialize_hook($, ArgList args, Cardinal *num_args) {}
@proc realize($, XtValueMask *mask, XSetWindowAttributes *attributes) {}
@proc destroy($) {}
@proc resize($) {}
@proc expose($, XEvent *event, Region region) {}
@proc Boolean set_values(Widget old, Widget request, $, ArgList args,
      Cardinal *num_args) {}
@proc Boolean set_values_hook($, ArgList args, Cardinal *num_args) {}
@proc set_values_almost(Widget old, $, XtWidgetGeometry *request,
      XtWidgetGeometry *reply) {}
@proc get_values_hook($, ArgList args, Cardinal *num_args) {}
@proc Boolean accept_focus($, Time *time) {}
@proc XtGeometryResult query_geometry($, XtWidgetGeometry *request,
      XtWidgetGeometry *reply) {}
@proc display_accelerator($, String str) {}

@classvars
@ pointer to superclass ClassRec	@var WidgetClass superclass = NULL
@ widget resource class name		@var String class_name = "Core"
@ size in bytes of widget record  @var Cardinal widget_size = sizeof(CoreRec)
@ has class been initialized?		@var XtEnum class_inited = FALSE
@ widget semantics name to proc map	@var XtActionList actions = NULL
@ number of entries in actions		@var Cardinal num_actions = 0
@ resources for subclass fields		@var XtResourceList resources = NULL
@ number of entries in resources	@var Cardinal num_resources = 0
@ resource class quarkified		@var XrmClass xrm_class = NULLQUARK
@ compress MotionNotify for widget	@var Boolean compress_motion = False
@ compress Expose events for widget	@var XtEnum compress_exposure = FALSE
@ compress enter and leave events  @var Boolean compress_enterleave = False
@ select for VisibilityNotify		@var Boolean visible_interest = False
@ version of intrinsics used		@var XtVersionType version = XtVersion
@ list of callback offsets		@var XtPointer callback_private = NULL
@ state machine				@var String tm_table = NULL
@ pointer to extension record		@var XtPointer extension = NULL



@class Composite (Core) @nocode @nodoc @file="X11/Composite"

@classvars
@ pointer to extension record.		@var XtPointer extension = NULL

@methods

@ geometry manager for children
@proc XtGeometryResult geometry_manager(Widget child,
      XtWidgetGeometry *request, XtWidgetGeometry *reply) {}

@ change managed state of child
@proc change_managed($) {}

@ physically add child to parent
@proc insert_child(Widget child) {}

@ physically remove child
@proc delete_child(Widget child) {}

@public
@var WidgetList children = NULL
@var XtOrderProc insert_position <insertPosition> = NULL
@var Cardinal num_children <numChildren> = 0



@class Shell (Composite) @nodoc @nocode @file="X11/Shell.h"

@public
@var Boolean allow_shell_resize <allowShellResize> = False
@var XtCreatePopupChildProc create_popup_child_proc <createPopupChildProc>
     = NULL
@var String geometry = NULL
@var Boolean override_redirect <overrideRedirect> = False
@var <Callback> XtCallbackList popdown_callback <popdownCallback> = NULL
@var <Callback> XtCallbackList popup_callback <popupCallback> = NULL
@var Boolean save_under <saveUnder> = False
@var Visual* visual = CopyFromParent

@private
@var XtGrabKind grab_kind
@var Boolean spring_loaded
@var Boolean popped_up
@ re-using old name
@var Boolean client_specified

@classvars
@var XtPointer extension = NULL



@class OverrideShell (Shell) @nodoc @nocode @file="X11/Shell"

@classvars
@var XtPointer $extension = NULL



@class WMShell (Shell) @nodoc @nocode @file="X11/Shell"

@public
@var int base_height <baseHeight> = XtUnspecifiedShellInt
@var int base_width <baseWidth> = XtUnspecifiedShellInt
@var String title = NULL
@var Atom title_encoding <titleEncoding> = XA_STRING
@var Boolean transient = False
@var Boolean wait_for_wm <waitForWm> = True
@var int win_gravity <winGravity> = NULL
@var int wm_timeout <wmTimeout> = 5000

@classvars
@var XtPointer $extension = NULL



@class VendorShell (WMShell) @nodoc @nocode @file="X11/Vendor"

@private
@var int vendor_specific

@classvars
@var XtPointer $extension = NULL



@class TransientShell (VendorShell) @nodoc @nocode @file="X11/Shell"

@public
@var Widget transient_for <transientFor> = NULL

@classvars
@var XtPointer $extension = NULL



@class TopLevelShell (VendorShell) @nodoc @nocode @file="X11/Shell"

@public
@var String icon_name <iconName> = NULL
@var Boolean iconic = False
@var Atom icon_name_encoding <iconNameEncoding> = XA_STRING

@classvars
@var XtPointer $extension = NULL



@class ApplicationShell (TopLevelShell) @nodoc @nocode @file="X11/Shell"

@public
@var int argc = 0
@var String *argv = NULL

@private
@var String class
@var XrmClass xrm_class

@classvars
@var XtPointer $extension = NULL



@class Constraint (Composite) @nodoc @nocode @file="X11/Constraint"

@classvars
@ constraint resource list
	@var XtResourceList constraint_resources = NULL
@ number of constraints in list
	@var Cardinal num_constraint_resources = 0
@ size of constraint record
	@var Cardinal constraint_size = 0
@ pointer to extension record
	@var XtPointer constraint_extension = NULL

@methods

@ constraint initialization
@proc constraint_initialize(Widget request, Widget new, ArgList args,
	Cardinal* num_args) {}

@ constraint destroy proc
@proc constraint_destroy(Widget child) {}

@ constraint set_values proc
@proc constraint_set_values(Widget old, Widget request, Widget new,
	ArgList args, Cardinal* num_args) {}



@class XmPrimitive (Core) @nodoc @nocode @file="Xm/Primitive"
# @nopubinclude

@public
@var Pixel bottom_shadow_color = 0
@var Pixmap bottom_shadow_pixmap = None
@var Pixel foreground = 0
@var XtCallbackList help_callback = NULL
@var Pixel highlight_color = 0
@var Boolean highlight_on_enter = 0
@var Pixmap highlight_pixmap = 0
@var Dimension highlight_thickness = 2
@var <NavigationType> XmNavigationType navigation_type <navigationType>
	= XmNONE
@var Dimension shadow_thickness = 2
@var Pixel top_shadow_color = 0
@var Pixmap top_shadow_pixmap = None
@var Boolean traversal_on = True
@var unsigned char unit_type = XmPIXELS
@var XtPointer user_data = NULL

@private
@var Boolean have_traversal
@var Boolean highlight_drawn
@var Boolean highlighted
@var GC highlight_GC
@var GC top_shadow_GC
@var GC bottom_shadow_GC

@classvars
@var String translations = XtInheritTranslations
@var XmSyntheticResource * syn_resources = 0
@var int num_syn_resources = 0
@var XtPointer primitive_extension = NULL

@methods
@proc border_highlight($) {}
@proc border_unhighlight($) {}
@proc arm_and_activate($, XEvent* event, String*params, Cardinal*num_params) {}


@class XmManager (Constraint) @nodoc @nocode @file="Xm/Manager"
# @nopubinclude

@public
@var Pixel bottom_shadow_color = 0
@var Pixmap bottom_shadow_pixmap = 0
@var Pixel foreground = 0
@var <Callback> XtCallbackList help_callback = NULL
@var Pixel highlight_color = 0
@var Pixmap highlight_pixmap = None
@var Widget initial_focus = NULL
@var <NavigationType> XmNavigationType navigation_type <navigationType>
	= XmTAB_GROUP
@var Dimension shadow_thickness = 0
@var XmStringDirection string_direction = 0
@var Pixel top_shadow_color = 0
@var Pixmap top_shadow_pixmap = None
@var Boolean traversal_on = True
@var unsigned char unit_type = XmPIXELS
@var XtPointer user_data = NULL

@private
@var Boolean event_handler_added
@var Widget active_child
@var Widget highlighted_widget
@var Widget accelerator_widget
@var Boolean has_focus
@var XmKeyboardData * keyboard_list
@var short num_keyboard_entries
@var short size_keyboard_list
@var XmGadget selected_gadget
@var XmGadget eligible_for_multi_button_event
@var GC background_GC
@var GC highlight_GC
@var GC top_shadow_GC
@var GC bottom_shadow_GC

@classvars
@var String translations = XtInheritTranslations
@var XmSyntheticResource * syn_resources = NULL
@var int num_syn_resources = 0
@var XmSyntheticResource * syn_constraint_resources = NULL
@var int num_syn_constraint_resources = 0
@var XtPointer manager_extension = NULL

@ Override from Constraint:
@var Cardinal constraint_size = sizeof(XmManagerConstraintPart)

@methods
@proc parent_process($, XmParentProcessData data) {}


@class XmDrawingArea (XmManager) @nodoc @nocode @file="Xm/DrawingA"

@public
@var Dimension margin_width <marginWidth> = 10
@var Dimension margin_height <marginHeight> = 10
@var <Callback> XtCallbackList resize_callback <resizeCallback> = NULL
@var <Callback> XtCallbackList expose_callback <exposeCallback> = NULL
@var <Callback> XtCallbackList input_callback <inputCallback> = NULL
@var unsigned char resize_policy <resizePolicy> = XmRESIZE_ANY


@class XmBulletinBoard (XmManager) @nodoc @nocode @file="Xm/BulletinB"

@public
@var Dimension margin_width = 10
@var Dimension margin_height = 10
@var Widget default_button = NULL
@var Widget dynamic_default_button = NULL
@var Widget cancel_button = NULL
@var Widget dynamic_cancel_button = NULL
@var XtCallbackList focus_callback = NULL
@var XtCallbackList map_callback = NULL
@var XtCallbackList unmap_callback = NULL
@var XtTranslations text_translations = NULL
@var XmFontList button_font_list = NULL
@var XmFontList label_font_list = NULL
@var XmFontList text_font_list = NULL
@var Boolean allow_overlap = TRUE
@var Boolean default_position = TRUE
@var Boolean auto_unmanage = TRUE
@var unsigned char resize_policy = XmRESIZE_ANY
@var Dimension old_width = 0
@var Dimension old_height = 0
@var Dimension old_shadow_thickness = 0
@var unsigned char shadow_type = XmSHADOW_OUT
@var Boolean in_set_values = FALSE
@var Boolean initial_focus = FALSE
@var Boolean no_resize = FALSE
@var unsigned char dialog_style = 0
@var XmString dialog_title = NULL
@var Widget shell = NULL
@var Widget _UNUSED = NULL
@var XmGeoMatrix geo_cache = NULL

@classvars
@var Boolean always_install_accelerators = TRUE
@var XmGeoCreateProc geo_matrix_create = XmInheritGeoMatrixCreate
@var XmFocusMovedProc focus_moved_proc = XmInheritFocusMovedProc
@var XtPointer bulletin_board_extension = NULL

