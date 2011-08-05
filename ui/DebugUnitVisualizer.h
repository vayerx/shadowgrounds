
#ifndef DEBUGUNITVISUALIZER_H
#define DEBUGUNITVISUALIZER_H


// bitmask
typedef int DebugUnitVisFlags;

#define DEBUGUNITVISUALIZER_FLAG_SELECTED (1<<0)
#define DEBUGUNITVISUALIZER_FLAG_RESERVED_1 (1<<1)
#define DEBUGUNITVISUALIZER_FLAG_RESERVED_2 (1<<2)
#define DEBUGUNITVISUALIZER_FLAG_RESERVED_3 (1<<3)
#define DEBUGUNITVISUALIZER_FLAG_RESERVED_4 (1<<4)


namespace game
{
	class UnitList;
	class Unit;
}

namespace ui
{
	class DebugUnitVisualizer
	{
	public:
		static void visualizeUnit(game::Unit *unit, const VC3 &cameraPosition, DebugUnitVisFlags visFlags);

		static void visualizeUnits(game::UnitList *units, const VC3 &cameraPosition);
	};
}

#endif
