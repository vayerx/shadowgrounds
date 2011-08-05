#ifndef INC_MENUBASEIMPL_H
#define INC_MENUBASEIMPL_H

#include "IMenuBase.h"
#include "../ogui/IOguiButtonListener.h"
#include "../ogui/OguiButton.h"
#include "IGameControllerKeyreader.h"

#include <list>
#include <map>
#include <string>


class OguiWindow;
class OguiButtonEvent;
class OguiButton;
class Ogui;
class IOguiFont;
class IOguiImage;
class OguiTextLabel;

namespace game
{
	class Game;
}


namespace ui {

class MenuCollection;

struct SelectionButtonDescs
{
	SelectionButtonDescs() :
		first( NULL ),
		second( NULL )
	{ }

	OguiButton* first;
	OguiButton* second;
};

class MenuBaseImpl : public IMenuBase, public IOguiButtonListener, public IGameControllerKeyreader

{
public:
	explicit MenuBaseImpl( OguiWindow* window );
	virtual  ~MenuBaseImpl();

	virtual void hide();
	virtual void show();
	virtual void raise();
	virtual bool isVisible() const; 
	virtual bool wasQuitPressed() const;

	virtual void update();

	virtual void CursorEvent( OguiButtonEvent *eve );
	virtual void readKey( char ascii, int keycode, const char *keycodeName );
	virtual void escPressed();

protected:
	
	void debugKeyreader( int keyreader, bool release = false, std::string who = "MenuBaseImpl" );

	virtual void handleEsc();
	
	virtual OguiButton*	addButton( const std::string& text, int command, IOguiFont* font = NULL, IOguiFont* high = NULL, IOguiFont* down = NULL, IOguiFont* disa = NULL, OguiButton::TEXT_H_ALIGN halign = OguiButton::TEXT_H_ALIGN_CENTER );
	virtual OguiButton*	addDescription( const std::string& text, int x_add, int y_add, IOguiFont* font = NULL );
	virtual OguiButton*	addSmallButton( const std::string& text, int command, IOguiFont* font = NULL, IOguiFont* high = NULL, IOguiFont* down = NULL, IOguiFont* disa = NULL );
	virtual void		addSeparator();
	virtual OguiButton*	addImageButtton( const std::string& image_norm, const std::string& image_down, const std::string& image_high, const std::string& image_disa, int command, int x, int y, int w, int h );
	
	virtual void addSelectionButton( const std::string& text, int command, IOguiFont* font = NULL, void* param = NULL );

	virtual void addCloseButton( const std::string& text, int command, IOguiFont* font = NULL );

	virtual void addHeaderText( const std::string& text, IOguiFont* font = NULL );

	virtual void editButton( OguiButton* button, const std::string& defaultstring = "", const std::string& before_the_input = "" );
	virtual void editButtonEnter( const std::string& string );

	virtual void selectButton( int command );
	virtual void highlightSelectButton( int i );
	virtual void downlightSelectButton( int i );


	OguiWindow*		win;
	Ogui*					ogui;
	

	// Utilitys for the creation of the specific menus
	int				buttonX;
	int				buttonW;
	int				buttonH;
	int				buttonY;
	int				buttonAddX;
	int				buttonAddY;
	int				separatorH;
	int				separatorW;

	int				buttonDescriptionW;
	int				buttonDescriptionH;

	std::string		buttonNormal;
	std::string		buttonHigh;
	std::string		buttonDown;
	std::string		buttonPaddingString;

	std::string		smallButtonNormal;
	std::string		smallButtonHigh;
	std::string		smallButtonDown;
	std::string		smallButtonDisabled;
	IOguiImage*		smallButtonDisabledImage;

	int				smallButtonX;		// will mark the position of the buttons
	int				smallButtonY;		
	int				smallButtonStartAddX; // will be added to the button_x / button_y
	int				smallButtonStartAddY; // will be added to the button_x / button_y
	int				smallButtonW;
	int				smallButtonH;
	int				smallButtonAddX;
	int				smallButtonAddY;

	int				closeMeButtonX;
	int				closeMeButtonY;
	int				closeMeButtonW;
	int				closeMeButtonH;
	int				closeMeButtonAddX;
	int				closeMeButtonAddY;

	std::string		closeMeButtonNormal;
	std::string		closeMeButtonHigh;
	std::string		closeMeButtonDown;

	// std::map< int, std::pair< OguiButton*, OguiButton* > >	sliderButtons;
	
	std::list< OguiButton* >		buttons;
	std::map< int, OguiButton* >	selectButtons;
	int								numberOfWorkingSelectButtons;  // HACK

	int								activeSelection;

	IOguiImage*		imageSelectNorm;
	IOguiImage*		imageSelectDown;
	IOguiFont*		fontSelectNorm;
	IOguiFont*		fontSelectDown;
	IOguiFont*		fontDescNorm;
	IOguiFont*		fontDescDown;

	std::string		buttonFontSelectNormal;
	std::string		buttonFontSelectDown;
	std::string		buttonFontDescNormal;
	std::string		buttonFontDescDown;

	OguiTextLabel*	headerText;
	int				headerTextX;
	int				headerTextY;
	int				headerTextW;
	int				headerTextH;

	std::string		editBuffer;
	std::string		editBufferBefore;
	std::string		editBufferAfter;
	int				editHandle;
	OguiButton*		editButtonP;  // NULL if we dont have anything to edit
	bool			editCursorDrawn;
	int				editCursorDrawnTime;
	int				editCursorBlinkTime;

	// sounds
	std::string		soundClick;
	std::string		soundMouseover;
	std::string		soundDisabled;

	game::Game*				game;
	bool					closeMenuByEsc;
	bool					canWeCloseTheMenuNow;

};

}
#endif
