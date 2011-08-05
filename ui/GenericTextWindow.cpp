#include "precompiled.h"

#include "GenericTextWindow.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../ui/CombatWindow.h"
#include "../ui/ElaborateHintMessageWindow.h"
#include "../game/Unit.h"
#include "../ogui/OguiButton.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../system/Timer.h"

#include "../game/DHLocaleManager.h"
#include "CombatSubWindowFactory.h"

using namespace game;

namespace ui {

	GenericTextWindow *GenericTextWindow::last_opened_window = NULL;

	GenericTextWindow::GenericTextWindow( Ogui* ogui, game::Game* game, int player )
		: ogui(ogui), game(game), player(player), reallyHidden(false)
	{
		oguiLoader = new OguiLocaleWrapper(ogui);
		win = NULL;
		text = NULL;
		updator = NULL;
		last_opened_window = this;
	}

	GenericTextWindow::~GenericTextWindow()
	{
		if(last_opened_window == this)
			last_opened_window = NULL;

		delete updator;
		delete text;
		delete win;
		delete oguiLoader;
	}

	void GenericTextWindow::loadDataFromLocales( const std::string& locale_name )
	{
		win = oguiLoader->LoadWindow(locale_name);
		win->SetUnmovable();
		text = oguiLoader->LoadFormattedText("text", win, 0);
	}

	void GenericTextWindow::setUpdator(IGenericTextWindowUpdator *updator_)
	{
		updator = updator_;
		update();
	}

	void GenericTextWindow::hide( int time )
	{
		if(win)
			win->Hide();
		reallyHidden = true;
	}

	void GenericTextWindow::show( int time )
	{
		if(win)
			win->Show();
		reallyHidden = false;
	}

	void GenericTextWindow::update()
	{
		if(updator)
			updator->update(this);
	}

	void GenericTextWindow::setText(const std::string &str)
	{
		if(text)
			text->setText(str);
	}

	void GenericTextWindow::move(int x, int y)
	{
		win->MoveTo(x, y);
	}

	int GenericTextWindow::getX()
	{
		return win->GetPositionX();
	}

	int GenericTextWindow::getY()
	{
		return win->GetPositionY();
	}
}
