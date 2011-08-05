#ifndef INC_LAUNCH_DIALOG_H
#define INC_LAUNCH_DIALOG_H

#pragma warning(disable:4786)

#include "resource.h"
#include "LauncherWindow.h"
#include "dlghandlerimpl.h"
#include "options_value_manager.h"

#define TAB_LAUNCH 0
#define TAB_GRAPHICS 1
#define TAB_SOUNDS 2
#define TAB_PHYSICS 3
#define TAB_MODS_AND_LEVEL_EDITOR 4
#define TAB_UPDATES_AND_NEWS 5
#define TAB_AMOUNT 6

const int tabHandles[] = {
	IDD_LAUNCHDIALOG, 
	IDD_GRAPHICSDIALOG, 
	IDD_SOUNDSDIALOG, 
	IDD_PHYSICSDIALOG, 
	IDD_MODDIALOG, 
	IDD_UPDATESDIALOG
};

namespace frozenbyte {
namespace launcher {

class Window;
class DialogData;
class LauncherMainDialogHandler;
class LauncherWindow;

///////////////////////////////////////////////////////////////////////////////

class LauncherMainDialog
{
public:
	LauncherMainDialog( const Window& parent_window, LauncherWindow* lwindow );
	~LauncherMainDialog();

	void getSize(int &x, int &y) const;
	void show();
	void hide();

private:

	void initTabs();
	bool tabsInitialized;

	DialogData * dialogData;
	DialogData * tabs;

	OptionsValueManager *optionsValueManager;
	DialogData ** subDialogs;
	LauncherMainDialogHandler * handler;

	DlgHandlerImpl * handlerList[TAB_AMOUNT];

};

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher
} // end of namespace frozenbyte

#endif