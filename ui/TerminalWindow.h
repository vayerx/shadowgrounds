#ifndef INC_TERMINALWINDOW_H
#define INC_TERMINALWINDOW_H

#include <string>
#include <vector>
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"

namespace game
{
	class Game;
}

namespace ui {


// A window that is opened when a terminal / computer / log etc is executed
class TerminalWindow : public IOguiButtonListener
{
public:
	TerminalWindow( Ogui* ogui, game::Game* game, const std::string& style_name );
	~TerminalWindow();
	
	void setText( const std::string& txt );
	void setPage( int page );
	void nextPage();
	void prevPage();

	bool isVisible() const;

	void update();

	void CursorEvent( OguiButtonEvent* eve );
	

private:
	OguiButton* loadButton( const std::string& name, int id = 0 );
	OguiTextLabel* loadText( const std::string& name );

	game::Game* game;
	Ogui*		ogui;

	OguiWindow*			window;
	OguiFormattedText*	textarea;
	OguiButton*			next;
	OguiButton*			prev;
	OguiButton*			close;
	OguiButton*			pageNumButton;
	OguiButton*			vsyncImage1;
	OguiButton*			vsyncImage2;
	float				vsyncY1;
	float				vsyncY2;
	int					vsyncX;
	float				vsyncSpeed1;
	float				vsyncSpeed2;
	int					vsyncBeginPos;
	int					vsyncDownPos;
	int					pageNum;
	int					pageMax;
	bool				visible;

	std::vector< IOguiFont* >	fonts;
	std::vector< std::string >	text;
	
	std::vector<OguiTextLabel*> buttonTexts;
	
};

}

#endif
