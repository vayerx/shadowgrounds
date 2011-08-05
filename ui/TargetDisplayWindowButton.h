#ifndef INC_TARGETDISPLAYWINDOWBUTTON_H
#define INC_TARGETDISPLAYWINDOWBUTTON_H

#include "TargetDisplayButtonManager.h"

#include <vector>
#include <string>

class OguiButton;
class Ogui;
class OguiWindow;


namespace ui
{

enum directions
{
	topleft = 0,
	topright = 1,
	bottomright = 2,
	bottomleft = 3,
	buttontext = 4
};

class TargetDisplayWindowButton
{
public:

	typedef TargetDisplayButtonManager::ButtonData button;

	TargetDisplayWindowButton();
	TargetDisplayWindowButton( int style );
	TargetDisplayWindowButton( const TargetDisplayWindowButton& button );
	virtual ~TargetDisplayWindowButton();

	// virtual
	unsigned int updatedInTick;

	void operator=( const TargetDisplayWindowButton& button );

	virtual void updateRect(void);
	virtual void setRect( int x, int y, int w, int h, float distance = 0.0f );
	virtual void setText( const std::string& text );
	virtual void setSliderValue( float v, float scale );

	virtual bool hasSlider() const;

	virtual void hide();
	void show();

	virtual bool hasEnded() const { return false; }

	virtual bool isAniOver() const { return beginAnimPos == 0.0f; }
	virtual int timeActive() const;

	virtual void resetAni();
	virtual void release();

	bool isEmpty() const;

	virtual int  getStyle() const;

	static void setManager( TargetDisplayButtonManager* manager )
	{
		buttonManager = manager;
	}

	static bool isRegistered( int style )
	{
		if( buttonManager )
		{
			return buttonManager->isRegistered( style );
		}
		else
		{
			return false;
		}
	}
	
protected:
	
	void setButtonPosition( int i, int x, int y, int transparency );
	void setButtonTextPosition( int x, int y, int bottomx, int transparency );
	void setButtonSliderPosition( int x, int y, int bottomx, int transparency );
	void hideButton( int i );
	void showButton( int i );
	void hideSlider();
	void showSlider();

	float getAnimPos( int x );

	button b;
	
	float beginAnimInitialPosition;
	float beginAnimPos;
	int imageWidth;
	int imageHeight;
	int style;
	unsigned int	startTicks;
	int createTime;

	int oldW;
	int oldH;

	static TargetDisplayButtonManager* buttonManager;

};

class TargetDisplayRisingScoreButton : public TargetDisplayWindowButton
{
public:
	TargetDisplayRisingScoreButton() { amIDead = false; }
	TargetDisplayRisingScoreButton( int style ) : TargetDisplayWindowButton( style ) { amIDead = false; }
	~TargetDisplayRisingScoreButton() { }
	
	virtual void updateRect(void);
	virtual void setRect( int x, int y, int w, int h, float distance = 0.0f );
	virtual bool hasEnded() const { return amIDead; }

	bool amIDead;
	int oldx;
	int oldy;
	int oldw;
	int oldh;
};

} // end of namespace ui
#endif
