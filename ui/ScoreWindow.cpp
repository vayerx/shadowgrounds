#include "precompiled.h"

#include "ScoreWindow.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../util/fb_assert.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/GameProfiles.h"
#include "../game/GameStats.h"
#include "../game/scripting/GameScripting.h"
#include "../game/DHLocaleManager.h"
#include "../util/StringUtil.h"
#include "../game/options/options_gui.h"
#include "../game/options/options_players.h"
#include "../ui/UIEffects.h"
#include "GameController.h"

#include <map>
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace game;

namespace ui {

const int SCOREWINDOW_CLOSEME = 1;
const int SCOREWINDOW_RESTART = 2;
const int SCOREWINDOW_NAME = 3;

///////////////////////////////////////////////////////////////////////////////
class ScoreWindow::ScoreWindowImpl : private IOguiButtonListener, public IGameControllerKeyreader
{
public:
	
	//=========================================================================
	ScoreWindowImpl( Ogui* ogui, Game* game, int player ) :
		ogui( ogui ),
		win( NULL ),
		game( game ),
		allowLoading( false ),
		player( player ),
		closebut( NULL ),
		restartbut( NULL ),
		textArea( NULL ),
		
		scoreLimitHint( NULL ),
		oguiLoader( ogui ),
		pressed_restart( false )
	{
		// Logger::getInstance()->debug( "ScoreWindowImpl" );
		oguiLoader.SetLogging( false, "scorewindow.txt" );

		win = oguiLoader.LoadWindow( "scorewindow" );

		const char *empty = "";
		clickCatcher = ogui->CreateSimpleTextButton(win, 0, 0, 1024, 768, empty, empty, empty, empty);
		clickCatcher->SetListener(this);
		clickCatcher->SetDisabled(false);


		title = oguiLoader.LoadButton( "title", win, 0 );
		title->SetDisabled(true);

		textArea = oguiLoader.LoadFormattedText( "textarea", win, 0 );
		originalStatsText = getLocaleGuiString( "gui_scorewindow_textarea_text" );

		highScores = oguiLoader.LoadFormattedText( "highscores", win, 0);
		highScoreNumbers = oguiLoader.LoadFormattedText( "highscores", win, 0);
		highScoreNumbers->setTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);

		counterIncSound = getLocaleGuiString( "gui_scorewindow_sound_counter_inc" );
		counterDoneSound = getLocaleGuiString( "gui_scorewindow_sound_counter_done" );
		
		newHighScoreHint = oguiLoader.LoadFormattedText( "newscore", win, 0);
		newHighScoreHintText = getLocaleGuiString( "gui_scorewindow_newscore_text" );
		newHighScoreHint->setText("");


		win->Raise();

		win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, 500);
		win->Show();

		setPlayer(player, -1);

		scorefader = NULL;

		name = oguiLoader.LoadButton( "nameinput", win, SCOREWINDOW_NAME );
		name->SetDisabled(true);

		nametext = oguiLoader.LoadButton( "nametext", win, 0 );
		nametext->SetDisabled(true);
		originalNameText = *nametext->getText();
		nametext->SetText("");


		closebut = NULL;
		restartbut = NULL;
		/*closebut = oguiLoader.LoadButton( "closebutton", win, SCOREWINDOW_CLOSEME );
		closebut->SetListener(this);
		closebut->SetDisabled(false);

		restartbut = oguiLoader.LoadButton( "restartbutton", win, SCOREWINDOW_RESTART );
		restartbut->SetListener(this);
		restartbut->SetDisabled(false);*/


		editHandle = -1;
		editcursor_show = true;
		editcursor_last_change = game->gameTimer;

