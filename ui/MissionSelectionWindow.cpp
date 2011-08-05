#include "precompiled.h"

#include "MissionSelectionWindow.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiSelectList.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../util/fb_assert.h"
#include "../game/Game.h"
#include "../game/scripting/GameScripting.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"


using namespace game;

namespace ui {


struct SingleMissionButton
{
	OguiButton* button;
	std::string missionId;
	int			guiId;
	std::string missionDescription;

	std::string missionScriptName;
	std::string missionSubName;

	// int			
};


const int MISSIONSELECTIONWINDOW_CLOSEME = 101;
const int MISSIONSELECTIONWINDOW_LOADMISSION = 102;

///////////////////////////////////////////////////////////////////////////////
class MissionSelectionWindow::MissionSelectionWindowImpl : private IOguiButtonListener, private IOguiSelectListListener
{
public:
	
	//=========================================================================
	MissionSelectionWindowImpl( Ogui* ogui, Game* game ) :
		ogui( ogui ),
		win( NULL ),
		game( game ),
		closebut( NULL ),
		loadMissionBut( NULL ),
		textArea( NULL ),
		selectList( NULL ),
		oguiLoader( ogui ),
		highlightedButton( 0 ),
		missionButtonIds( 0 ),
		missionButtons()
	{
		// Logger::getInstance()->debug( "ScoreWindowImpl" );
		oguiLoader.SetLogging( true, "missionselectionwindow.txt" );

		win = oguiLoader.LoadWindow( "missionselectionwindow" );

		closebut = oguiLoader.LoadButton( "closebutton", win, MISSIONSELECTIONWINDOW_CLOSEME );
		closebut->SetListener( this );
		closebut->SetDisabled( false );

		loadMissionBut = oguiLoader.LoadButton( "loadmissionbutton", win, MISSIONSELECTIONWINDOW_LOADMISSION );
		loadMissionBut->SetListener( this );
		loadMissionBut->SetDisabled( false );

		textArea = oguiLoader.LoadFormattedText( "textarea", win, 0 );

		selectList = oguiLoader.LoadSelectList( "selectlist", win, 0 );
		selectList->setListener( this );

		win->Raise();

		win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, 500);
		win->Show();

		// GenerateStats();
		LoadMissionButtonThingie( "1" );
		LoadMissionButtonThingie( "2" );

	}

	~MissionSelectionWindowImpl()
	{
		std::map< int, SingleMissionButton* >::iterator i;
		for( i = missionButtons.begin(); i!= missionButtons.end(); ++i )
		{
			delete i->second->button;
			delete i->second;
		}

		delete closebut;
		delete textArea;
		delete selectList;
		delete win;
	}
	//=========================================================================

	virtual void CursorEvent( OguiButtonEvent *eve )
	{
		if ( eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK )
		{
		    switch( eve->triggerButton->GetId() )
			{
			case MISSIONSELECTIONWINDOW_CLOSEME:
				game->getGameUI()->closeMissionSelectionWindow();
				break;
			case MISSIONSELECTIONWINDOW_LOADMISSION:
				ExecuteSelectedMission();
				game->getGameUI()->closeMissionSelectionWindow();
				break;
			default:
				{
					std::map< int, SingleMissionButton* >::iterator i = missionButtons.find( eve->triggerButton->GetId() );
					if( i != missionButtons.end() )
					{
						HighlightMission( eve->triggerButton->GetId() );
					}
				}
				break;

			};
			// Logger::getInstance()->debug( "CLOSE ME NOW" );
		}
	}
	
	//=========================================================================

	virtual void SelectEvent( OguiSelectListEvent* eve )
	{
		if( eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT )
			HighlightMission( eve->selectionNumber );
	}

	///////////////////////////////////////////////////////////////////////////
	
	void HighlightMission( int id )
	{
		std::map< int, SingleMissionButton* >::iterator i = missionButtons.find( highlightedButton );
		if( i != missionButtons.end() )
		{
			i->second->button->SetDisabled( false );
		}

		i = missionButtons.find( id );
		if( i != missionButtons.end() )
		{
			selectList->setSelected( highlightedButton, false );
			highlightedButton = id;
			textArea->setText( i->second->missionDescription );
			selectList->setSelected( id, true );
			i->second->button->SetDisabled( true );
		}
	}

	///////////////////////////////////////////////////////////////////////////

	void ExecuteSelectedMission()
	{
		std::map< int, SingleMissionButton* >::iterator i = missionButtons.find( highlightedButton );
		if( i != missionButtons.end() )
		{
			// game->setFailureMission(stringData);
			// game->loadGame( i->second->missionId.c_str() );
			game->gameScripting->runMissionScript( i->second->missionScriptName.c_str(), i->second->missionSubName.c_str() );
		}
	}

	///////////////////////////////////////////////////////////////////////////

	void Update()
	{
		// Logger::getInstance()->debug( "update" );
	}

	//=========================================================================

	void LoadMissionButtonThingie( const std::string& id )
	{
		int button_id = missionButtonIds;
		missionButtonIds++;
		
		SingleMissionButton* result = new SingleMissionButton;
		result->button = oguiLoader.LoadButton( id, win, button_id );
		result->button->SetListener( this );
	
		result->guiId = button_id;
		result->missionId = id;
		{
			std::string temp = "gui_missionselectionwindow_" + id + "_description";
			result->missionDescription = getLocaleGuiString( temp.c_str() );
			
			temp = "gui_missionselectionwindow_" + id + "_scriptname";
			result->missionScriptName = getLocaleGuiString( temp.c_str() );
			
			temp = "gui_missionselectionwindow_" + id + "_scriptsubname";
			result->missionSubName = getLocaleGuiString( temp.c_str() );
		}


		missionButtons.insert( std::pair< int, SingleMissionButton* >( button_id, result ) );

		selectList->addItem( id.c_str(), id.c_str() );
	}

	///////////////////////////////////////////////////////////////////////////

private:
	Ogui*		ogui;
	OguiWindow*	win;
	game::Game*	game;
	
	OguiButton*	closebut;
	OguiButton* loadMissionBut;
	OguiFormattedText* textArea;
	OguiSelectList*	selectList;

	OguiLocaleWrapper	oguiLoader;
	
	int highlightedButton;
	int missionButtonIds;
	std::map< int, SingleMissionButton* >	missionButtons;
};

///////////////////////////////////////////////////////////////////////////////

MissionSelectionWindow::MissionSelectionWindow( Ogui* ogui, game::Game* game ) :
	impl( NULL )
{
	// Logger::getInstance()->debug("ScoreWindow opened");
	impl = new MissionSelectionWindowImpl( ogui, game );
}

//=============================================================================

MissionSelectionWindow::~MissionSelectionWindow()
{
	// Logger::getInstance()->debug("ScoreWindow killed");
	delete impl;
	impl = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void MissionSelectionWindow::Update( int )
{
	fb_assert( impl );
	impl->Update();
}

///////////////////////////////////////////////////////////////////////////////

void MissionSelectionWindow::AddMissionButton( const std::string& id )
{
	fb_assert( impl );
	impl->LoadMissionButtonThingie( id );
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
