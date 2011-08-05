
#include "precompiled.h"

#include "UpgradeWindow.h"

#include <string.h>
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UpgradeManager.h"
#include "../game/UpgradeType.h"
#include "../game/Unit.h"
#include "../game/Weapon.h"
#include "../game/PlayerWeaponry.h"
#include "../ui/uidefaults.h"
#include "../ui/GUIEffectWindow.h"
#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../util/fb_assert.h"
#include "../game/DHLocaleManager.h"

#define UPGRADE_WINDOW_DEFAULT_SIZE_X 1024
#define UPGRADE_WINDOW_DEFAULT_SIZE_Y 768

//#define UPGRADE_WEAPON_SELECTION_SIZE_X 128
//#define UPGRADE_WEAPON_SELECTION_SIZE_Y 64

//#define UPGRADE_SLOT_SIZE_X 32
//#define UPGRADE_SLOT_SIZE_Y 32
//#define UPGRADE_SLOT_PADDING 8

// Button ids
#define UPGRADEW_CLOSEBUT 1
#define UPGRADEW_UNDOBUT 2
#define UPGRADEW_WEAPONBUT_FIRST 100
#define UPGRADEW_WEAPONBUT_LAST (UPGRADEW_WEAPONBUT_FIRST+UPGRADEWINDOW_MAX_WEAPONS-1)
#define UPGRADEW_UPGRADEBUT_FIRST (UPGRADEW_WEAPONBUT_LAST+1)
#define UPGRADEW_UPGRADEBUT_LAST (UPGRADEW_UPGRADEBUT_FIRST+(UPGRADEWINDOW_MAX_WEAPONS*UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON)-1)

static const int FADE_IN_TIME = 500;
static const int FADE_OUT_TIME = 500;

using namespace game;

namespace ui
{
	UpgradeWindow::UpgradeWindow(Ogui *ogui, game::Game *game, game::Unit *unit)
	{
		//this->selectionNumber = -1;
		this->game = game;
		this->ogui = ogui;
		this->unit = unit;

		this->visible = 1;

		highlightedWeaponSlot = -1;
		highlightedUpgradeSlot = -1;
		highlightedUpgradeId = -1;
		highlightOn = false;

		this->upgradesPending = new LinkedList();
		this->upgradesPendingCost = 0;

		assert(unit != NULL);

		const char *l1Image = getLocaleGuiString("gui_upgrades_window_effect_layer1_image");
		const char *l2Image = getLocaleGuiString("gui_upgrades_window_effect_layer2_image");
		const char *l3Image = getLocaleGuiString("gui_upgrades_window_effect_layer3_image");
		this->effectWindow = new GUIEffectWindow(ogui, l1Image, l2Image, l3Image);

		// create window
		{
			int x = getLocaleGuiInt("gui_upgrades_window_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_window_position_y", 0);
			int sizex = getLocaleGuiInt("gui_upgrades_window_size_x", UPGRADE_WINDOW_DEFAULT_SIZE_X);
			int sizey = getLocaleGuiInt("gui_upgrades_window_size_y", UPGRADE_WINDOW_DEFAULT_SIZE_Y);
			//const char *bgimage = getLocaleGuiString("gui_upgrades_window_background_image");

			win = ogui->CreateSimpleWindow(x, y, sizex, sizey, NULL);
		}

		win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, FADE_IN_TIME);
		win->SetEffectListener(this);
		effectWindow->fadeIn(FADE_IN_TIME);

		// title
		{
			int x = getLocaleGuiInt("gui_upgrades_title_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_title_position_y", 0);
			int sizex = getLocaleGuiInt("gui_upgrades_title_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_title_size_y", 0);
			const char *label = getLocaleGuiString("gui_upgrades_title");
			const char *image = getLocaleGuiString("gui_upgrades_title_image");

			titleButton = ogui->CreateSimpleTextButton(win, x, y, sizex, sizey, image, NULL, NULL, label, 0);
			titleButton->SetReactMask(0);
			titleButton->SetFont(ui::defaultIngameFont);
		}

