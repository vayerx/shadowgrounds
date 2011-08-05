#ifndef INC_CREDITSMENU_H
#define INC_CREDITSMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"

class OguiFormattedText;
class IOguiFont;

namespace game
{
	class Game;
}

namespace ui
{

extern std::string G_UiCredits;

class CreditsMenu : public MenuBaseImpl
{
public:
	//.........................................................................

	enum COMMANDS 
	{
		COMMANDS_CLOSEME
	};

	//.........................................................................
	
	CreditsMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* g );
	~CreditsMenu();

	//.........................................................................

	int getType() const;

	//.........................................................................
	
	void closeMenu();
	void openMenu( int m );
	void applyChanges();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );
	void update();

private:
	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;

	std::vector<IOguiFont *> credits_fonts;
	OguiWindow*				maskWindow;
	OguiWindow*				textWindow;
	OguiFormattedText*		theText;
	float					yPosition;
	int						lastYPosition;
	int						lastUpdate;
	float					speed;
};


} // end of namespace


#endif
