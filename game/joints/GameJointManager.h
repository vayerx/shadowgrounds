
#ifndef GAMEJOINTMANAGER_H
#define GAMEJOINTMANAGER_H

#include "joint_types.h"
#include "joint_class_ids.h"
#include "IJoint.h"
#include "../unified_handle.h"

namespace game
{
	class Game;
	class GameJointManagerImpl;

	class GameJointManager
	{
	public:
		GameJointManager(Game *game);

		~GameJointManager();

		void initJoints();

		void deleteAllJoints();

		void addJoint(IJoint *joint);

		void run();

		UnifiedHandle getUnifiedHandle(IJoint *joint);

		IJoint *getJointByUnifiedHandle();

	private:
		GameJointManagerImpl *impl;
	};
}

#endif