		// parts available icon
		{
			int x = getLocaleGuiInt("gui_upgrades_parts_icon_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_parts_icon_position_y", 0);
			int sizex = getLocaleGuiInt("gui_upgrades_parts_icon_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_parts_icon_size_y", 0);
			const char *image = getLocaleGuiString("gui_upgrades_parts_icon_image");

			upgradePartsIcon = ogui->CreateSimpleImageButton(win, x, y, sizex, sizey, NULL, NULL, NULL, image, 0);
			upgradePartsIcon->SetDisabled(true);
		}

		// parts available text
		{
			int x = getLocaleGuiInt("gui_upgrades_parts_text_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_parts_text_position_y", 0);
			int sizex = getLocaleGuiInt("gui_upgrades_parts_text_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_parts_text_size_y", 0);

			upgradePartsText = ogui->CreateTextLabel(win, x, y, sizex, sizey, "");
			upgradePartsText->SetFont(ui::defaultIngameFont);
		}

		// some misc images
		{
			const char *tmp;

			tmp = getLocaleGuiString("gui_upgrades_slot_background_image");
			upgradeBGImage = ogui->LoadOguiImage(tmp);

			tmp = getLocaleGuiString("gui_upgrades_slot_background_disabled_image");
			upgradeBGDisabledImage = ogui->LoadOguiImage(tmp);

			tmp = getLocaleGuiString("gui_upgrades_weapon_highlight_image");
			weaponHighlightImage = ogui->LoadOguiImage(tmp);

			tmp = getLocaleGuiString("gui_upgrades_upgrade_highlight_image");
			upgradeHighlightImage = ogui->LoadOguiImage(tmp);

			tmp = getLocaleGuiString("gui_upgrades_upgrade_selected_image");
			upgradeSelectedImage = ogui->LoadOguiImage(tmp);
		}


		//selectionImage = ogui->LoadOguiImage("Data/GUI/Buttons/Upgrade/selectedweapon.tga");
		//selection = NULL;

		// first init weapon and upgrade buttons to null...
		{
			for (int i = 0; i < UPGRADEWINDOW_MAX_WEAPONS; i++)
			{
				weaponImages[i] = NULL;
				weaponInfoImages[i] = NULL;
				weaponButtons[i] = NULL;
				slotBGs[i] = NULL;
				for (int j = 0; j < UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON; j++)
				{
					upgradeImages[i][j] = NULL;
					//upgradeDisabledImages[i][j] = NULL;
					upgradeButtons[i][j] = NULL;
					upgradeSelections[i][j] = NULL;
				}
			}
		}

