
#ifndef UNIFIEDHANDLEMANAGER_H
#define UNIFIEDHANDLEMANAGER_H

#include "unified_handle_type.h"

namespace game
{
	class Game;

	class UnifiedHandleManager
	{
	public:
		UnifiedHandleManager(Game *game);
		~UnifiedHandleManager();

		bool doesObjectExist(UnifiedHandle uh) const;

		VC3 getObjectPosition(UnifiedHandle uh) const;
		void setObjectPosition(UnifiedHandle uh, const VC3 &position);

		QUAT getObjectRotation(UnifiedHandle uh) const;
		void setObjectRotation(UnifiedHandle uh, const QUAT &rotation);

		VC3 getObjectVelocity(UnifiedHandle uh) const;
		void setObjectVelocity(UnifiedHandle uh, const VC3 &velocity);

		VC3 getObjectCenterPosition(UnifiedHandle uh) const;
		void setObjectCenterPosition(UnifiedHandle uh, const VC3 &centerPosition);

	private:
		Game *game;
	};
}

#endif


