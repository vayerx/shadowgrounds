
#ifndef UPGRADEWINDOW_H
#define UPGRADEWINDOW_H

#include "../ogui/IOguiButtonListener.h"
#include "../ogui/IOguiEffectListener.h"

#include <vector>


#define UPGRADEWINDOW_MAX_WEAPONS 10

#define UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON 3


namespace game
{
	class Game;
	class Unit;
}

class LinkedList;
class Ogui;
class OguiWindow;
class OguiButton;
class OguiTextLabel;
class IOguiImage;
class OguiFormattedText;
class IOguiFont;

namespace ui
{
	class GUIEffectWindow;

	class UpgradeWindow : public IOguiButtonListener, private IOguiEffectListener
	{
		public:

			UpgradeWindow(Ogui *ogui, game::Game *game, game::Unit *unit);
				
			~UpgradeWindow();

			void update();

			void effectUpdate(int msecTimeDelta);

			void raise();

			void applyUpgrades();
			void undoUpgrades();

			virtual void CursorEvent(OguiButtonEvent *eve);
			void EffectEvent(OguiEffectEvent *e);

			void fadeOut();
			int getFadeInTime() const;
			int getFadeOutTime() const;
			int isVisible() const;

		private:

			//int selectionNumber;
			game::Game *game;
			game::Unit *unit;
			Ogui *ogui;

			LinkedList *upgradesPending;
			int upgradesPendingCost;

			GUIEffectWindow *effectWindow;

			OguiWindow *win;
			//IOguiImage *selectionImage;
			//OguiButton *selection;

			IOguiImage *weaponImages[UPGRADEWINDOW_MAX_WEAPONS];
			IOguiImage *weaponInfoImages[UPGRADEWINDOW_MAX_WEAPONS];
			OguiButton *weaponButtons[UPGRADEWINDOW_MAX_WEAPONS];

			IOguiImage *weaponHighlightImage;
			OguiButton *weaponHighlight;

			IOguiImage *upgradeImages[UPGRADEWINDOW_MAX_WEAPONS][UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON];
			//IOguiImage *upgradeDisabledImages[UPGRADEWINDOW_MAX_WEAPONS][UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON];
			//IOguiImage *upgradeSelectedImages[UPGRADEWINDOW_MAX_WEAPONS][UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON];
			IOguiImage *upgradeBGImage;
			IOguiImage *upgradeBGDisabledImage;
			IOguiImage *upgradeHighlightImage;
			IOguiImage *upgradeSelectedImage;
			//IOguiImage *upgradeBGSelectedImage;
			OguiButton *slotBGs[UPGRADEWINDOW_MAX_WEAPONS];
			OguiButton *upgradeSelections[UPGRADEWINDOW_MAX_WEAPONS][UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON];
			OguiButton *upgradeHighlight;
			OguiButton *upgradeButtons[UPGRADEWINDOW_MAX_WEAPONS][UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON];

			//OguiButton *apply;
			//OguiButton *close;

			//OguiButton *ok;
			//OguiButton *cancel;

			OguiButton *closeButton;
			OguiButton *undoButton;

			OguiButton *upgradePartsIcon;
			OguiTextLabel *upgradePartsText;

			OguiButton *titleButton;

			OguiButton *infoBackground;
			OguiButton *infoIcon;
			OguiFormattedText *infoText;

			int highlightedWeaponSlot;
			int highlightedUpgradeSlot;
			intptr_t highlightedUpgradeId;
			bool highlightOn;

			int visible;

			std::vector< IOguiFont* > fonts;
			// dev
			//OguiButton *dev_downgrade_all;
			//OguiButton *dev_restore_orig;
			//OguiButton *dev_apply_all;

			void getWeaponPosition(int number, int *x, int *y);

	};
}


#endif