		// then create weapon and upgrade buttons...
		{
			for (int i = 0; i < UPGRADEWINDOW_MAX_WEAPONS; i++)
			{
				// NOTE: this is actually not an ok part type id!
				// (thus, should not use this value like this)
				int partTypeId = -1;

				partTypeId = PlayerWeaponry::getWeaponIdByUINumber(unit, i);

				int wnum = -1;
				if (partTypeId != -1)
				{
					wnum = unit->getWeaponByWeaponType(partTypeId);
				}

				// weapon image/button
				int wx = 0;
				int wy = 0;
				getWeaponPosition(i, &wx, &wy);

				{
					int sizex = getLocaleGuiInt("gui_upgrades_slot_size_x", 0);
					int sizey = getLocaleGuiInt("gui_upgrades_slot_size_y", 0);

					slotBGs[i] = ogui->CreateSimpleImageButton(win, wx, wy, sizex, sizey, NULL, NULL, NULL, 0);
					slotBGs[i]->SetReactMask(0);
					slotBGs[i]->SetDisabled(true);
					if (wnum != -1)
					{
						slotBGs[i]->SetDisabledImage(upgradeBGImage);
					} else {
						slotBGs[i]->SetDisabledImage(upgradeBGDisabledImage);
					}
					//slotBGs[i]->SetListener(this);
				}

				if (wnum != -1)
				{
					{
						char buf[256];
						strcpy(buf, "Data/GUI/Buttons/Upgrade/weapon_");
						buf[strlen(buf) + 1] = '\0';
						buf[strlen(buf)] = '0' + i;
						strcat(buf, ".tga");
						weaponImages[i] = ogui->LoadOguiImage(buf);

						strcpy(buf, "Data/GUI/Buttons/Upgrade/weapon_");
						buf[strlen(buf) + 1] = '\0';
						buf[strlen(buf)] = '0' + i;
						strcat(buf, "_info.tga");
						weaponInfoImages[i] = ogui->LoadOguiImage(buf);

						{
							int sizex = getLocaleGuiInt("gui_upgrades_slot_weapon_size_x", 0);
							int sizey = getLocaleGuiInt("gui_upgrades_slot_weapon_size_y", 0);
							int woffx = getLocaleGuiInt("gui_upgrades_slot_weapon_offset_x", 0);

							int id = UPGRADEW_WEAPONBUT_FIRST + i;
							weaponButtons[i] = ogui->CreateSimpleImageButton(win, wx + woffx, wy, sizex, sizey, NULL, NULL, NULL, NULL, id, (void *)partTypeId);
							weaponButtons[i]->SetReactMask(0);
							weaponButtons[i]->SetDisabled(false);
							weaponButtons[i]->SetImage(weaponImages[i]);
							weaponButtons[i]->SetListener(this);
							weaponButtons[i]->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
						}
					}

					// upgrades for that weapon
					{
						const char *weapName = NULL;

						Weapon *w = unit->getWeaponType(wnum);
						if (w != NULL)
						{
							weapName = w->getPartTypeIdString();
						}
						std::vector<int> upgIds;
						game->upgradeManager->getUpgradesForPart(weapName, upgIds);

						if (upgIds.size() > UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON)
						{
							Logger::getInstance()->error("UpgradeWindow - Weapon has too many upgrades.");
							Logger::getInstance()->debug(weapName);
						}

						for (int j = 0; j < (int)upgIds.size(); j++)
						{
							UpgradeType *upgType = game->upgradeManager->getUpgradeTypeById(upgIds[j]);
							const char *upgScript = upgType->getScript();

							char buf[256];
							strcpy(buf, "Data/GUI/Buttons/Upgrade/");
							if (strlen(upgScript) < 64)
								strcat(buf, upgScript);
							strcat(buf, ".tga");
							upgradeImages[i][j] = ogui->LoadOguiImage(buf);

							//strcpy(buf, "Data/GUI/Buttons/Upgrade/");
							//if (strlen(upgScript) < 64)
							//	strcat(buf, upgScript);
							//strcat(buf, "_disabled.tga");
							//upgradeDisabledImages[i][j] = ogui->LoadOguiImage(buf);

							int x = 0;
							int y = 0;
							getWeaponPosition(i, &x, &y);

							int woffx = getLocaleGuiInt("gui_upgrades_slot_weapon_offset_x", 0);
							x += woffx;

							//y += UPGRADE_WEAPON_SELECTION_SIZE_Y;
							//x += j * (UPGRADE_SLOT_SIZE_X+UPGRADE_SLOT_PADDING);
							int firstpadx = getLocaleGuiInt("gui_upgrades_slot_upgrade_start_offset_x", 0);
							int padx = getLocaleGuiInt("gui_upgrades_slot_upgrade_offset_x", 0);
							int sizex = getLocaleGuiInt("gui_upgrades_slot_upgrade_size_x", 0);
							int sizey = getLocaleGuiInt("gui_upgrades_slot_upgrade_size_y", 0);

							x += firstpadx + j * padx;

							upgradeSelections[i][j] = ogui->CreateSimpleImageButton(win, x, y, sizex, sizey, NULL, NULL, NULL, NULL, 0, NULL);
							upgradeSelections[i][j]->SetReactMask(0);
							upgradeSelections[i][j]->SetDisabled(true);

							int id = UPGRADEW_UPGRADEBUT_FIRST + i*UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON+j;
							assert(id >= UPGRADEW_UPGRADEBUT_FIRST && id <= UPGRADEW_UPGRADEBUT_LAST);
							int upgid = upgIds[j];
							upgradeButtons[i][j] = ogui->CreateSimpleImageButton(win, x, y, sizex, sizey, NULL, NULL, NULL, NULL, id, (void *)upgid);
							upgradeButtons[i][j]->SetReactMask(0);
							upgradeButtons[i][j]->SetImage(upgradeImages[i][j]);
							//upgradeButtons[i][j]->SetDisabledImage(upgradeDisabledImages[i][j]);
							upgradeButtons[i][j]->SetListener(this);
							upgradeButtons[i][j]->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

							//int tmp = this->upgradesPendingCost;
							//if (game->upgradeManager->canUpgrade(unit, upgIds[j], &tmp))
							//{
								upgradeButtons[i][j]->SetDisabled(false);
							//} else {
							//	upgradeButtons[i][j]->SetDisabled(true);
							//}
						}
					}

				} else {
					weaponImages[i] = NULL;
					weaponButtons[i] = NULL;
				}
			}
		}

