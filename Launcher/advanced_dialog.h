#ifndef INC_ADVANCED_DIALOG_H
#define INC_ADVANCED_DIALOG_H

#pragma warning(disable:4786)

namespace frozenbyte {
namespace launcher {

class Window;
class DialogData;
class AdvancedDialogHandler;
class LauncherWindow;

///////////////////////////////////////////////////////////////////////////////

class AdvancedDialog
{
public:
	AdvancedDialog( const Window& parent_window, LauncherWindow* lwindow );
	~AdvancedDialog();

	void show();
	void hide();

private:

	DialogData*				dialog;
	AdvancedDialogHandler*	handler;
	
};

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher
} // end of namespace frozenbyte

#endif