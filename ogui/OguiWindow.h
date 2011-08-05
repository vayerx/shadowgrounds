
#ifndef OGUIWINDOW_H
#define OGUIWINDOW_H

//
// C++ wrapper class for another wrapper class for orvgui windows ;)
//
// v1.0.0 - 24.4.2002 - jpkokkon
//

#include "OguiException.h"
#include "OguiButton.h"
#include "IOguiImage.h"
#include "IOguiFont.h"
#include "IOguiEffectListener.h"

// react to buttons, these must be the same as ones in orvgui.h
#define OGUI_WIN_REACT_MASK_CURSOR_1 1
#define OGUI_WIN_REACT_MASK_CURSOR_2 2
#define OGUI_WIN_REACT_MASK_CURSOR_3 4
#define OGUI_WIN_REACT_MASK_CURSOR_4 8
#define OGUI_WIN_REACT_MASK_CURSOR_5 16
#define OGUI_WIN_REACT_MASK_CURSOR_6 32
#define OGUI_WIN_REACT_MASK_CURSOR_7 64
#define OGUI_WIN_REACT_MASK_CURSOR_8 128
#define OGUI_WIN_REACT_MASK_ALL 255


// these must be the same as the effect event type values
#define OGUI_WINDOW_EFFECT_FADEOUT 1
#define OGUI_WINDOW_EFFECT_FADEIN 2
#define OGUI_WINDOW_EFFECT_MOVEOUT 4
#define OGUI_WINDOW_EFFECT_MOVEIN 8
#define OGUI_WINDOW_EFFECT_TEXTTYPE 16
#define OGUI_WINDOW_EFFECT_TEXTLINE 32



class Ogui;
class LinkedList;


class OguiWindow
{
public:

	// NOTE: these values must equal the ones defined in orvgui.h
	enum MOVE_BOUND
	{
		MOVE_BOUND_ALL_IN_SCREEN = 0, // windows must be fully inside screen
		MOVE_BOUND_PART_IN_SCREEN = 1, // some part of window must be on screen
		MOVE_BOUND_NO_PART_IN_SCREEN = 2 // no part of the window must be on screen :)
	};

	OguiWindow(Ogui *ogui, int x, int y, int sizex, int sizey, IOguiImage *img, int id);
	~OguiWindow();

	void Raise();
	void Lower();
	void Show();
	void Hide();
	bool IsVisible() const;
	void SetPopup();
	void SetPopupNoClose();
	void SetPopupNoCloseOnButton();
	void SetOnlyActive();
	void RestoreAllActive();
	void MoveTo(int x, int y);
	void Resize(int x, int y);

	int GetPositionX();
	int GetPositionY();
	int GetSizeX();
	int GetSizeY();

	void SetReactMask(int reactCursors);

	void SetMovable();
	void SetUnmovable();

	int GetId();

	void StartEffect(int windowEffect, int effectDuration);
	void EndAllEffects();

	void SetEffectListener(IOguiEffectListener *listener);

	// HACK: SPECIAL EFFECTS HACK - should be re-implemented in some better way...
	void setBackgroundScroll(float scrollX, float scrollY);
	void setBackgroundRepeatFactor(float x, float y);
	void setBackgroundRepeatAuto(void);

	void setBackgroundImage( IOguiImage* img );

	// 0 for no transparency, 100 for full transparency
	void SetTransparency(int transparencyPercentage);

	void SetMoveBoundaryType(MOVE_BOUND btype);

	// this should only be called by the ogui button...
	// not private, cos don't want to play with friend operators.
	void buttonDeleted(OguiButton *b);

	IOguiImage *getBackgroundImage() const;

private:
	
	OguiButton *CreateNewButton(int x, int y, int sizex, int sizey, 
		IOguiImage *img, IOguiImage *imgdown, IOguiImage *imghigh, IOguiImage *imgdisabled, bool withText, 
		const char *text, int id, const void *argument, IOguiFont *font, bool clipToWindow = true );

	Ogui *ogui;
	IOguiImage *image;
	bool release_bg_image;
	bool is_only_active;
	mutable bool is_visible;
	int id;

	int effectFadeTimeLeft;
	int effectFadeTimeTotal;
	int effectMoveTimeLeft;
	int effectMoveTimeTotal;
	int effectTextTypeTimeLeft;
	int effectTextTypeTimeTotal;
	int effectTextLineTimeLeft;
	int effectTextLineTimeTotal;

	bool fadingIn;
	bool fadingOut;
	bool movingIn;
	bool movingOut;
	
	int windowPositionX;
	int windowPositionY;

	IOguiEffectListener *effectListener;

	LinkedList *buttonList;

	// some internal hacks 
	// (the real orvgui window implementation is hidden behind this pointer)
	void *win;

	friend class Ogui;
	friend class OguiAligner;

	// internal method, reset stuff (next storm generation)
	void ResetData();
};

#endif

