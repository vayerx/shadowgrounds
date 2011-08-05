
#ifndef WEAPONSELECTIONWINDOW_H
#define WEAPONSELECTIONWINDOW_H


#define WEAPONSELECTIONWINDOW_MAX_WEAPONS 8


namespace game
{
	class Game;
}

class Ogui;
class OguiWindow;
class OguiButton;
class IOguiImage;

namespace ui
{
	class WeaponSelectionWindow
	{
		public:

			WeaponSelectionWindow(game::Game *game, Ogui *ogui, 
				int cornerNumber, int weaponsMask);
				
			~WeaponSelectionWindow();

			void setSelectedWeapon(int selectionNumber);

			bool advanceTimeout();

		private:

			int selectionNumber;
			game::Game *game;
			Ogui *ogui;

			OguiWindow *win;
			IOguiImage *selectionImage;
			OguiButton *selection;

			IOguiImage *weaponImages[WEAPONSELECTIONWINDOW_MAX_WEAPONS];
			OguiButton *weaponButtons[WEAPONSELECTIONWINDOW_MAX_WEAPONS];

			void getWeaponPosition(int number, int *x, int *y);

	};
}


#endif



