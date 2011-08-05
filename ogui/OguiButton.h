
#ifndef OGUIBUTTON_H
#define OGUIBUTTON_H

//
// C++ wrapper class for orvgui buttons
//
// v1.0.0 - 24.4.2002 - jpkokkon
//

// These are pretty much just containers, functionality is in OguiWindow.
#include <string>

#include "IOguiButtonListener.h"
#include "IOguiImage.h"
#include "IOguiFont.h"
#include "OguiButtonStyle.h"


// these must be the same as the button event type values
#define OGUI_EMASK_CLICK 1
#define OGUI_EMASK_PRESS 2
#define OGUI_EMASK_OUT 4
#define OGUI_EMASK_OVER 8
#define OGUI_EMASK_LEAVE 16
#define OGUI_EMASK_HOLD 32

#define OGUI_EMASK_ALL 63

// react to buttons, these must be the same as ones in orvgui.h
#define OGUI_REACT_MASK_BUT_1 1
#define OGUI_REACT_MASK_BUT_2 2
#define OGUI_REACT_MASK_BUT_3 4
#define OGUI_REACT_MASK_BUT_4 8
#define OGUI_REACT_MASK_BUT_5 16
#define OGUI_REACT_MASK_BUT_6 32
#define OGUI_REACT_MASK_BUT_7 64
#define OGUI_REACT_MASK_OVER 128
#define OGUI_REACT_MASK_ALL 255


// incomplete defs for friend operator
class OguiWindow;
class Ogui;


class OguiButton
{
public:
	enum TEXT_H_ALIGN
	{
		TEXT_H_ALIGN_LEFT = 1,
		TEXT_H_ALIGN_CENTER = 2,
		TEXT_H_ALIGN_RIGHT = 3
	};

	enum TEXT_V_ALIGN
	{
		TEXT_V_ALIGN_TOP = 1,
		TEXT_V_ALIGN_MIDDLE = 2,
		TEXT_V_ALIGN_BOTTOM = 3
	};

	// never construct directly, use ogui to make instances of these
	OguiButton(Ogui *ogui, int id, const void *argument);
	~OguiButton();

	// manually set images and fonts
	void SetImage(IOguiImage *image);
	void SetDownImage(IOguiImage *imageDown);
	void SetDisabledImage(IOguiImage *imageDisabled);
	void SetHighlightedImage(IOguiImage *imageHighlighted);

	void GetImages(IOguiImage **image, IOguiImage **down, IOguiImage **disabled, IOguiImage **high);
	void SetImageAutoDelete(bool image, bool down, bool disabled, bool high);
	void GetImageAutoDelete(bool *image, bool *down, bool *disabled, bool *high);

	// TODO: Doesn't support if font size changes
	void SetFont(IOguiFont *font);
	void SetDownFont(IOguiFont* font );
	void SetDisabledFont( IOguiFont* font );
	void SetHighlightedFont( IOguiFont* font );

	// query for font (for OguiTextLabel use)
	IOguiFont *GetFont();

	// sets the size of button
	void Resize(int sizeX, int sizeY);

	// for textlabel
	int GetSizeX();
	int GetSizeY();
	int GetX();
	int GetY();

	void SetClipToWindow( bool clip ); // by Pete

	// move button
	void Move(int x, int y);
	void MoveBy( int x, int y );		// by Pete

	// set images, fonts and size according to a button style
	void SetStyle(OguiButtonStyle *style);

	// sets the listener object for this button
	void SetListener(IOguiButtonListener *listener);

	// sets events that will be sent to listener (defaults to click event only)
	void SetEventMask(int allowedEvents);

	// sets cursor buttons to which this button visually reacts to 
	// that is, to which buttons presses the button down image is shown
	// (also, defines whether button highlighting will occur or not)
	void SetReactMask(int reactButtons);

	// sets the button caption text, use only for text buttons
	// returns true if the text has been changed
	bool SetText(const char *text);

	// sets the interpretation of linebreaks on or off (linefeed chars)
	void SetLineBreaks(bool linebreaks);

	// query for line breaks (for OguiTextLabel use)
	bool IsLineBreaks();

	// sets horizontal alignment for the button caption text
	void SetTextHAlign(OguiButton::TEXT_H_ALIGN hAlign);

	// sets vertical alignment for the button caption text
	void SetTextVAlign(OguiButton::TEXT_V_ALIGN vAlign);

	// disables or enables the button
	void SetDisabled(bool disabled);
	bool isDisabled();

	// set rotation angle
	void SetAngle(float angle);

	// 0 for no transparency, 100 for full transparency
	void SetTransparency(int transparencyPercentage);

	// get id number
	int GetId();
	void SetId(int id);

	// get argument
	const void *GetArgument();

	void Focus(int withCursor);

	void SetClip(float leftX, float topY, float rightX, float bottomY);
	void GetClip(float &leftX, float &topY, float &rightX, float &bottomY);

	void SetRepeat(float repeatX, float repeatY);
	void SetScroll(float scrollX, float scrollY);
	void SetWrap(bool wrap);

	void SetHotKey(int hotkey, int hotkeyModifier1 = 0, int hotkeyModifier2 = 0);

	// swaps image_norm and image_down images
	void SetSelected( bool selected );
	bool IsSelected();

	// NOTE! these are auto deleted automagicly
	void SetSelectedImages( IOguiImage* selected_norm, IOguiImage* selected_high );
	void DeleteSelectedImages();

	// manually adjust the pressing state of the button
	void PressButton( bool press );

	inline std::string * getText() { return &text; };

	struct Vertex
	{
		int x,y;
		DWORD color;
	};
	void SetCustomShape(Vertex *vertices, int numVertices);

private:
	Ogui *ogui;
	int id;
	int eventMask;
	const void *argument;
	IOguiButtonListener *listener;
	OguiWindow *parent;

	IOguiImage *image;
	IOguiImage *imageDown;
	IOguiImage *imageDisabled;
	IOguiImage *imageHighlighted;
	IOguiFont *font;
	IOguiFont *fontDown;
	IOguiFont *fontDisabled;
	IOguiFont *fontHighlighted;

	bool imageAutodel;
	bool imageDownAutodel;
	bool imageDisabledAutodel;
	bool imageHighlightedAutodel;
	float rotation;
	

	// some internal hacks 
	// (the real button implementation is hidden behind this pointer)
	void *but;

	// added by Pete for the text type effect
	std::string text;

	// added because of the fucking artists and their crappy feature requests
	bool selected;
	IOguiImage* imageSelected;
	IOguiImage* imageSelectedHigh;
	
	friend class Ogui;
	friend class OguiWindow;
	friend class OguiAligner;
	friend void ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE et);

	// internal method, sets images to orvgui
	void ApplyImages();
	void ApplyFonts();

	// internal method, reset stuff (next storm generation)
	void ResetData();

	// internal method, for figuring out which font is currently in use
	IOguiFont* GetTheFontCurrentlyInUse();
};

#endif

