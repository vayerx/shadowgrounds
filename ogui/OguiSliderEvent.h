#ifndef INC_OGUISLIDEREVENT_H
#define INC_OGUISLIDEREVENT_H

class OguiSlider;

struct OguiSliderEvent
{
	enum EVENT_TYPE
	{
		EVENT_TYPE_MOUSEDOWN,	// this gets called when mouse is pressed down on the button
		EVENT_TYPE_RELEASE,		// this gets called when mouse button is released or 
								// currently if mouse is moved off from the slider
		EVENT_TYPE_MOUSEDRAGGED // this gets called when the slider value is changed, by 
								// mouse dragging
	};

	OguiSliderEvent( OguiSlider* button, float value, EVENT_TYPE type ) : 
		button( button ), 
		value( value ), 
		type( type ) 
	{ }
      
	OguiSlider* button;
	float		value;
	EVENT_TYPE	type;

};

#endif
