
#include "precompiled.h"

#include <assert.h>
#include <limits.h>
#include <boost/lexical_cast.hpp>

#include "WeaponWindow.h"

#include "Visual2D.h"
#include "uidefaults.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../convert/str2int.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/GameProfiles.h"
#include "../game/PlayerWeaponry.h"
#include "../game/DHLocaleManager.h"
#include "../game/Weapon.h"

#include "../ogui/OguiAligner.h"

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"
#include "../system/Timer.h"
#include "CombatSubWindowFactory.h"

using namespace game;

namespace ui
{
///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( WeaponWindow );
///////////////////////////////////////////////////////////////////////////////

	WeaponWindow::WeaponWindow() :
		ogui( NULL ),
		game( NULL ),
		clientNum( 0 ),
		win( NULL ),

		weaponIconButton( NULL ),

		listWin( NULL ),

		weaponListImage( NULL ),
		weaponListSelectedImage( NULL ),
		weaponListDisabledImage( NULL ),

		weaponNames( WEAPONWINDOW_MAX_WEAPONS ),
		weaponSelectionFont( NULL ),
		playerNameFont( NULL ),
		weaponSelectionText( NULL ),
		labelButton( NULL ),
		hideWeaponSelection( 0 ),

		profile(),
		coop(false),

		lastUpdateValue( 0 ),
		lastUpdateWeaponAmount( 0 )
	{
		int i = 0;
		for ( i = 0; i < WEAPONWINDOW_MAX_WEAPONS; i++ )
		{
			weaponImages[i] = NULL;
			weaponListButtons[i] = NULL;
		}
	}

	WeaponWindow::WeaponWindow(Ogui *ogui, game::Game *game, int clientNum, bool coop, const std::string& profile ) :
		ogui( NULL ),
		game( NULL ),
		clientNum( 0 ),
		win( NULL ),

		weaponIconButton( NULL ),

		listWin( NULL ),

		weaponListImage( NULL ),
		weaponListSelectedImage( NULL ),
		weaponListDisabledImage( NULL ),

		weaponNames( WEAPONWINDOW_MAX_WEAPONS ),
		weaponSelectionFont( NULL ),
		weaponSelectionText( NULL ),
		labelButton( NULL ),
		hideWeaponSelection( 0 ),

		profile(profile),
		coop(coop),

		lastUpdateValue( 0 ),
		lastUpdateWeaponAmount( 0 )
	{
		this->ogui = ogui;
		this->game = game;
		this->clientNum = clientNum;

		std::string prefix = "gui_weapon_" + profile;

		if( coop )
		{
			prefix = "gui_weapon_coop" + boost::lexical_cast< std::string >( clientNum ) + "_";
		}

		int xPosition = getLocaleGuiInt( (prefix + "icon_position_x" ).c_str(), 0);
		int yPosition = getLocaleGuiInt( (prefix + "icon_position_y" ).c_str(), 0);

		this->win = ogui->CreateSimpleWindow(xPosition, yPosition, getLocaleGuiInt( (prefix + "icon_window_size_x" ).c_str(), 0), getLocaleGuiInt( (prefix + "icon_window_size_y" ).c_str(), 0), NULL);
		this->win->SetUnmovable();

		this->listWin = ogui->CreateSimpleWindow(getLocaleGuiInt( (prefix + "list_position_x" ).c_str(), 0), getLocaleGuiInt( (prefix + "list_position_y" ).c_str(), 0), getLocaleGuiInt( (prefix + "list_size_x" ).c_str(), 0), getLocaleGuiInt( (prefix + "list_size_y" ).c_str(), 0), NULL);
		this->listWin->SetUnmovable();

		{
			weaponNames.resize( WEAPONWINDOW_MAX_WEAPONS );
			for (int i = 0; i < WEAPONWINDOW_MAX_WEAPONS; i++)
			{
#ifdef PROJECT_SHADOWGROUNDS
				char buf[256];
				strcpy(buf, "gui_weapon_image_");
				strcat(buf, int2str(i));
				const char *tmp = getLocaleGuiString(buf);
				this->weaponImages[i] = ogui->LoadOguiImage(tmp);
				
				weaponNames[ i ] = getLocaleGuiString( ( std::string( "gui_weapon_select_text_" ) + boost::lexical_cast< std::string >( i ) ).c_str() );
#else
				std::string buf = prefix + "image_" + boost::lexical_cast< std::string >( i ); 
				std::string tmp = getLocaleGuiString(buf.c_str());
				this->weaponImages[i] = ogui->LoadOguiImage(tmp.c_str());
				
				std::string prefix2 = prefix;

#ifdef PROJECT_SURVIVOR
				if(coop)
				{
					// fix weapon names for coop
					prefix2 = "gui_weapon_marine_";
					Unit *unit = game->gameUI->getFirstPerson(clientNum);
					if(unit)
					{
						if(strcmp(unit->getUnitType()->getName(), "Surv_Napalm") == 0)
							prefix2 = "gui_weapon_napalm_";
						else if(strcmp(unit->getUnitType()->getName(), "Surv_Sniper") == 0)
							prefix2 = "gui_weapon_sniper_";
					}
				}
#endif
				weaponNames[ i ] = getLocaleGuiString( ( std::string( prefix2 + "select_text_" ) + boost::lexical_cast< std::string >( i ) ).c_str() );

#endif
			}
		}

		weaponIconButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt( (prefix + "icon_pos_x" ).c_str(), 0), getLocaleGuiInt( (prefix + "icon_pos_y" ).c_str(), 0), getLocaleGuiInt( (prefix + "icon_size_x" ).c_str(), 0), getLocaleGuiInt( (prefix + "icon_size_y" ).c_str(), 0), NULL, NULL, NULL);
		weaponIconButton->SetDisabled(true);
		//weaponIconButton->SetDisabledImage(weaponImages[0]);

		weaponListImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix + "list_image").c_str()));
		weaponListSelectedImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix + "list_selected_image").c_str()));
		weaponListDisabledImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix + "list_disabled_image").c_str()));

		{
			int offsetX = getLocaleGuiInt( (prefix + "list_slot_offset_x" ).c_str(), 0);
			int offsetY = getLocaleGuiInt( (prefix + "list_slot_offset_y" ).c_str(), 0);
			int sizeX = getLocaleGuiInt( (prefix + "list_slot_size_x" ).c_str(), 0);
			int sizeY = getLocaleGuiInt( (prefix + "list_slot_size_y" ).c_str(), 0);
			for (int i = 0; i < WEAPONWINDOW_MAX_WEAPONS; i++)
			{
#ifdef PROJECT_SURVIVOR
				// only show 5th slot if carrying electric gun
				if(i == 4)
				{
					if(game->gameUI->getFirstPerson(clientNum) && game->gameUI->getFirstPerson(clientNum)->getWeaponByWeaponType(PARTTYPE_ID_STRING_TO_INT("W_Elect3")) != -1)
					{

					}
					else
					{
						weaponListButtons[i] = NULL;
						continue;
					}
				}
#endif
				weaponListButtons[i] = ogui->CreateSimpleImageButton(listWin, 0 + i * offsetX, 0 + i * offsetY, sizeX, sizeY, NULL, NULL, NULL, 0);
				weaponListButtons[i]->SetDisabledImage(weaponListDisabledImage);
				weaponListButtons[i]->SetDisabled(true);
			}
		}
		
		weaponSelectionFont = ogui->LoadFont( getLocaleGuiString( ( prefix + "text_font" ).c_str() ) );
		playerNameFont = ogui->LoadFont( getLocaleGuiString( ( prefix + "player_name_font" ).c_str() ) );


		if( getLocaleGuiInt( ( prefix + "use_text" ).c_str(), 0 ) )
		{
			weaponSelectionText = ogui->CreateTextLabel(win, getLocaleGuiInt( (prefix + "text_offset_x" ).c_str(), 0 ), getLocaleGuiInt( (prefix + "text_offset_y" ).c_str(), 0 ), getLocaleGuiInt( (prefix + "text_size_x").c_str(), 0), getLocaleGuiInt( (prefix + "text_size_y").c_str(), 0), "");
			weaponSelectionText->SetFont( weaponSelectionFont? weaponSelectionFont : playerNameFont );
			if( coop )
				weaponSelectionText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
			else 
				weaponSelectionText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_RIGHT );
		}

		if( getLocaleGuiInt( ( prefix + "use_label" ).c_str(), 0 ) )
		{
			labelButton = ogui->CreateSimpleTextButton( win, getLocaleGuiInt( ( prefix + "label_offset_x" ).c_str(), 0 ),  getLocaleGuiInt( ( prefix + "label_offset_y" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "label_size_x" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "label_size_y" ).c_str(), 0 ), NULL, NULL, NULL, NULL, 0 );
			labelButton->SetFont( playerNameFont? playerNameFont : ui::defaultSmallIngameFont );
			labelButton->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
#ifdef PROJECT_SURVIVOR
			labelButton->SetText( game->getGameProfiles()->getCurrentProfile(clientNum) );
#else
			labelButton->SetText( getLocaleGuiString( ( prefix + "label_text" ).c_str() ) );
#endif
		}

		lastUpdateValue = -1;
		//selectionTextTimeLeft = 0;
		lastUpdateWeaponAmount = -1;

		hideWeaponSelection =			getLocaleGuiInt( ( prefix + "time_to_start_fading_weapon_text" ).c_str(), 0 );
		hideWeaponSelectionFadeLength = getLocaleGuiInt( ( prefix + "fade_length" ).c_str(), 0 );
		lastUpdateWeaponText = 0;

		win->SetEffectListener(this);

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(win, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		OguiAligner::align(listWin, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		OguiAligner::align(weaponIconButton, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		for (int i = 0; i < WEAPONWINDOW_MAX_WEAPONS; i++)
		{
			if(weaponListButtons[i])
				OguiAligner::align(weaponListButtons[i], OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		}
		OguiAligner::align(weaponSelectionText, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		OguiAligner::align(labelButton, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
	}


	WeaponWindow::~WeaponWindow()
	{
		// FIXME: this may leak (not delete) some created buttons, images, etc.?

		//if (weaponSelectionText != NULL)
		//{
		//	delete weaponSelectionText;
		//	weaponSelectionText = NULL;
		//}

		delete weaponSelectionText;
		delete weaponSelectionFont;

		delete labelButton;
		delete playerNameFont;

		if (weaponIconButton != NULL)
		{
			delete weaponIconButton;
			weaponIconButton = NULL;
		}
		if (win != NULL)
		{
			delete win;
			win = NULL;
		}
		if (listWin != NULL)
		{
			delete listWin;
			listWin = NULL;
		}

		for (int i = 0; i < WEAPONWINDOW_MAX_WEAPONS; i++)
		{
			delete weaponImages[i];
		}
	}


	void WeaponWindow::hide(int fadeTime)
	{
		if(fadeTime)
		{
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
			listWin->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		}
		else
		{
			win->Hide();
			listWin->Hide();
		}
	}


	void WeaponWindow::show(int fadeTime)
	{
		if(fadeTime)
		{
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);
			listWin->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);
		}

		win->Show();
		listWin->Show();
	}



	void WeaponWindow::forceUpdate()
	{
		lastUpdateValue = INT_MIN;
		update();
	}


	void WeaponWindow::update()
	{
		// hack: dont update invisible win
		if (!win->IsVisible())
			return;

		if (game->gameUI->getFirstPerson(clientNum) == NULL)
			return;

		int selWeap = game->gameUI->getFirstPerson(clientNum)->getSelectedWeapon();

		//if (game->gameUI->getFirstPerson(clientNum)->getWeaponType(selWeap) == NULL)
		//	return;

		int weaponAmount = 0;

		{
			for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
			{
				if(game->gameUI->getFirstPerson(clientNum)->getWeaponType(w) == NULL)
					continue;

				Weapon *weap = game->gameUI->getFirstPerson(clientNum)->getWeaponType(w);

				if ( (weap->isSelectableWithoutAmmo() || game->gameUI->getFirstPerson(clientNum)->getWeaponAmmoAmount(w) > 0)
#ifdef PROJECT_SURVIVOR
					&& true)
#else
					&& game->gameUI->getFirstPerson(clientNum)->isWeaponOperable(w))
#endif
				{
					weaponAmount++;
				}
			}
		}

		if (selWeap != lastUpdateValue || lastUpdateWeaponAmount != weaponAmount)
		{
			lastUpdateWeaponAmount = weaponAmount;

			for (int i = 0; i < WEAPONWINDOW_MAX_WEAPONS; i++)
			{
				int wtypeId = game::PlayerWeaponry::getWeaponIdByUINumber(game->gameUI->getFirstPerson(clientNum), i);
				int weapNum = game->gameUI->getFirstPerson(clientNum)->getWeaponByWeaponType(wtypeId);
#ifndef PROJECT_SURVIVOR
				if (!game->gameUI->getFirstPerson(clientNum)->isWeaponOperable(weapNum))
					weapNum = -1;
#endif

				if (weapNum != -1) {
				Weapon *weap = game->gameUI->getFirstPerson(clientNum)->getWeaponType(weapNum);
				if(weap && !weap->isSelectableWithoutAmmo() && game->gameUI->getFirstPerson(clientNum)->getWeaponAmmoAmount(weapNum) == 0)
					weapNum = -1;
				}

				if (weapNum != -1)
				{
					if(weaponListButtons[i])
						weaponListButtons[i]->SetDisabledImage(weaponListImage);
				} else {
					if(weaponListButtons[i])
						weaponListButtons[i]->SetDisabledImage(weaponListDisabledImage);
				}
			}

			lastUpdateValue = selWeap;

			if (weaponAmount == 0 || selWeap == -1)
			{
				weaponIconButton->SetDisabledImage(NULL);
			} else {
				int uinum = game::PlayerWeaponry::getUINumberByWeaponId(game->gameUI->getFirstPerson(clientNum), game->gameUI->getFirstPerson(clientNum)->getWeaponType(selWeap)->getPartTypeId());
				if(uinum >= 0 && uinum < WEAPONWINDOW_MAX_WEAPONS)
				{
					if(!game->gameUI->getFirstPerson(clientNum)->isWeaponOperable(selWeap))
					{
						std::string prefix = "gui_weapon_" + profile;
						if( coop )
						{
							prefix = "gui_weapon_coop" + boost::lexical_cast< std::string >( clientNum ) + "_";
						}
						std::string buf = prefix + "image_" + boost::lexical_cast< std::string >( uinum ) + "_disabled";

						if(::game::DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, buf.c_str() ))
						{
							IOguiImage *img = ogui->LoadOguiImage(getLocaleGuiString(buf.c_str()));
							weaponIconButton->SetDisabledImage(img);
							if(img != NULL)
							{
								bool autodel[4];
								weaponIconButton->GetImageAutoDelete(&autodel[0],&autodel[1],&autodel[2],&autodel[3]);
								weaponIconButton->SetImageAutoDelete( autodel[0], autodel[1],       true, autodel[3]);
							}
						}
						else
						{
							weaponIconButton->SetDisabledImage(weaponImages[uinum]);
						}
					}
					else
					{
						weaponIconButton->SetDisabledImage(weaponImages[uinum]);
					}
					if(weaponListButtons[uinum])
						weaponListButtons[uinum]->SetDisabledImage(weaponListSelectedImage);
				}

				if ( weaponSelectionText && uinum >= 0 && uinum < (int)weaponNames.size() )
				{
					lastUpdateWeaponText = Timer::getTime();
					weaponSelectionText->SetTransparency( 0 );
					weaponSelectionText->SetText( weaponNames[ uinum ].c_str() );
				}
			}

			/*{
				int uinum = game::PlayerWeaponry::getUINumberByWeaponId(game->gameUI->getFirstPerson(clientNum), game->gameUI->getFirstPerson(clientNum)->getWeaponType(selWeap)->getPartTypeId());
				if ( weaponSelectionText && uinum >= 0 && uinum < (int)weaponNames.size() )
				{
					weaponSelectionText->SetText( weaponNames[ uinum ].c_str() );
				}
			}*/
			//weaponSelectionText->SetText(...);
			//selectionTextTimeLeft = 2000;
		}


		if( weaponSelectionText && hideWeaponSelection && 
			( ( Timer::getTime() - lastUpdateWeaponText ) > hideWeaponSelection ) )
		{
			int time = ( Timer::getTime() - ( lastUpdateWeaponText + hideWeaponSelection ) );
			if( time > 0 && time < hideWeaponSelectionFadeLength )
			{
				float trans = (float)( time / (float)( hideWeaponSelectionFadeLength ) );

				if( trans <= 0.0f ) trans = 0.0f;
				if( trans >= 1.0f ) trans = 1.0f;

				weaponSelectionText->SetTransparency( (int)( trans * 100.0f ) );
			} else {
				weaponSelectionText->SetTransparency( 100 );
				weaponSelectionText->SetText( "" );
			}

		}
	}

	void WeaponWindow::EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
			win->Hide();
	}

}
