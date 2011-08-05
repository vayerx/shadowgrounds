#ifndef INC_ELABORATEHINTMESSAGEWINDOW_H
#define INC_ELABORATEHINTMESSAGEWINDOW_H

#include <string>

#include "ICombatSubWindow.h"

namespace game {
	class Game;
}
class OguiFormattedText;
class Ogui;

namespace ui {

class ElaborateHintMessageWindow : public ICombatSubWindow
{
public:
	ElaborateHintMessageWindow( Ogui* ogui, game::Game* game, int player_num );
	~ElaborateHintMessageWindow();

	void hide( int time = 0 );
	void show( int time = 0 );
	void update();

	void setStyle( const std::string& style );
	void showMessage( const std::string& message );
	
	void showMessage( const std::string& style, const std::string& message );

	OguiFormattedText* getFormattedTextForStyle( const std::string &style );
	
private:
	class ElaborateHintMessageWindowImpl;
	ElaborateHintMessageWindowImpl* impl;

	std::string style;
};

} // end of namespace ui

#endif
