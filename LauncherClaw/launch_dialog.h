#ifndef INC_LAUNCH_DIALOG_H
#define INC_LAUNCH_DIALOG_H

#pragma warning(disable:4786)

namespace frozenbyte {
namespace launcher {

class Window;
class DialogData;
class LaunchDialogHandler;
class LauncherWindow;

///////////////////////////////////////////////////////////////////////////////

class LaunchDialog
{
public:
	LaunchDialog( const Window& parent_window, LauncherWindow* lwindow );
	~LaunchDialog();

	void getSize(int &x, int &y) const;
	void show();
	void hide();

private:

	DialogData*				dialog;
	LaunchDialogHandler*	handler;
	
};

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher
} // end of namespace frozenbyte

#endif