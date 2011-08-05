
#include "precompiled.h"

#include "CombatMessageWindowWithHistory.h"
#include "../system/Timer.h"
#include "../game/DHLocaleManager.h"

using namespace game;

namespace ui {

///////////////////////////////////////////////////////////////////////////////

struct CombatMessageWindowWithHistory::TextMessage
{
	OguiTextLabel* text;
	int			initTime;
	float		alpha;
};

///////////////////////////////////////////////////////////////////////////////

CombatMessageWindowWithHistory::CombatMessageWindowWithHistory( Ogui* ogui, const std::string& textConfName, const std::string& iconConfName ) :
	CombatMessageWindow( ogui, NULL, 0, textConfName.c_str(), iconConfName.c_str() ),
	messageBuffer(),
	numOfMessages( 1 ),
	textHeight( 0 ),
	messageStartFadeTime( 0 ),
	messageEndFadeTime( 0 )
{

	visible = true;

	numOfMessages =			getLocaleGuiInt( ( "gui_message_" + textConfName + "_number_of_elements" ).c_str(), 1 );
	textHeight =			getLocaleGuiInt( ( "gui_message_" + textConfName + "_text_height" ).c_str(), 20 );
	messageStartFadeTime =	getLocaleGuiInt( ( "gui_message_" + textConfName + "_fade_start_time").c_str(), 5000 );
	messageEndFadeTime =	getLocaleGuiInt( ( "gui_message_" + textConfName + "_fade_end_time").c_str(), 6000 );

	if( numOfMessages < 1 )
		numOfMessages = 1;

	messageBuffer.resize( numOfMessages );
	int i = 0;
	for( i = 0; i < (int)messageBuffer.size(); i++ )
	{
		messageBuffer[ i ] = NULL;
	}
}

//=============================================================================

CombatMessageWindowWithHistory::~CombatMessageWindowWithHistory()
{
	int i = 0;
	for( i = 0; i < (int)messageBuffer.size(); i++ )
	{
		if( messageBuffer[ i ] != NULL )
		{
			delete messageBuffer[ i ]->text;
			messageBuffer[ i ]->text = NULL;
		}

		delete messageBuffer[ i ];
		messageBuffer[ i ] = NULL;

		
	}
}

///////////////////////////////////////////////////////////////////////////////

void CombatMessageWindowWithHistory::showMessage( const char* messa, Visual2D* image )
{
	const std::string& msg = messa;

	pushMessage( msg );
}

//=============================================================================

void CombatMessageWindowWithHistory::clearMessage()
{
	// popMessage();
}

///////////////////////////////////////////////////////////////////////////////

void CombatMessageWindowWithHistory::update()
{

	int i = 0;
	for( i = 0; i < (int)messageBuffer.size(); i++ )
	{
		if( messageBuffer[ i ] != NULL && messageBuffer[ i ]->text != NULL )
		{
			float transparency = ( (float)( ( Timer::getTime() - messageBuffer[ i ]->initTime  ) - messageStartFadeTime ) / ( messageEndFadeTime - messageStartFadeTime ) ) * messageBuffer[ i ]->alpha * 100.0f;
			messageBuffer[ i ]->text->SetTransparency( (int)transparency );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CombatMessageWindowWithHistory::pushMessage( const std::string& message )
{
	if( messageBuffer[ 0 ] != NULL ) 
		popMessage();

	messageBuffer[ 0 ] = new TextMessage;
	messageBuffer[ 0 ]->initTime = Timer::getTime();
	messageBuffer[ 0 ]->text = ogui->CreateTextLabel( win, textXPosition, getVerticalPosition( 0 ), textXSize, textHeight, message.c_str() );

	messageBuffer[ 0 ]->text->SetFont( textFont );
	messageBuffer[ 0 ]->alpha = 1.0f;

}

//=============================================================================

void CombatMessageWindowWithHistory::popMessage() 
{
	int i = 0;
	TextMessage* msg = messageBuffer[ messageBuffer.size() - 1 ];
	for ( i = (int)messageBuffer.size() - 1; i > 0; i-- )
	{
		messageBuffer[ i ] = messageBuffer[ i - 1];
		if( messageBuffer[ i ] && messageBuffer[ i ]->text )
		{
			// float alpha = messageBuffer[ i ]->alpha;
			// alpha *= 0.5f;
			// messageBuffer[ i ]->alpha = alpha;
			// messageBuffer[ i ]->text->SetTransparency( (int)( 100.0f * alpha)  );
			messageBuffer[ i ]->text->Move( textXPosition, getVerticalPosition( i ) );
		}
	}

	messageBuffer[ 0 ] = NULL;

	if( msg )
	{
		delete msg->text;
		delete msg;
	}

}

///////////////////////////////////////////////////////////////////////////////

void CombatMessageWindowWithHistory::hide( int fadeTime  )
{
	visible = false;
	CombatMessageWindow::hide( fadeTime );
}

//=============================================================================

void CombatMessageWindowWithHistory::show( int fadeTime  )
{
	visible = true;
	CombatMessageWindow::show( fadeTime );
}

///////////////////////////////////////////////////////////////////////////////

int CombatMessageWindowWithHistory::getVerticalPosition( int i )
{
	return this->textYPosition + ( this->textHeight * ( ( numOfMessages - 1 ) - i ) );
}

//=============================================================================

float CombatMessageWindowWithHistory::getAlphaValue( int i )
{
	i++;
	if ( i == 0 ) return 1.0f;
	return 1.0f / (float)i;
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