		scrollScores = 0;
	}

	~ScoreWindowImpl()
	{
		delete scoreLimitHint;
		for(unsigned int i = 0; i < starButtons.size(); i++)
		{
			delete starButtons[i];
		}

		if(editHandle != -1)
		{
			game->gameUI->getController(0)->removeKeyreader( editHandle );
		}
		delete scorefader;
		delete restartbut;
		delete closebut;
		delete clickCatcher;
		delete title;
		delete textArea;
		delete newHighScoreHint;
		delete highScores;
		delete highScoreNumbers;
		delete name;
		delete nametext;
		delete win;
	}
	//=========================================================================

	virtual void CursorEvent( OguiButtonEvent *eve )
	{
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			if(eve->triggerButton->GetId() == SCOREWINDOW_CLOSEME)
			{
				Close();
			}
			else if(eve->triggerButton->GetId() == SCOREWINDOW_RESTART)
			{
				pressed_restart = true;
				Close();
			}
			else
			{
				if(game && game->gameTimer - creationTime > GAME_TICKS_PER_SECOND)
				{
					// clicking anywhere else makes the counters finish really quickly
					if(scoreCounters[4].current == 0)
					{
						lastCounterUpdate = 0;
						for(unsigned int i = 0; i < scoreCounters.size(); i++)
						{
							scoreCounters[i].current = scoreCounters[i].target - 1;
							scoreCounters[i].wait = 0;
						}
					}
				}
			}
		}
	}

	void editButtonEnter()
	{
		if(editBuffer.empty())
		{
			return;
		}

		game->gameUI->getController(0)->removeKeyreader( editHandle );
		//game->gameUI->playGUISound( counterDoneSound.c_str() );
		name->SetText("");
		nametext->SetText("");

		// show players score
		generateScoreList(true);
		editHandle = -1;
	}

	void readKey(char ascii, int keycode, const char *keycodeName)
	{
		switch( keycode )
		{
		case 1: // esc
			editButtonEnter();
			return;
			break;

		case 14: // backspace
			if( !editBuffer.empty() )
			{
				editBuffer.erase( editBuffer.size() - 1 );
				//game->gameUI->playGUISound( counterIncSound.c_str() );
			}
			break;

		case 28: // enter
			editButtonEnter();
			return;
			break;

		default:
			if ( ascii != '\0' && editBuffer.length() < 20)
			{
				editBuffer += ascii;
				//game->gameUI->playGUISound( counterIncSound.c_str() );
			}
			break;
		}
		name->SetText((">" + editBuffer + "_").c_str());
		editcursor_show = true;
		editcursor_last_change = game->gameTimer;
	}
	
	///////////////////////////////////////////////////////////////////////////
	
	void GenerateStats()
	{
		game::GameStats* stats = game::GameStats::instances[ player ];

		// mark death here if not already earlier..
		stats->markTimeOfDeath();

		int time, kills, scoreTime, scoreKills, totalScore;
		stats->getScoreValues(time, kills, scoreTime, scoreKills, totalScore);

		scoreCounters.resize(5);
		scoreCounters[0].current = 0;
		scoreCounters[0].target = scoreTime;
		scoreCounters[0].step = scoreTime / GAME_TICKS_PER_SECOND; // calc for 1 secs
		if(scoreCounters[0].step < 1) scoreCounters[0].step = 1;
		scoreCounters[0].wait = GAME_TICKS_PER_SECOND / 2; // chill afterwards
		
		scoreCounters[1].current = 0;
		scoreCounters[1].target = scoreKills;
		scoreCounters[1].step = scoreKills / GAME_TICKS_PER_SECOND; // calc for 1 secs
		if(scoreCounters[1].step < 1) scoreCounters[1].step = 1;
		scoreCounters[1].wait = 3 * GAME_TICKS_PER_SECOND / 4; // chill afterwards

		scoreCounters[2].current = 0;
		scoreCounters[2].step = totalScore;
		scoreCounters[2].target = totalScore;
		scoreCounters[2].wait = GAME_TICKS_PER_SECOND / 2; // chill afterwards

		scoreCounters[3].current = 0;
		scoreCounters[3].step = 1;
		scoreCounters[3].target = 1;
		scoreCounters[3].wait = GAME_TICKS_PER_SECOND / 4;

		scoreCounters[4].current = 0;
		scoreCounters[4].step = 1;
		scoreCounters[4].target = 1;
		scoreCounters[4].wait = GAME_TICKS_PER_SECOND;


		if( stats != NULL )
		{
			std::string text = originalStatsText;
			text = util::StringReplace("($name)", game->getGameProfiles()->getCurrentProfile(player), text);
			text = util::StringReplace("($time)", time2str(time), text);
			text = util::StringReplace("($kills)", int2str(kills), text);

			modifiedStatsText = text;

			text = util::StringReplace("($totalscore)", "0", text);
			text = util::StringReplace("($scoretime)", "0", text);
			text = util::StringReplace("($scorekills)", "0", text);
			textArea->setText( text );
			
		}

		if(!game::SimpleOptions::getBool(DH_OPT_B_SCOREWINDOW_NAME_INPUT))
		{
			generateScoreList(false);
		}
	}

	void generateScoreList(bool showPlayerScore)
	{
		game::GameStats* stats = game::GameStats::instances[ player ];

		unsigned int myPos = 0;
		std::vector<GameStats::ScoreData> scores;

		// take name from profile
		if(!game::SimpleOptions::getBool(DH_OPT_B_SCOREWINDOW_NAME_INPUT))
			editBuffer = game->getGameProfiles()->getCurrentProfile(player);

		stats->updateScoreList(scores, myPos, editBuffer);

		std::stringstream ss;
		std::stringstream ss2;
		for(unsigned int i = 0; i < scores.size(); i++)
		{
			// this is my score
			if(i == myPos)
			{
				if(!showPlayerScore) continue;

				if(i >= 2)
				{
					// scroll
					int offsetY = highScores->getFont()->getHeight() * 4;
					scrollScores += (i - 2) * offsetY;
					last_scroll = Timer::getTime() + 1000; // start scrolling after 1 second
				}

				/*// didn't make it to top 3, add separator
				if(i == 3)
				{
					ss << getLocaleGuiString( "gui_scorewindow_highscores_separator" );
				}*/
				// add as highlighted
				ss << "<h1>" << scores[i].name << "</h1><br><i>";
				ss << scores[i].time << " (" << scores[i].score << "p)</i><br><br><br>";
				ss2 << "   <h1>" << i+1 << ".</h1><br><br><br><br>";
			}

			// don't list the 4th score since it's not my score
			else //if(i < 3 || !showPlayerScore)
			{
				ss << "<b>" << scores[i].name << "</b><br>";
				ss << scores[i].time << " (" << scores[i].score << "p)<br><br><br>";
				ss2 << "   <b>" << i+1 << ".</b><br><br><br><br>";
			}
		}

		highScores->setText( ss.str() );
		highScoreNumbers->setText( ss2.str() );
		
		// give some room for fader
		highScores->moveBy(0, 25);
		highScoreNumbers->moveBy(0, 25);

		// score limit stars
		//
		int developer, ultimate;
		std::string mission = std::string("survival\\") + game->getMissionId();
		if(GameStats::getScoreLimits(mission.c_str(), developer, ultimate))
		{
			for(unsigned int i = 0; i < scores.size(); i++)
			{
				int score = atoi(scores[i].score.c_str());
				if(score > ultimate)
				{
					int x = getLocaleGuiInt("gui_scorewindow_ultimatestar_x", 0) + highScoreNumbers->getX();
					int y = getLocaleGuiInt("gui_scorewindow_ultimatestar_y", 0) + highScoreNumbers->getLinePositionY(i * 4) + 25;
					int w = getLocaleGuiInt("gui_scorewindow_ultimatestar_w", 0);
					int h = getLocaleGuiInt("gui_scorewindow_ultimatestar_h", 0);
					const char *img = getLocaleGuiString("gui_scorewindow_ultimatestar_img");
					OguiButton *but = ogui->CreateSimpleImageButton(win, x, y, w, h, NULL, NULL, NULL, img, 0, 0, false);
					but->SetDisabled(true);
					starButtons.push_back(but);
				}
				else if(score > developer)
				{
					int x = getLocaleGuiInt("gui_scorewindow_developerstar_x", 0) + highScoreNumbers->getX();
					int y = getLocaleGuiInt("gui_scorewindow_developerstar_y", 0) + highScoreNumbers->getLinePositionY(i * 4) + 25;
					int w = getLocaleGuiInt("gui_scorewindow_developerstar_w", 0);
					int h = getLocaleGuiInt("gui_scorewindow_developerstar_h", 0);
					const char *img = getLocaleGuiString("gui_scorewindow_developerstar_img");
					OguiButton *but = ogui->CreateSimpleImageButton(win, x, y, w, h, NULL, NULL, NULL, img, 0, 0, false);
					but->SetDisabled(true);
					starButtons.push_back(but);
				}
			}
		}

		if(scorefader != NULL)
			delete scorefader;

		scorefader = oguiLoader.LoadButton( "scorefader", win, 0 );
		scorefader->SetDisabled(true);

		// re-order buttons a bit..
		//

		if(game->isCooperative() && game::SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + player + 1 ))
		{
			if(closebut != NULL)
				delete closebut;

			closebut = oguiLoader.LoadButton( "nextplayerbutton", win, SCOREWINDOW_CLOSEME );
			closebut->SetText( getLocaleGuiString("gui_scorewindow_closebutton_text") );
			closebut->SetListener(this);
			closebut->SetDisabled(false);
		}
		else
		{
			if(closebut != NULL)
				delete closebut;

			closebut = oguiLoader.LoadButton( "closebutton", win, SCOREWINDOW_CLOSEME );
			closebut->SetText( getLocaleGuiString("gui_close") );
			closebut->SetListener(this);
			closebut->SetDisabled(false);

			if(restartbut != NULL)
				delete restartbut;

			restartbut = oguiLoader.LoadButton( "restartbutton", win, SCOREWINDOW_RESTART );
			restartbut->SetListener(this);
			restartbut->SetDisabled(false);
		}

		// made it to top 3
		/*if(showPlayerScore && myPos < 3)
		{
			// show new score hint
			int offsetY = highScores->getFont()->getHeight() * 4;
			newHighScoreHint->setText(newHighScoreHintText);
			newHighScoreHint->moveBy(0, myPos * offsetY + highScores->getFont()->getHeight()/2);
		}*/
	}

	///////////////////////////////////////////////////////////////////////////
	
	bool AllowLoading() const
	{
		return allowLoading;
	}

	void Close()
	{
		allowLoading = true;
		if( game )
		{
			game::GameStats* stats = game::GameStats::instances[ player ];

			if(game->gameUI->getFirstPerson(player))
			{
				unsigned int myPos = 0;
				std::vector<GameStats::ScoreData> scores;

				// take name from profile
				if(!game::SimpleOptions::getBool(DH_OPT_B_SCOREWINDOW_NAME_INPUT))
					editBuffer = game->getGameProfiles()->getCurrentProfile(player);

				stats->updateScoreList(scores, myPos, editBuffer);

				// only store top 20 scores
				if(scores.size() > 20) scores.resize(20);
				GameStats::saveScores(GameStats::getCurrentScoreFile(), scores);
			}
		}
		//win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, 100);
		//win->Hide();
	}

	//=========================================================================

	void Update()
	{
		if(resetPlayerTime > 0 && game->gameTimer > resetPlayerTime)
		{
			setPlayer(resetPlayerTo, -1);
		}

		if(game->gameTimer - lastCounterUpdate > 0)
		{
			lastCounterUpdate = game->gameTimer;

			if(scoreCounters[4].current == 0)
			{

				for(unsigned int i = 0; i < scoreCounters.size(); i++)
				{
					if(scoreCounters[i].current < scoreCounters[i].target)
					{
						scoreCounters[i].current += scoreCounters[i].step;
						if(scoreCounters[i].current >= scoreCounters[i].target)
						{
							scoreCounters[i].current = scoreCounters[i].target;
							if(i != 3)
							game->gameUI->playGUISound( counterDoneSound.c_str() );
							// pause for a while
							lastCounterUpdate += scoreCounters[i].wait;
						}
						else
							game->gameUI->playGUISound( counterIncSound.c_str() );
						break;
					}
				}

				std::string text = modifiedStatsText;
				text = util::StringReplace("($scoretime)", int2str(scoreCounters[0].current), text);
				text = util::StringReplace("($scorekills)", int2str(scoreCounters[1].current), text);
				text = util::StringReplace("($totalscore)", int2str(scoreCounters[2].current), text);
				textArea->setText( text );

				if(scoreCounters[4].current == 1)
				{
					if(!game::SimpleOptions::getBool(DH_OPT_B_SCOREWINDOW_NAME_INPUT))
					{
						// show players score
						generateScoreList(true);
					}
					else
					{
						nametext->SetText(originalNameText.c_str());
						name->SetText(">_");
						editcursor_show = true;
						editcursor_last_change = game->gameTimer;
						editHandle = game->gameUI->getController(0)->addKeyreader( this );
					}
				}
				else if(scoreCounters[3].current == 1)
				{
					// test score limits
					int developer, ultimate;
					std::string mission = std::string("survival\\") + game->getMissionId();
					if(GameStats::getScoreLimits(mission.c_str(), developer, ultimate))
					{
						if(scoreCounters[2].target > ultimate)
						{
							scoreLimitHint = oguiLoader.LoadButton("ultimate_hint", win, 0);
							scoreLimitHint->SetDisabled(true);
							game->gameUI->playStreamedSound( getLocaleGuiString("gui_scorewindow_ultimate_hint_sound") );
						}
						else if(scoreCounters[2].target > developer)
						{
							scoreLimitHint = oguiLoader.LoadButton("developer_hint", win, 0);
							scoreLimitHint->SetDisabled(true);
							game->gameUI->playStreamedSound( getLocaleGuiString("gui_scorewindow_developer_hint_sound") );
						}
						else
						{
							// no limit reached, don't wait
							lastCounterUpdate = game->gameTimer;
						}
					}
				}

			}
		}

		if(game->gameTimer - editcursor_last_change > GAME_TICKS_PER_SECOND/2)
		{
			editcursor_last_change = game->gameTimer;

			if(editHandle != -1)
			{
				editcursor_show = !editcursor_show;

				if(editcursor_show)
					name->SetText((">" + editBuffer + "_").c_str());
				else
					name->SetText((">" + editBuffer + " ").c_str());
			}
		}
		

		if(scrollScores > 0)
		{
			int delta_time = Timer::getTime() - last_scroll;
			int scroll_delay = 500 / (scrollScores+1);
			if(scroll_delay > 8)
				scroll_delay = 8;
			else if(scroll_delay < 4)
				scroll_delay = 4;

			if(delta_time >= scroll_delay)
			{
				last_scroll = Timer::getTime();
				int amount = -delta_time / scroll_delay;

				if(scrollScores + amount < 0)
					amount = -scrollScores;

				highScoreNumbers->moveBy(0, amount, true, false);
				highScores->moveBy(0, amount, true, false);
				for(unsigned int i = 0; i < starButtons.size(); i++)
				{
					if(starButtons[i])
					{
						starButtons[i]->MoveBy(0, amount);
						if(starButtons[i]->GetY() < highScores->getY())
						{
							delete starButtons[i];
							starButtons[i] = NULL;
						}
					}
				}
				scrollScores += amount;
			}
		}
	}

	int getPlayer()
	{
		return player;
	}

	void setPlayer(int p, int time)
	{
		if(time < 0)
		{
			if(resetPlayerTo != -1)
				game->gameUI->getEffects()->startFadeIn(200);

			resetPlayerTime = -1;
			resetPlayerTo = -1;

			player = p;
			allowLoading = false;
			highScores->setText("");
			highScoreNumbers->setText("");
			for(unsigned int i = 0; i < starButtons.size(); i++)
			{
				delete starButtons[i];
			}
			starButtons.clear();
			// wait before starting counting
			creationTime = game->gameTimer;
			lastCounterUpdate = game->gameTimer + 3 * GAME_TICKS_PER_SECOND / 2;
			editBuffer.clear();
			GenerateStats();
		}
		else
		{
			delete closebut;
			closebut = NULL;

			allowLoading = false;
			resetPlayerTo = p;
			resetPlayerTime = game->gameTimer + time;
		}
	}

	bool shouldRestart()
	{
		return pressed_restart;
	}

	void pleaseClose()
	{
		// finish counters
		if(scoreCounters[4].current == 0)
		{
			lastCounterUpdate = 0;
			for(unsigned int i = 0; i < scoreCounters.size(); i++)
			{
				scoreCounters[i].current = scoreCounters[i].target - 1;
				scoreCounters[i].wait = 0;
			}
		}
		else if(editHandle != -1)
		{
			// stop writing name
			if(editBuffer.empty())
			{
				editBuffer = game->getGameProfiles()->getCurrentProfile(player);
			}
			editButtonEnter();
		}
		else if(scrollScores != 0 && highScores != NULL && highScoreNumbers != NULL)
		{
			highScoreNumbers->moveBy(0, -scrollScores, true, false);
			highScores->moveBy(0, -scrollScores, true, false);
			scrollScores = 0;
		}
		else
		{
			// close
			Close();
		}
	}

	//=========================================================================

