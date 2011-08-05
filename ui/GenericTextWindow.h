#ifndef GENERICTEXTWINDOW_H_
#define GENERICTEXTWINDOW_H_

class Ogui;
class OguiWindow;
class OguiLocaleWrapper;
class OguiFormattedText;

#include <string>
#include "../ogui/Ogui.h"
#include "ICombatSubWindow.h"


namespace game
{
	class Game;
}

namespace ui
{
	class GenericTextWindow;

	class IGenericTextWindowUpdator
	{
	public:
		virtual ~IGenericTextWindowUpdator() {}
		virtual void update(GenericTextWindow *win) = 0;
	};

	class GenericTextWindow : public ICombatSubWindow
	{
	public:
		GenericTextWindow( Ogui* ogui, game::Game* game, int player );
		~GenericTextWindow();

		void setUpdator(IGenericTextWindowUpdator *updator);
		void loadDataFromLocales( const std::string& locale_name );

		void setText(const std::string &str);

		inline OguiFormattedText *getText() { return text; }
		inline OguiWindow *getWindow() { return win; }

		inline bool isHidden() const { return reallyHidden; }
		void hide( int time = 0 );
		void show( int time = 0 );
		void update();

		void move(int x, int y);
		int getX();
		int getY();

		static GenericTextWindow *last_opened_window;

	private:
		Ogui* ogui;
		game::Game* game;
		int player;

		OguiWindow* win;
		OguiFormattedText *text;
		OguiLocaleWrapper *oguiLoader;

		bool reallyHidden;
		IGenericTextWindowUpdator *updator;
	};

}
#endif