		// info picture and text
		{
			// background
			int x = getLocaleGuiInt("gui_upgrades_info_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_info_position_y", 0);
			int bgsizex = getLocaleGuiInt("gui_upgrades_info_size_x", 0);
			int bgsizey = getLocaleGuiInt("gui_upgrades_info_size_y", 0);
			const char *image = getLocaleGuiString("gui_upgrades_info_background_image");

			infoBackground = ogui->CreateSimpleImageButton(win, x, y, bgsizex, bgsizey, NULL, NULL, NULL, image, 0, NULL);
			//infoBackground->SetListener(this);
			infoBackground->SetDisabled(true);

			// picture
			{
				int offx = getLocaleGuiInt("gui_upgrades_info_picture_offset_x", 0);
				int offy = getLocaleGuiInt("gui_upgrades_info_picture_offset_y", 0);
				int sizex = getLocaleGuiInt("gui_upgrades_info_picture_size_x", 0);
				int sizey = getLocaleGuiInt("gui_upgrades_info_picture_size_y", 0);

				infoIcon = ogui->CreateSimpleImageButton(win, x + offx, y + offy, sizex, sizey, NULL, NULL, NULL, NULL, 0, NULL);
				infoIcon->SetDisabled(true);
			}

			// text
			{
				int offx = getLocaleGuiInt("gui_upgrades_info_textarea_offset_x", 0);
				int offy = getLocaleGuiInt("gui_upgrades_info_textarea_offset_y", 0);
				int sizex = getLocaleGuiInt("gui_upgrades_info_textarea_size_x", 0);
				int sizey = getLocaleGuiInt("gui_upgrades_info_textarea_size_y", 0);
				
				infoText = new OguiFormattedText( win, ogui, x + offx, y + offy, sizex, sizey );
				// infoText = ogui->CreateTextArea(win, x + offx, y + offy, sizex, sizey, "");
				// infoText->SetFont(ui::defaultIngameFont);

				std::string name;
				name = "gui_upgrades_info_textarea";

				fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_default" ).c_str()		) ) );
				fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_bold" ).c_str()		) ) );
				fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_italic" ).c_str()		) ) );
				fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_underline" ).c_str()	) ) );
				fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_h1" ).c_str()			) ) );
				fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_page" ).c_str()		) ) );

				infoText->setFont( fonts[ 0 ] );
				infoText->registerFont( "b",    fonts[ 1 ] );
				infoText->registerFont( "i",    fonts[ 2 ] );
				infoText->registerFont( "u",	fonts[ 3 ] );
				infoText->registerFont( "h1",   fonts[ 4 ] );
			}

		}

