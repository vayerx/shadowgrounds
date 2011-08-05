// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_GUI_H
#define INCLUDED_EDITOR_GUI_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif

namespace frozenbyte {
namespace editor {

class Dialog;
class Window;
class IMode;
class ICommand;
class Mouse;
struct GuiData;

class Gui
{
	boost::scoped_ptr<GuiData> data;

public:
	Gui();
	~Gui();

	void setMenuCommand(int id, ICommand *command);
	void reset();
	bool handleDialogs(MSG &message);

	Mouse &getMouse();

	Window &getMainWindow();
	Dialog &getRenderDialog();
	Dialog &getMenuDialog();

	Dialog &getTerrainDialog();
	Dialog &getSceneDialog();
	Dialog &getObjectsDialog();
	Dialog &getBuildingsDialog();
	Dialog &getUnitsDialog();
	Dialog &getDecoratorsDialog();
	Dialog &getParticleDialog();
	Dialog &getLightDialog();
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
