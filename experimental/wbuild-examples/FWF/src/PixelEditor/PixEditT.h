/*****************************************************************************

	PixelEditor_test.h

 *****************************************************************************/

#ifndef	PixelEditor_test_H
#define	PixelEditor_test_H

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

/* If no Xpm, insert '#define NO_XPM' here */

#define	USE_VIEWPORT

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <X11/Xaw/BoxP.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/CommandP.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Viewport.h>

#ifndef NO_XPM
#include <X11/xpm.h>
#endif

#include <Xfwf/PixEdit.h>

#ifndef NO_FILE_SELECTOR
#include <Xfwf/FileSel.h>
#endif

typedef	void (*PFV)();

#define	FILE_BITMAP		0
#define	FILE_XPM3		1

#define	COMMAND_COPY		0
#define	COMMAND_CUT		1
#define	COMMAND_HFLIP		2
#define	COMMAND_VFLIP		3
#define	COMMAND_FILL		4
#define	COMMAND_INVERT		5

#define	END_OF_BUTTONS		-1
#define	BUTTON_PENCIL		0
#define	BUTTON_BRUSH		1
#define	BUTTON_ERASER		2
#define	BUTTON_REGION		3
#define	BUTTON_PASTE		4
#define	BUTTON_COPY		5
#define	BUTTON_CUT		6
#define	BUTTON_HFLIP		7
#define	BUTTON_VFLIP		8
#define	BUTTON_FILL		9
#define	BUTTON_INVERT		10
#define	BUTTON_GRID		11
#define	BUTTON_ZOOM_IN		12
#define	BUTTON_ZOOM_OUT		13
#define	BUTTON_INC_FG_COLOR	14
#define	BUTTON_DEC_FG_COLOR	15
#define	BUTTON_INC_BG_COLOR	16
#define	BUTTON_DEC_BG_COLOR	17
#define	BUTTON_SELECT_ALL	18
#define	BUTTON_LOAD_BITMAP	19
#define	BUTTON_SAVE_BITMAP	20
#define	BUTTON_LOAD_XPM3	21
#define	BUTTON_SAVE_XPM3	22
#define	BUTTON_SET_NAME		23
#define	BUTTON_SET_HOTSPOT	24
#define	BUTTON_SET_SIZE		25
#define	BUTTON_QUIT		26

#define	NUM_MENU_BUTTONS	27

typedef struct
{
	int number;
	char *name;
	Boolean sensitive;
	PFV function;
	int arg;
} ButtonData;

typedef struct
{
	Boolean done;
	Boolean cancelled;
	char file_name[1024];
} SelectFileState;

typedef struct
{
	Boolean done;
	Boolean cancelled;
	char string[1024];
} DialogState;

typedef struct
{
	XfwfPixelEditorWidget pixel_editor;
	Widget top1,top2,top3;
	Widget main_menu;
	Widget brush_menu;
	Widget fs_pop_up;
	Widget fs;
	Widget name_dialog;
	Widget size_dialog;
	Widget hot_spot_dialog;
	Widget buttons[NUM_MENU_BUTTONS];
	SelectFileState sel_state;
	DialogState dialog_state;
	char base_name[16];
	Boolean has_hot_spot;
	int hotx,hoty;
	Boolean have_xpm3_attributes;
	Boolean use_viewport;
#ifndef NO_XPM
	XpmAttributes xpm3_attributes;
#endif
} Globals;

int			main();
Widget			NewButton();
void			SetValue();
void			ChangeTool();
void			ChooseBrush();
void			RegionChange();
void			RegionCommand();
void			ToggleGrid();
void			SelectAll();
void			Zoom();
void			ChangeFGColor();
void			ChangeBGColor();
void			Quit();
void			LoadFile();
void			SaveFile();
Boolean			LoadBitmapFile();
Boolean			SaveBitmapFile();
#ifndef NO_XPM
Boolean			LoadXPM3File();
Boolean			SaveXPM3File();
#endif
void			CreateMainMenu();
void			CreateBrushMenu();
void			CreatePixelEditor();
void			CreateFileSelector();
Widget			CreateDialog();
char			*DialogGetInput();
char 			*SelectFile();
#ifndef NO_FILE_SELECTOR
void			FileSelectorOk();
void			FileSelectorCancel();
#endif
char 			*DialogGetInput();
void 			DialogOk();
void 			DialogNone();
void 			DialogCancel();

#endif