private:
	Ogui*		ogui;
	OguiWindow*	win;
	game::Game*	game;
	bool		allowLoading;
	int player;
	
	struct ScoreCounter
	{
		int current;
		int target;
		int step;
		int wait;
	};
	std::vector<ScoreCounter> scoreCounters;
	std::string counterIncSound;
	std::string counterDoneSound;
	int lastCounterUpdate;
	int creationTime;
	
	OguiButton*	closebut;
	OguiButton* restartbut;
	OguiFormattedText* textArea;
	std::string originalStatsText;
	std::string modifiedStatsText;

	OguiFormattedText* newHighScoreHint;
	std::string newHighScoreHintText;

	OguiFormattedText* highScores;
	OguiFormattedText* highScoreNumbers;
	OguiButton* scorefader;
	std::vector<OguiButton *> starButtons;
	OguiButton* scoreLimitHint;
	int scrollScores;
	int last_scroll;

	OguiButton* title;
	OguiButton* clickCatcher;

	OguiLocaleWrapper	oguiLoader;

	OguiButton* name;
	OguiButton* nametext;
	std::string originalNameText;
	std::string editBuffer;
	int editHandle;
	bool editcursor_show;
	int editcursor_last_change;

	int resetPlayerTo;
	int resetPlayerTime;

	bool pressed_restart;
};

///////////////////////////////////////////////////////////////////////////////

ScoreWindow::ScoreWindow( Ogui* ogui, game::Game* game, int player ) :
	impl( NULL )
{
	// Logger::getInstance()->debug("ScoreWindow opened");
	impl = new ScoreWindowImpl( ogui, game, player );
}

//=============================================================================

ScoreWindow::~ScoreWindow()
{
	// Logger::getInstance()->debug("ScoreWindow killed");
	delete impl;
	impl = NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool ScoreWindow::AllowLoading() const 
{
	fb_assert( impl );
	return impl->AllowLoading();
}

//=============================================================================

bool ScoreWindow::CloseMePlease() const
{
	return AllowLoading();
}

void ScoreWindow::PleaseClose()
{
	fb_assert( impl );
	impl->pleaseClose();
}
///////////////////////////////////////////////////////////////////////////////

void ScoreWindow::Update( int )
{
	fb_assert( impl );
	impl->Update();
}

int ScoreWindow::getPlayer()
{
	return impl->getPlayer();
}

void ScoreWindow::setPlayer(int player)
{
	impl->setPlayer(player, GAME_TICKS_PER_SECOND/2);
}

bool ScoreWindow::shouldRestart() const
{
	return impl->shouldRestart();
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
