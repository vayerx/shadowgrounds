#ifndef OGUI_ALIGNER_H
#define OGUI_ALIGNER_H

class OguiTextLabeL;
class OguiWindow;
class OguiButton;
class Ogui;

class OguiAligner
{
public:
	enum Flags
	{
		// maintain aspect ratio in widescreen by aligning to right edge
		WIDESCREEN_FIX_RIGHT = 0x00000001,
		// maintain aspect ratio in widescreen by aligning to left edge
		WIDESCREEN_FIX_LEFT = 0x00000002,
		// maintain aspect ratio in widescreen by aligning to horizontal center
		WIDESCREEN_FIX_CENTER = 0x00000004,
	};

	static void align(OguiWindow *win, int flags, Ogui *ogui);
	static void align(OguiButton *button, int flags, Ogui *ogui);
	static void align(OguiTextLabel *label, int flags, Ogui *ogui);
	static void align(int &x, int &y, int &w, int &h, int flags, Ogui *ogui);
};

#endif
