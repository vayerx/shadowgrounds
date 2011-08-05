
#include "precompiled.h"

#include "GUIEffectWindow.h"
#include "../ogui/Ogui.h"
#include "../system/SystemRandom.h"

#include <math.h>

namespace ui
{

	GUIEffectWindow::GUIEffectWindow(Ogui *ogui, const char *layer1Image,
		const char *layer2Image, const char *layer3Image, const VC2I &position, const VC2I &size)
	{
		this->ogui = ogui;
		this->effectTime = 0;
		this->lastNoiseScrollX = 0.0f;
		this->lastNoiseScrollY = 0.0f;

		this->win1 = ogui->CreateSimpleWindow(position.x, position.y, size.x, size.y, layer1Image);
		this->win2 = ogui->CreateSimpleWindow(position.x, position.y, size.x, size.y, layer2Image);
		this->win3 = ogui->CreateSimpleWindow(position.x, position.y, size.x, size.y, layer3Image);

		// added by Pete
		win1->SetReactMask( 0 );
		win2->SetReactMask( 0 );
		win3->SetReactMask( 0 );

		// hax hax
		if(size.x != 1024)
		{
			win3->setBackgroundRepeatFactor(2.0f,2.0f);
			win2->setBackgroundRepeatFactor(2.0f,2.0f);
			charWindow = true;
		}
		else
		{
			win1->setBackgroundRepeatFactor(0.2f,0.2f);
			charWindow = false;
		}
	}
				
	GUIEffectWindow::~GUIEffectWindow()
	{
		if (this->win3 != NULL)
		{
			delete this->win3;
		}
		if (this->win2 != NULL)
		{
			delete this->win2;
		}
		if (this->win1 != NULL)
		{
			delete this->win1;
		}
	}

	void GUIEffectWindow::update(int msecTimeDelta)
	{
		this->effectTime += msecTimeDelta;

		if (win3 != NULL)
		{
			win3->setBackgroundScroll(0, 1.0f - float(this->effectTime) / 50000.0f);
		}
		if (win2 != NULL)
		{
			//win2->setBackgroundScroll(0, float(msecTimeDelta) / 1000.0f);
		}
		if (win1 != NULL)
		{
			float x = (SystemRandom::getInstance()->nextInt() % 100) / 100.0f;
			float y = (SystemRandom::getInstance()->nextInt() % 100) / 100.0f;
			if (fabs(x - lastNoiseScrollX) < 0.2f)
				x += 0.3f;
			if (fabs(y - lastNoiseScrollY) < 0.2f)
				y += 0.3f;
			win1->setBackgroundScroll(x, y);

if(!charWindow)
				win1->setBackgroundRepeatFactor(4.0f,4.0f);

			win1->SetTransparency(50);
		}
	}

	void GUIEffectWindow::show()
	{
		if(win1)
			win1->Show();
		if(win2)
			win2->Show();
		if(win3)
			win3->Show();
	}

	void GUIEffectWindow::hide()
	{
		if(win1)
			win1->Hide();
		if(win2)
			win2->Hide();
		if(win3)
			win3->Hide();
	}

	void GUIEffectWindow::raise()
	{
		if (win1 != NULL)
		{
			win1->Raise();
		}
		if (win2 != NULL)
		{
			win2->Raise();
		}
		if (win3 != NULL)
		{
			win3->Raise();
		}
	}

	void GUIEffectWindow::setTransparency(int value, int value2)
	{
		if(win1)
			win1->SetTransparency(value);
		if(win2)
			win2->SetTransparency(value2);
		if(win3)
			win3->SetTransparency(value2);
	}

	void GUIEffectWindow::fadeIn(int time)
	{
		if(win1)
			win1->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, time);
		if(win2)
			win2->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, time);
		if(win3)
			win3->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, time);
	}

	void GUIEffectWindow::fadeOut(int time)
	{
		if(win1)
			win1->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, time);
		if(win2)
			win2->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, time);
		if(win3)
			win3->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, time);
	}
}

