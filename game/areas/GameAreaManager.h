
#ifndef GAMEAREAMANAGER_H
#define GAMEAREAMANAGER_H

#include "area_class_ids.h"
#include "area_types.h"
#include "IArea.h"
#include "../unified_handle.h"

namespace game
{
	class ChainArea;
	class Game;
	class GameAreaManagerImpl;

	class GameAreaManager
	{
	public:
		GameAreaManager(Game *game);

		~GameAreaManager();

		// for now, only single area type areas supported...
		//void addAreaOfMultiType(IArea *area, area_type_mask areaTypeMask);

		void addArea(IArea *area, area_type_value areaType);

		void run();

		UnifiedHandle getUnifiedHandle(IArea *area);

		IArea *getAreaByUnifiedHandle();

	private:
		GameAreaManagerImpl *impl;

		// run calls this for all types and all triggering units, etc.
		// chain area will also call this directly.
		void runType(const VC3 &position, area_type_value areaType);

		friend class ChainArea;
	};
}

#endif
