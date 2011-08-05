#ifndef INC_LAUNCHER_WINDOW_H
#define INC_LAUNCHER_WINDOW_H

namespace frozenbyte {
namespace launcher {
	
class LauncherWindowImpl;


class LauncherWindow
{
public:
	LauncherWindow();
	~LauncherWindow();

	void openAdvanced();
	void closeAdvanced();
	
private:
	LauncherWindowImpl* impl;
};


} // end of namespace launcher
} // end of namespace frozenbyte

#endif