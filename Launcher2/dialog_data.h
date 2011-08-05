#ifndef INC_DIALOG_DATA_H
#define INC_DIALOG_DATA_H

#include <windows.h>

namespace frozenbyte {
namespace launcher {

class Window;
class IDlgHandler;

class DialogDataImpl;

class DialogData
{
public:

	enum ResizeType
	{
		ATTACH_NONE = 0,
		ATTACH_RIGHT = 1,
		ATTACH_BOTTOM = 2,
		ATTACH_ALL = 3
	};	

	DialogData(int resourceId, const HWND parentHandle, ResizeType type = ATTACH_NONE, IDlgHandler* handler = 0 );
	explicit DialogData(int resourceId); 
	~DialogData();

	void getSize(int &x, int &y) const;
	void getPosition( int &x, int &y) const;

	void updateSize();
	void setPosition( int x, int y );
	void setSize( int width, int height );
	void forceDialogRect( RECT &rc );
	void show();
	void hide();

	void setDialogHandler( IDlgHandler* handler );

	HWND getHwnd();

private:
	DialogDataImpl* impl;
};

} // end of namespace launcher
} // end of namespace frozenbyte

#endif