#ifndef DEBUGTERRAINOBJECTVISUALIZER_H
#define DEBUGTERRAINOBJECTVISUALIZER_H

#include "../game/unified_handle_type.h"

// bitmask
typedef int DebugTerrainObjectVisFlags;

#define DEBUGTERRAINOBJECTVISUALIZER_FLAG_SELECTED   (1 << 0)
#define DEBUGTERRAINOBJECTVISUALIZER_FLAG_RESERVED_1 (1 << 1)
#define DEBUGTERRAINOBJECTVISUALIZER_FLAG_RESERVED_2 (1 << 2)
#define DEBUGTERRAINOBJECTVISUALIZER_FLAG_RESERVED_3 (1 << 3)
#define DEBUGTERRAINOBJECTVISUALIZER_FLAG_RESERVED_4 (1 << 4)

namespace game
{
    class Game;
}

namespace ui
{
    class DebugTerrainObjectVisualizer {
    public:
        static void visualizeTerrainObject(game::Game                *game,
                                           UnifiedHandle              terrainObject,
                                           const VC3                 &cameraPosition,
                                           DebugTerrainObjectVisFlags visFlags);

        static void visualizeTerrainObjects(game::Game *game, const VC3 &cameraPosition);
    };
}

#endif
