
#ifndef DEBUGCAMERAAREAVISUALIZER_H
#define DEBUGCAMERAAREAVISUALIZER_H

#include "../game/unified_handle_type.h"

// bitmask
typedef int DebugCameraAreaVisFlags;

#define DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED (1<<0)
#define DEBUGCAMERAAREAVISUALIZER_FLAG_PARENTSELECTED (1<<1)
#define DEBUGCAMERAAREAVISUALIZER_FLAG_RESERVED_2 (1<<2)
#define DEBUGCAMERAAREAVISUALIZER_FLAG_RESERVED_3 (1<<3)
#define DEBUGCAMERAAREAVISUALIZER_FLAG_RESERVED_4 (1<<4)


namespace game
{
	class Game;
}

namespace ui
{
	class DebugCameraAreaVisualizer
	{
	public:
		static void visualizeCameraArea(game::Game *game, UnifiedHandle area, const VC3 &cameraPosition, DebugCameraAreaVisFlags visFlags);
		static void visualizeCameraAreas(game::Game *game, const VC3 &cameraPosition);

		static void visualizeTriggerArea(game::Game *game, UnifiedHandle area, const VC3 &cameraPosition, DebugCameraAreaVisFlags visFlags);
		static void visualizeTriggerAreas(game::Game *game, const VC3 &cameraPosition);		
	};
}

#endif
