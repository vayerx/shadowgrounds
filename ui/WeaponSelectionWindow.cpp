
#include "precompiled.h"

#include "WeaponSelectionWindow.h"

#include <string.h>
#include "../game/Game.h"
#include "../ogui/Ogui.h"

#define WEAPSELECT_WINDOW_SIZE 256
#define WEAPSELECT_SELECTION_SIZE 72
#define WEAPSELECT_CORNER_MARGIN 16


namespace ui
{
	WeaponSelectionWindow::WeaponSelectionWindow(game::Game *game, Ogui *ogui,
		int cornerNumber, int weaponsMask)
	{
		assert(cornerNumber >= 0 && cornerNumber < 4);
		
		this->selectionNumber = -1;
		this->game = game;
		this->ogui = ogui;

		int x = 4;
		int y = 4;
		if (cornerNumber == 0 || cornerNumber == 4)
			x = 1024-WEAPSELECT_WINDOW_SIZE-4;
		if (cornerNumber == 0 || cornerNumber == 1)
			y = 768-WEAPSELECT_WINDOW_SIZE-4;

		win = ogui->CreateSimpleWindow(x, y, WEAPSELECT_WINDOW_SIZE, WEAPSELECT_WINDOW_SIZE, "Data/GUI/Windows/weaponselection.tga");

		selectionImage = ogui->LoadOguiImage("Data/GUI/Ingame/Weaponselect/selectedweapon.tga");
		selection = NULL;

		for (int i = 0; i < WEAPONSELECTIONWINDOW_MAX_WEAPONS; i++)
		{
			if ((weaponsMask & (1 << i)) != 0)
			{
				char buf[256];
				strcpy(buf, "Data/GUI/Ingame/Weaponselect/weapon_");
				buf[strlen(buf) + 1] = '\0';
				buf[strlen(buf)] = '0' + i;
				strcat(buf, ".tga");
				weaponImages[i] = ogui->LoadOguiImage(buf);

				int x = 0;
				int y = 0;
				getWeaponPosition(i, &x, &y);

				weaponButtons[i] = ogui->CreateSimpleImageButton(win, x, y, WEAPSELECT_SELECTION_SIZE, WEAPSELECT_SELECTION_SIZE, NULL, NULL, NULL);
				weaponButtons[i]->SetReactMask(0);
				weaponButtons[i]->SetDisabled(true);
				weaponButtons[i]->SetDisabledImage(weaponImages[i]);
			} else {
				weaponImages[i] = NULL;
				weaponButtons[i] = NULL;
			}
		}
	}


	WeaponSelectionWindow::~WeaponSelectionWindow()
	{
		//setSelectedWeapon(-1);
		if (selection != NULL)
		{
			delete selection;
			selection = NULL;
		}	

		delete selectionImage;
		for (int i = 0; i < WEAPONSELECTIONWINDOW_MAX_WEAPONS; i++)
		{
			if (weaponButtons[i] != NULL)
				delete weaponButtons[i];
			if (weaponImages[i] != NULL)
				delete weaponImages[i];
		}
		delete win;
	}


	void WeaponSelectionWindow::setSelectedWeapon(int selectionNumber)
	{
		assert(selectionNumber >= -1 && selectionNumber < 8);

		if (selectionNumber == -1)
		{
			/*
			if (selection != NULL)
			{
				delete selection;
				selection = NULL;
			}	
			*/		
		} else {
			if (weaponButtons[selectionNumber] != NULL)
			{
				if (selection == NULL)
				{
					selection = ogui->CreateSimpleImageButton(win, 0, 0, WEAPSELECT_SELECTION_SIZE, WEAPSELECT_SELECTION_SIZE, NULL, NULL, NULL, NULL, 0, 0, false);
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
	}


	bool WeaponSelectionWindow::advanceTimeout()
	{
		return true; // :)
	}


	void WeaponSelectionWindow::getWeaponPosition(int number, int *x, int *y)
	{
		assert(number >= -1 && number < 8);

		*x = (WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE)/2;
		*y = (WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE)/2;
		switch(number)
		{
			case 0:
				*y = 0;
				break;
			case 1:
				*x = WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE - WEAPSELECT_CORNER_MARGIN;
				*y = WEAPSELECT_CORNER_MARGIN;
				break;
			case 2:
				*x = WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE;
				break;
			case 3:
				*x = WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE - WEAPSELECT_CORNER_MARGIN;
				*y = WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE - WEAPSELECT_CORNER_MARGIN;
				break;
			case 4:
				*y = WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE;
				break;
			case 5:
				*x = WEAPSELECT_CORNER_MARGIN;
				*y = WEAPSELECT_WINDOW_SIZE - WEAPSELECT_SELECTION_SIZE - WEAPSELECT_CORNER_MARGIN;
				break;
			case 6:
				*x = 0;
				break;
			case 7:
				*x = WEAPSELECT_CORNER_MARGIN;
				*y = WEAPSELECT_CORNER_MARGIN;
				break;				
			default:
				// nop
				break;
		}
	}


}

