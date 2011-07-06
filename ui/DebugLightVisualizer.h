
#ifndef DEBUGLIGHTVISUALIZER_H
#define DEBUGLIGHTVISUALIZER_H

#include "../game/unified_handle_type.h"

// bitmask
typedef int DebugLightVisFlags;

#define DEBUGLIGHTVISUALIZER_FLAG_SELECTED (1<<0)
#define DEBUGLIGHTVISUALIZER_FLAG_RESERVED_1 (1<<1)
#define DEBUGLIGHTVISUALIZER_FLAG_RESERVED_2 (1<<2)
#define DEBUGLIGHTVISUALIZER_FLAG_RESERVED_3 (1<<3)
#define DEBUGLIGHTVISUALIZER_FLAG_RESERVED_4 (1<<4)


namespace game
{
	class Game;
}

namespace ui
{
	class DebugLightVisualizer
	{
	public:
		static void visualizeLight(game::Game *game, UnifiedHandle light, const VC3 &cameraPosition, DebugLightVisFlags visFlags);

		static void visualizeLights(game::Game *game, const VC3 &cameraPosition);
	};
}

#endif
