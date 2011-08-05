
#ifndef SELECTIONVISUALIZER_H
#define SELECTIONVISUALIZER_H

#include "../game/unified_handle_type.h"

namespace game
{
	class Game;
}

namespace ui
{
	class SelectionVisualizer
	{
	public:
		static void visualizeSelections(game::Game *game);

		static void clearSelectionForUnifiedHandle(UnifiedHandle uh);
		static void setSelectionForUnifiedHandle(UnifiedHandle uh);
	};
}

#endif
