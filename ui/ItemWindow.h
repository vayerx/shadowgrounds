#ifndef INC_ITEMWINDOW_H
#define INC_ITEMWINDOW_H

#include <string>
#include <list>
#include <map>

#include "GuiItem.h"

#include "../ogui/Ogui.h"
#include "ICombatSubWindow.h"

namespace game 
{
	class Game;
}


namespace ui {

///////////////////////////////////////////////////////////////////////////////

// Represents items player has on the user interface
class ItemWindow : public ICombatSubWindow, private IOguiEffectListener
{
public:
	ItemWindow( Ogui *ogui, game::Game *game, int player );
	virtual ~ItemWindow();

	typedef ::ui::GuiItem Item;

	void addItem( const std::string& target, const Item& item );
	void removeItem( const std::string& target, const Item& item );

	
	void hide( int fadeTime = 0 );
	void show( int fadeTime = 0 );
	void update();
	// void updateCurve();
	void EffectEvent( OguiEffectEvent *e );

private:

	///////////////////////////////////////////////////////////////////////////

	class ItemList
	{
	public:
		ItemList( int x, int y, int addX, int addY );
		~ItemList();

		void addItem( const Item& item );
		void removeItem( const Item& item );

	private:
		
		int startX;
		int startY;
		int curX;
		int curY;
		int addX;
		int addY;
		
		std::list< Item > items;
	};

	///////////////////////////////////////////////////////////////////////////

	void addItemWindow( const std::string& item_window_name );

	///////////////////////////////////////////////////////////////////////////

	std::map< std::string, ItemList* >	itemWindows;
	Ogui*			ogui;
	game::Game*		game;
	OguiWindow*		window;
	int				windowX;
	int				windowY;
	float			lastUpdateValue;

};

///////////////////////////////////////////////////////////////////////////////
}

#endif