		// finally undo button
		{
			int x = getLocaleGuiInt("gui_upgrades_undo_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_undo_position_y", 0);
			int sizex = getLocaleGuiInt("gui_upgrades_undo_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_undo_size_y", 0);
			const char *image = getLocaleGuiString("gui_upgrades_undo_image");
			const char *imageDown = getLocaleGuiString("gui_upgrades_undo_down_image");
			const char *imageHighlight = getLocaleGuiString("gui_upgrades_undo_highlight_image");

			undoButton = ogui->CreateSimpleTextButton(win, x, y, sizex, sizey, image, imageDown, imageHighlight, "", UPGRADEW_UNDOBUT);
			undoButton->SetListener(this);
			undoButton->SetFont(ui::defaultIngameFont);
		}

		// and close button
		{
			int x = getLocaleGuiInt("gui_upgrades_close_position_x", 0);
			int y = getLocaleGuiInt("gui_upgrades_close_position_y", 0);
			int sizex = getLocaleGuiInt("gui_upgrades_close_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_close_size_y", 0);
			const char *label = getLocaleGuiString("gui_upgrades_close");
			const char *image = getLocaleGuiString("gui_upgrades_close_image");
			const char *imageDown = getLocaleGuiString("gui_upgrades_close_down_image");
			const char *imageHighlight = getLocaleGuiString("gui_upgrades_close_highlight_image");

			closeButton = ogui->CreateSimpleTextButton(win, x, y, sizex, sizey, image, imageDown, imageHighlight, label, UPGRADEW_CLOSEBUT);
			closeButton->SetListener(this);
			closeButton->SetFont(ui::defaultIngameFont);
		}

		// highlight for upgrades
		{
			int sizex = getLocaleGuiInt("gui_upgrades_slot_upgrade_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_slot_upgrade_size_y", 0);
			upgradeHighlight = ogui->CreateSimpleTextButton(win, 0, 0, sizex, sizey, NULL, NULL, NULL, NULL);
			upgradeHighlight->SetDisabled(true);
		}
		// highlight for weapons
		{
			int sizex = getLocaleGuiInt("gui_upgrades_slot_weapon_size_x", 0);
			int sizey = getLocaleGuiInt("gui_upgrades_slot_weapon_size_y", 0);
			weaponHighlight = ogui->CreateSimpleTextButton(win, 0, 0, sizex, sizey, NULL, NULL, NULL, NULL);
			weaponHighlight->SetDisabled(true);
		}

		update();

