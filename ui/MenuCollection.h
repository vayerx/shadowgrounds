#ifndef INC_MENUCOLLECTION_H
#define INC_MENUCOLLECTION_H

#include <string>

class Ogui;
class OguiWindow;
class IOguiFont;

namespace game
{ 
	class Game; 
}

namespace ui
{
	
	class MenuCollectionImpl;

   /**
	* MenuCollection a collection of menus
	* This is the interface for the collection of menus 
	* ( mainmenu, options, loadgame )
	*
	* @version 0.1, 15.02.2005
	* @author Petri Purho
	*/
	class MenuCollection
	{
	public:

		enum MENU_TYPE
		{
			MENU_TYPE_MAINMENU = 0,
			MENU_TYPE_LOADGAMEMENU,
			MENU_TYPE_SURVIVALMENU,
			MENU_TYPE_COOPMENU,
			MENU_TYPE_PROFILESMENU,
			MENU_TYPE_OPTIONSMENU,
			MENU_TYPE_CREDITSMENU,
			MENU_TYPE_NEWGAMEMENU
		};

		struct FontStyle
		{
			FontStyle() :
				normal( NULL ),
				disabled( NULL ),
				down( NULL ),
				highlighted( NULL )
			{

			}
			
			IOguiFont* normal;
			IOguiFont* disabled;
			IOguiFont* down;
			IOguiFont* highlighted;
		};

		// The fonts used in menu collections
		struct Fonts
		{
			FontStyle big;
			FontStyle medium;
			FontStyle little;
		};

		MenuCollection( Ogui *ogui, game::Game *game, int player, bool start_automagicly = false, int menu_to_be_opened = 0 );
		~MenuCollection( );

		void start();

		void hide();
		void show();
		bool isVisible() const;

		bool wasQuitPressed(); // should be const

		// opens a new menu on top of the stack
		void openMenu( int menu );
		
		// closes the and pops the top from stack
		void closeMenu();

		// closes the current menu and opens a new one
		void changeMenu( int menu );

		// Starts mission number n
		void loadMission( int n );

		// Starts the very first mission (starts a new game)
		void newMission();

		void escPressed();

		// Sets the background image to the given image
		void setBackgroundImage( const std::string& background );

	private:
		MenuCollectionImpl* impl;
		
	};


}

#endif
