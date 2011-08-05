// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_MOUSE_H
#define INCLUDED_EDITOR_MOUSE_H

#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif

struct Storm3D_CollisionInfo;
class ObstacleCollisionInfo;

#include <datatypedef.h>

namespace frozenbyte {
namespace editor {

struct Storm;

class Mouse
{
	HWND window;

	int x;
	int y;

	bool leftButtonDown;
	bool leftButtonClicked;
	bool rightButtonDown;
	bool rightButtonClicked;

	int wheelDelta;
	bool insideWindow;

	Storm *storm;

public:
	Mouse();
	~Mouse();

	void setTrackWindow(HWND windowHandle);
	void setStorm(Storm &storm);
	void update();

	void setLeftButtonDown();
	void setLeftButtonUp();
	void setRightButtonDown();
	void setRightButtonUp();
	void setWheelDelta(int delta);

	int getX() const;
	int getY() const;
	int getWheelDelta() const;

	bool isLeftButtonDown() const;
	bool hasLeftClicked() const;
	bool isRightButtonDown() const;
	bool hasRightClicked() const;
	bool isInsideWindow() const;

	bool cursorRayTrace(Storm3D_CollisionInfo &ci, VC3 *position = 0, VC3 *direction = 0 );
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
