#ifndef INC_OGUISLIDER_H
#define INC_OGUISLIDER_H

class OguiButtonEvent;
class IOguiSliderListener;
class OguiWindow;
class Ogui;
class OguiButton;
class IOguiImage;

#include "../ogui/IOguiButtonListener.h"
#include "IOguiSliderListener.h"

#include <string>

// Horizontal slider
class OguiSlider : public IOguiButtonListener
{
public:
	OguiSlider( OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, 
		const std::string& background_norm, const std::string& background_down, const std::string& background_high,
		const std::string& foreground_norm, const std::string& foreground_down, const std::string& foreground_high, int id = 0, float value = 1.0f );

	/*OguiWindow *win, int x, int y, int sizex, int sizey, 
    const char *imageFilename, const char *imageDownFilename, const char *imageHighlightFilename,
    int id = 0, void *argument = NULL*/
	
	virtual ~OguiSlider( );

	// sets a position for the bar. Not relative to the position of the background.
	void setBarPosition( int x, int y, int w, int h );

	// moves the slider (background and foreground) to give location
	void Move( int x, int y );

	// sets transparency value to both of the buttons
	void setTransparency( int transparency );

	// Returns the value between 0 and 1
	float getValue() const;

	// The given value between 0 and 1
	void  setValue( float value );

	// adds the disabled images
	void setDisabledImages( const std::string& background_disabled, const std::string& foreground_disabled );

	void setBackgroundImages( IOguiImage *norm, IOguiImage *down, IOguiImage *high);
	void setForegroundImages( IOguiImage *norm, IOguiImage *down, IOguiImage *high);

	// disables the slider
	virtual void setDisabled( bool disabled = true );

	//
	void setListener( IOguiSliderListener* listen );

	int  getId() const;

	void setSliderDirection( bool horizontal );

	void CursorEvent( OguiButtonEvent *eve );

	int getWidth() const;
	int getHeight() const;
	void resize(int w, int h);

protected:

	const int		backgroundId;
	const int		foregroundId;

	Ogui*			ogui;
	OguiWindow*		win;

	OguiButton*		background;
	OguiButton*		foreground;
	int				x;
	int				y;
	int				w;
	int				h;
	int				id;
	float			value;

	int				offset_x;
	int				offset_y;

	bool horizontal;

	IOguiSliderListener* listener;

	IOguiImage*		disabledBackground;
	IOguiImage*		disabledForeground;


	static	OguiSlider*		updateThis;
	static  OguiSlider*	    updateThisIfHold;
};

///////////////////////////////////////////////////////////////////////////////
// Good old dirty inheritance hack
// A slider that can't be manipulated with mouse 
class OguiGfxSlider : public OguiSlider
{
public:
	OguiGfxSlider(  OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, 
		const std::string& background_norm, const std::string& background_down, const std::string& background_high,
		const std::string& foreground_norm, const std::string& foreground_down, const std::string& foreground_high, int id = 0, float value = 1.0f  );
	virtual ~OguiGfxSlider();
	
	virtual void setDisabled( bool disabled = true );

private:
	void Initialize();
};

///////////////////////////////////////////////////////////////////////////////

#endif
