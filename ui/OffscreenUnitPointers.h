
#ifndef OFFSCREENUNITPOINTERS_H
#define OFFSCREENUNITPOINTERS_H

#include "../ogui/IOguiButtonListener.h"

namespace game
{
	class Unit;
	class Game;
}

class Ogui;
class OguiWindow;
class OguiButton;
class IOguiImage;
class LinkedList;

namespace ui
{
	class OffscreenUnitPointers : public IOguiButtonListener
	{
		public:
			OffscreenUnitPointers(Ogui *ogui, game::Game *game, int player, OguiWindow *window);

			~OffscreenUnitPointers();

			void setEnabled(bool enabled);

			void addUnitForChecklist(game::Unit *unit);

			void removeUnitFromChecklist(game::Unit *unit);

			void update();

	    virtual void CursorEvent(OguiButtonEvent *eve);

		private:
			Ogui *ogui;
			game::Game *game;
			int player;
			OguiWindow *win;

			IOguiImage *upImage;
			IOguiImage *downImage;
			IOguiImage *leftImage;
			IOguiImage *rightImage;

			LinkedList *checkUnits;

			int pointedUnitsAmount;
			OguiButton **pointerButs;
			game::Unit **pointedUnits;

			bool enabled;
	};
}


#endif

