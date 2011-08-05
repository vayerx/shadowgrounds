#ifndef INC_OGUICHECKBOX_H
#define INC_OGUICHECKBOX_H

#include <string>
#include "OguiButton.h"

class Ogui;
class OguiWindow;
class OguiButtonEvent;
class IOguiCheckBoxListener;
class IOguiImage;
class IOguiFont;
class OguiTextLabel;
class OguiCheckBox;

#include "IOguiButtonListener.h"

class OguiCheckBoxEvent
{
public:
	OguiCheckBox* checkBox;
	bool		  value;
};

class IOguiCheckBoxListener
{
public:
	virtual ~IOguiCheckBoxListener() { }

	virtual void checkBoxEvent( OguiCheckBoxEvent* eve ) = 0;
};


class OguiCheckBox : public IOguiButtonListener
{
public:

	enum TEXT_ALIGN
	{
		TEXT_ALIGN_LEFT = 0,
		TEXT_ALIGN_RIGHT = 1
	};

	OguiCheckBox( OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, 
		const std::string& checkbox_false_norm, std::string checkbox_false_down, std::string checkbox_false_high, 
		const std::string& checkbox_true_norm, const std::string& checkbox_true_down = "", const std::string& checkbox_true_high = "", int sid = 0, bool value = true, bool changeOnClick = true );
	~OguiCheckBox();

	//.........................................................................

	bool getValue() const;
	void setValue( bool v );

	// ........................................................................
	int getId() const;
	
	int getHeight() const;

	void setListener( IOguiCheckBoxListener* listener );

	void setText( const std::string& text, TEXT_ALIGN align = TEXT_ALIGN_RIGHT, int w = 200, IOguiFont* font = NULL, OguiButton::TEXT_V_ALIGN valign = OguiButton::TEXT_V_ALIGN_TOP );

	// ........................................................................
	
	void CursorEvent( OguiButtonEvent *eve );

private:
	std::string breakText( const std::string& text, IOguiFont* font );

	const int buttonId;
	const int textId;

	Ogui*			ogui;
	OguiWindow*		win;
	
	IOguiCheckBoxListener*	listener;

	OguiButton*				textButton;
	int						textW;
	int						textH;
	TEXT_ALIGN				textAlign;

	bool	value;
	int		id;
	bool	changeOnClick;

	OguiButton*	button;
	int			buttonX;
	int			buttonY;
	int			buttonW;
	int			buttonH;

	IOguiImage*	button_false_norm;
	IOguiImage*	button_false_down;
	IOguiImage*	button_false_high;
	IOguiImage*	button_true_norm;
	IOguiImage*	button_true_down;
	IOguiImage*	button_true_high;

};


#endif
