#ifndef INC_COMBATMESSAGEWINDOWWITHHISTORY_H
#define INC_COMBATMESSAGEWINDOWWITHHISTORY_H

#include "CombatMessageWindow.h"

#include <string>
#include <vector>

namespace game
{
	class Game;
}

class Ogui;

namespace ui
{


class CombatMessageWindowWithHistory : public CombatMessageWindow
{
public:


	CombatMessageWindowWithHistory( Ogui* ogui, const std::string& textConfName, const std::string& iconConfName );
	~CombatMessageWindowWithHistory();

	void showMessage(const char *messa, Visual2D *image);
	void clearMessage();

	void hide( int fadeTime = 0 );
	void show( int fadeTime = 0 );

	void update();
	void clearOneMessage();

	

private:
	
	void pushMessage( const std::string& message );
	void popMessage();

	int		getVerticalPosition( int i );
	float	getAlphaValue( int i );

	struct TextMessage;
	std::vector< TextMessage* >	messageBuffer;

	int numOfMessages;
	int textHeight;
	
	int messageStartFadeTime;
	int messageEndFadeTime;

	bool visible;

};

} // end of namespace ui

#endif