		//game->gameUI->setGUIVisibility(game->singlePlayerNumber, false);
	}


	UpgradeWindow::~UpgradeWindow()
	{
		// TODO: should restore _previous_ gui visibility...
		//game->gameUI->setGUIVisibility(game->singlePlayerNumber, true);

		//setSelectedWeapon(-1);

		while (!upgradesPending->isEmpty())
		{
			upgradesPending->popLast();
		}

		delete upgradesPending;
		upgradesPending = NULL;

		/*
		if (selection != NULL)
		{
			delete selection;
			selection = NULL;
		}	
		delete selectionImage;
		*/

		// TODO: delete a lot of buttons and stuff!!!!

		delete effectWindow;
		effectWindow = NULL;

		for (int i = 0; i < UPGRADEWINDOW_MAX_WEAPONS; i++)
		{
			if (weaponButtons[i] != NULL)
			{
				delete weaponButtons[i];
				weaponButtons[i] = NULL;
			}
			if (weaponImages[i] != NULL)
			{
				delete weaponImages[i];
				weaponImages[i] = NULL;
			}
		}

		delete win;
		win = NULL;
	}


	void UpgradeWindow::update()
	{
		/*
		int selectionNumber = -1;

		// currently selected weapon...

		// solve current weapon ui-number...
		int curWeap = -1;
		int selnum = unit->getSelectedWeapon();
		Weapon *wtype = NULL;
		if (selnum != -1) wtype = unit->getWeaponType(selnum);

		if (wtype != NULL)
		{
			curWeap = PlayerWeaponry::getUINumberByWeaponId(unit, wtype->getPartTypeId());
		}
		selectionNumber = curWeap;

		// set selection marker to that weapon...

		assert(selectionNumber >= -1 && selectionNumber < UPGRADEWINDOW_MAX_WEAPONS);

		if (selectionNumber == -1)
		{
			if (selection != NULL)
			{
				delete selection;
				selection = NULL;
			}	
		} else {
			if (weaponButtons[selectionNumber] != NULL)
			{
				if (selection == NULL)
				{
					selection = ogui->CreateSimpleImageButton(win, 0, 0, UPGRADE_WEAPON_SELECTION_SIZE_X, UPGRADE_WEAPON_SELECTION_SIZE_Y, NULL, NULL, NULL);
					selection->SetReactMask(0);
					selection->SetDisabled(true);
					selection->SetDisabledImage(selectionImage);
				}

				int x = 0;
				int y = 0;
				getWeaponPosition(selectionNumber, &x, &y);

				selection->Move(x, y);
			}
		}
		*/

		// upgrades available...

		for (int i = 0; i < UPGRADEWINDOW_MAX_WEAPONS; i++)
		{
			for (int j = 0; j < UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON; j++)
			{
				if (upgradeButtons[i][j] != NULL)
				{
					intptr_t upgid = (intptr_t)upgradeButtons[i][j]->GetArgument();
					//int tmp = this->upgradesPendingCost;

					//if (game->upgradeManager->canUpgrade(unit, upgid, &tmp))
					//{
						upgradeButtons[i][j]->SetDisabled(false);
					//} else {
					//	upgradeButtons[i][j]->SetDisabled(true);
					//}

					bool alreadyUpgraded = false;
					if (game->upgradeManager->isUpgraded(unit, upgid))
					{
						alreadyUpgraded = true;
					}

					bool pending = false;

					LinkedListIterator iter(this->upgradesPending);
					while (iter.iterateAvailable())
					{
						// WARNING: void * -> int cast
						intptr_t other = (intptr_t)iter.iterateNext();
						if (other == upgid)
						{
							pending = true;
							break;
						}
					}

					if (pending || alreadyUpgraded)
					{
						upgradeSelections[i][j]->SetDisabledImage(upgradeSelectedImage);
					} else {
						upgradeSelections[i][j]->SetDisabledImage(NULL);
					}

				}
			}
		}

		if (this->upgradesPendingCost > 0)
		{
			undoButton->SetDisabled(false);
			const char *label = getLocaleGuiString("gui_upgrades_undo");
			undoButton->SetText(label);
		} else {
			undoButton->SetDisabled(true);
			undoButton->SetText("");
		}

		if (highlightOn && highlightedWeaponSlot != -1 && highlightedUpgradeSlot == -1)
		{
			weaponHighlight->SetDisabledImage(weaponHighlightImage);
			int wx = 0;
			int wy = 0;
			getWeaponPosition(highlightedWeaponSlot, &wx, &wy);
			int woffx = getLocaleGuiInt("gui_upgrades_slot_weapon_offset_x", 0);
			wx += woffx;
			weaponHighlight->Move(wx,wy);
		} else {
			weaponHighlight->SetDisabledImage(NULL);
		}

		if (highlightedWeaponSlot != -1)
		{
			if (highlightOn && highlightedUpgradeSlot != -1)
			{
				upgradeHighlight->SetDisabledImage(upgradeHighlightImage);

				int wx = 0;
				int wy = 0;
				getWeaponPosition(highlightedWeaponSlot, &wx, &wy);
				int woffx = getLocaleGuiInt("gui_upgrades_slot_weapon_offset_x", 0);
				wx += woffx;
				int x = wx;
				int y = wy;
				int firstpadx = getLocaleGuiInt("gui_upgrades_slot_upgrade_start_offset_x", 0);
				int padx = getLocaleGuiInt("gui_upgrades_slot_upgrade_offset_x", 0);
				
				x += firstpadx + highlightedUpgradeSlot * padx;

				upgradeHighlight->Move(x,y);
			} else {
				upgradeHighlight->SetDisabledImage(NULL);
			}
		} else {
			upgradeHighlight->SetDisabledImage(NULL);
		}

		// info image & text
		if (highlightedWeaponSlot != -1)
		{
			infoIcon->SetDisabledImage(weaponInfoImages[highlightedWeaponSlot]);

			if (highlightedUpgradeSlot != -1)
			{
				fb_assert(highlightedUpgradeId != -1);

				int upgid = highlightedUpgradeId;

				UpgradeType *upgType = game->upgradeManager->getUpgradeTypeById(upgid);
				const char *upgScript = upgType->getScript();

				char buf[128];
				strcpy(buf, "upgrade_desc_");
				strcat(buf, upgScript);
				infoText->setText(getLocaleGuiString(buf));
			} else {
				char buf[128];
				strcpy(buf, "upgrade_weapon_desc_weapon_");
				strcat(buf, int2str(highlightedWeaponSlot));
				infoText->setText(getLocaleGuiString(buf));
			}
		}

		{			
			int tmp = game->upgradeManager->getUpgradePartsAmount(this->unit) - this->upgradesPendingCost;
			upgradePartsText->SetText(int2str(tmp));
		}
	}


	void UpgradeWindow::getWeaponPosition(int number, int *x, int *y)
	{
		fb_assert(number >= -1 && number < UPGRADEWINDOW_MAX_WEAPONS);

		int posx = getLocaleGuiInt("gui_upgrades_slot_position_x", 0);
		int posy = getLocaleGuiInt("gui_upgrades_slot_position_y", 0);
		int offx = getLocaleGuiInt("gui_upgrades_slot_offset_x", 0);
		int offy = getLocaleGuiInt("gui_upgrades_slot_offset_y", 0);

		*x = posx + (number / 5) * offx;
		*y = posy + (number % 5) * offy;
	}


	void UpgradeWindow::CursorEvent(OguiButtonEvent *eve)
	{
		int id = eve->triggerButton->GetId();

		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_LEAVE)
		{
			if (id >= UPGRADEW_WEAPONBUT_FIRST
				&& id <= UPGRADEW_WEAPONBUT_LAST)
			{
				highlightOn = false;
			}			
			if (id >= UPGRADEW_UPGRADEBUT_FIRST
				&& id <= UPGRADEW_UPGRADEBUT_LAST)
			{
				highlightOn = false;
			}
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
		{
			if (id >= UPGRADEW_WEAPONBUT_FIRST
				&& id <= UPGRADEW_WEAPONBUT_LAST)
			{
				highlightedWeaponSlot = eve->triggerButton->GetId() - UPGRADEW_WEAPONBUT_FIRST;
				highlightedUpgradeSlot = -1;
				highlightedUpgradeId = -1;
				highlightOn = true;
			}			
			if (id >= UPGRADEW_UPGRADEBUT_FIRST
				&& id <= UPGRADEW_UPGRADEBUT_LAST)
			{
				highlightedWeaponSlot = (eve->triggerButton->GetId() - UPGRADEW_UPGRADEBUT_FIRST) / UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON;
				highlightedUpgradeSlot = (eve->triggerButton->GetId() - UPGRADEW_UPGRADEBUT_FIRST) % UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON;
				highlightedUpgradeId = (intptr_t)eve->extraArgument;
				highlightOn = true;
			}
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			/*
			if (id >= UPGRADEW_WEAPONBUT_FIRST
				&& id <= UPGRADEW_WEAPONBUT_LAST)
			{
				int weapId = (int)eve->extraArgument;
				PlayerWeaponry::selectWeapon(game, unit, weapId);
			}
			*/
			if (id == UPGRADEW_CLOSEBUT)
			{
				game->gameUI->playGUISound(getLocaleGuiString("gui_upgrades_sound_close"));

				this->applyUpgrades();
				game->setPaused(false);

				// NOTE: must return here as this UpgradeWindow object HAS BEEN DELETED!
				//game->gameUI->closeUpgradeWindow(this->unit);
				fadeOut();
				return;
			}
			if (id == UPGRADEW_UNDOBUT)
			{
				game->gameUI->playGUISound(getLocaleGuiString("gui_upgrades_sound_undo"));
				this->undoUpgrades();
			}

			if (id >= UPGRADEW_UPGRADEBUT_FIRST
				&& id <= UPGRADEW_UPGRADEBUT_LAST)
			{
				bool playDoneSound = false;

				// upgrade button
				intptr_t upgid = (intptr_t)eve->extraArgument;

				bool alreadyPending = false;

				LinkedListIterator iter(this->upgradesPending);
				while (iter.iterateAvailable())
				{
					// WARNING: void * -> int cast
					intptr_t other = (intptr_t)iter.iterateNext();
					if (other == upgid)
					{
						alreadyPending = true;
						break;
					}
				}

				if (!alreadyPending)
				{
					int tmp = this->upgradesPendingCost;
					if (game->upgradeManager->canUpgrade(unit, upgid, &tmp))
					{
						fb_assert(tmp > this->upgradesPendingCost);

						//game->upgradeManager->upgrade(unit, upgid);
						// WARNING: int -> void * cast
						upgradesPending->append((void *)upgid);
						this->upgradesPendingCost = tmp;

						playDoneSound = true;
					}
				}
				else	// undo by clicking
				{
					this->upgradesPendingCost -= game->upgradeManager->getUpgradePartCost( upgid );

					LinkedListIterator iter(this->upgradesPending);
					while (iter.iterateAvailable())
					{
						// WARNING: void * -> int cast
						void* ptr = iter.iterateNext();
						if ( (intptr_t)ptr == upgid)
						{
							upgradesPending->remove( ptr );
						}
					}

				}
				
				if (playDoneSound)
				{
					game->gameUI->playGUISound(getLocaleGuiString("gui_upgrades_sound_upgrade_done"));
				} else {
					game->gameUI->playGUISound(getLocaleGuiString("gui_upgrades_sound_upgrade_failed"));
				}
			}
		}

		update();
	}

	void UpgradeWindow::EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
		{
			game->gameUI->closeUpgradeWindow(this->unit);
			return;
		}
	}

	void UpgradeWindow::raise()
	{
		effectWindow->raise();
		win->Raise();
	}


	void UpgradeWindow::undoUpgrades()
	{
		while (!upgradesPending->isEmpty())
		{
			upgradesPending->popLast();
		}
		this->upgradesPendingCost = 0;
	}


	void UpgradeWindow::applyUpgrades()
	{
		while (!upgradesPending->isEmpty())
		{
			// WARNING: void * -> int cast!
			intptr_t upgid = (intptr_t)upgradesPending->popLast();

			if (game->upgradeManager->canUpgrade(unit, upgid))
			{
				game->upgradeManager->upgrade(unit, upgid);
			} else {
				Logger::getInstance()->warning("UpgradeWindow::applyUpgrades - Pending upgrade cannot be applied, internal error.");
			}
		}
		this->upgradesPendingCost = 0;
		
	}

	void UpgradeWindow::effectUpdate(int msecTimeDelta)
	{
		this->effectWindow->update(msecTimeDelta);
	}

	void UpgradeWindow::fadeOut()
	{
		visible = 2;
		win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, FADE_OUT_TIME);
		effectWindow->fadeOut(FADE_OUT_TIME);
		game->gameUI->prepareCloseUpgradeWindow(unit);
	}

	int UpgradeWindow::getFadeInTime() const
	{
		return FADE_IN_TIME;
	}

	int UpgradeWindow::getFadeOutTime() const
	{
		return FADE_IN_TIME;
	}

	int UpgradeWindow::isVisible() const
	{
		return visible;
	}
}

